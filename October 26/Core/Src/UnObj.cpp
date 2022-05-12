/*=============================================================================
	UnObj.cpp: Unreal object manager.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "..\..\Engine\Src\EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Object manager internal variables.
UBOOL						    UObject::GObjInitialized		= 0;
UBOOL						    UObject::GObjNoRegister			= 0;
INT							    UObject::GObjBeginLoadCount		= 0;
INT							    UObject::GObjRegisterCount		= 0;
INT							    UObject::GImportCount			= 0;
UObject*					    UObject::GAutoRegister			= NULL;
UPackage*					    UObject::GObjTransientPkg		= NULL;
TCHAR						    UObject::GObjCachedLanguage[32] = TEXT("");
TCHAR						    UObject::GLanguage[64]          = TEXT("int");
UObject*					    UObject::GObjHash[4096];
TArray<UObject*>			    UObject::GObjLoaded;
TArray<UObject*>			    UObject::GObjObjects;
TArray<INT>					    UObject::GObjAvailable;
TArray<UObject*>			    UObject::GObjLoaders;
TArray<UObject*>			    UObject::GObjRoot;
TArray<UObject*>			    UObject::GObjRegistrants;
TArray<FPreferencesInfo>	    UObject::GObjPreferences;
TArray<FRegistryObjectInfo>     UObject::GObjDrivers;
TArray<FModelDataSpecification>	UObject::GObjModelData;
TArray<FModelDataGroup> 	    UObject::GObjModelDataGroups;
TMultiMap<FName,FName>*		    UObject::GObjPackageRemap;
static INT GGarbageRefCount=0;

/*-----------------------------------------------------------------------------
	UObject constructors.
-----------------------------------------------------------------------------*/

UObject::UObject()
{
	//Magic=UOBJECT_MAGIC;
}
UObject::UObject( const UObject& Src )
{
	check(&Src);
	if( Src.GetClass()!=GetClass() )
		appErrorf( TEXT("Attempt to copy-construct %s from %s"), GetFullName(), Src.GetFullName() );

	//Magic=UOBJECT_MAGIC;

}
UObject::UObject( EInPlaceConstructor, UClass* InClass, UObject* InOuter, FName InName, DWORD InFlags )
{
	//Magic=UOBJECT_MAGIC;
	StaticAllocateObject( InClass, InOuter, InName, InFlags, NULL, GError, this );
}
UObject::UObject( ENativeConstructor, UClass* InClass, const TCHAR* InName, const TCHAR* InPackageName, DWORD InFlags )
:	Index		( INDEX_NONE				)
,	HashNext	( NULL						)
,	StateFrame	( NULL						)
,	_Linker		( NULL						)
,	ObjectFlags	( InFlags | RF_Native	    )
,	Class		( InClass					)
,	_LinkerIndex( INDEX_NONE				)
,	Outer       ( NULL						)
,	Name        ( NAME_None					)
{

	// Make sure registration is allowed now.
	check(!GObjNoRegister);

	// Setup registration info, for processing now (if inited) or later (if starting up).
	check(sizeof(Outer       )>=sizeof(InPackageName));
	check(sizeof(Name        )>=sizeof(InName       ));
	check(sizeof(_LinkerIndex)>=sizeof(GAutoRegister));
	//Magic=UOBJECT_MAGIC;
	*(const TCHAR  **)&Outer        = InPackageName;
	*(const TCHAR  **)&Name         = InName;
	*(UObject      **)&_LinkerIndex = GAutoRegister;
	GAutoRegister                   = this;

	// Call native registration from terminal constructor.
	if( GetInitialized() && GetClass()==StaticClass() )
		Register();

}
UObject& UObject::operator=( const UObject& Src )
{
	check(&Src);
	if( Src.GetClass()!=GetClass() )
		appErrorf( TEXT("Attempt to assign %s from %s"), GetFullName(), Src.GetFullName() );
	return *this;
}

/*-----------------------------------------------------------------------------
	UObject class initializer.
-----------------------------------------------------------------------------*/

void UObject::StaticConstructor()
{
	//Magic=UOBJECT_MAGIC;
}

/*-----------------------------------------------------------------------------
	UObject implementation.
-----------------------------------------------------------------------------*/

//
// Rename this object to a unique name.
//
void UObject::Rename( const TCHAR* InName )
{

	FName NewName = InName ? FName(InName) : MakeUniqueObjectName( GetOuter(), GetClass() );
	UnhashObject( Outer ? Outer->GetIndex() : 0 );
	debugfSlow( TEXT("Renaming %s to %s"), *Name, *NewName );
	Name = NewName;
	HashObject();

}

//
// Shutdown after a critical error.
//
void UObject::ShutdownAfterError()
{
}

//
// Make sure the object is valid.
//
UBOOL UObject::IsValid()
{
	if( !this )
	{
		debugf( NAME_Warning, TEXT("NULL object") );
		return 0;
	}
	else if( !GObjObjects.IsValidIndex(GetIndex()) )
	{
		debugf( NAME_Warning, TEXT("Invalid object index %i"), GetIndex() );
		//debugf( NAME_Warning, TEXT("This is: %s"), GetFullName() );
		return 0;
	}
	else if( GObjObjects(GetIndex())==NULL )
	{
		debugf( NAME_Warning, TEXT("Empty slot") );
		//debugf( NAME_Warning, TEXT("This is: %s"), GetFullName() );
		return 0;
	}
	else if( GObjObjects(GetIndex())!=this )
	{
		debugf( NAME_Warning, TEXT("Other object in slot") );
		//debugf( NAME_Warning, TEXT("This is: %s"), GetFullName() );
		debugf( NAME_Warning, TEXT("Other is: %s"), GObjObjects(GetIndex())->GetFullName() );
		return 0;
	}
	else return 1;
}

//
// Do any object-specific cleanup required
// immediately after loading an object, and immediately
// after any undo/redo.
//
void UObject::PostLoad()
{

	// Note that it has propagated.
	SetFlags( RF_DebugPostLoad );

}

//
// Edit change notification.
//
void UObject::PostEditChange()
{
	Modify();
}

//
// Do any object-specific cleanup required immediately
// before an object is killed.  Child classes may override
// this if they have to do anything here.
//
void UObject::Destroy()
{
	SetFlags( RF_DebugDestroy );

	// Destroy properties.
	ExitProperties( (BYTE*)this, GetClass() );

	// Log message.
	if( GObjInitialized && !GIsCriticalError )
		debugfSlow( NAME_DevKill, TEXT("Destroying %s"), GetFullName() );

	// Unhook from linker.
	SetLinker( NULL, INDEX_NONE );

	// Remember outer name index.
	_LinkerIndex = Outer ? Outer->GetIndex() : 0;

}

//
// Set the object's linker.
//
void UObject::SetLinker( ULinkerLoad* InLinker, INT InLinkerIndex )
{
	// Detach from existing linker.
	if( _Linker )
	{
		check(_Linker->ExportMap(_LinkerIndex)._Object!=NULL);
		check(_Linker->ExportMap(_LinkerIndex)._Object==this);
		_Linker->ExportMap(_LinkerIndex)._Object = NULL;
	}

	// Set new linker.
	_Linker      = InLinker;
	_LinkerIndex = InLinkerIndex;

}

//
// Return the object's path name.
//warning: Must be safe for NULL objects.
//warning: Must be safe for class-default meta-objects.
//
const TCHAR* UObject::GetPathName( UObject* StopOuter, TCHAR* Str ) const
{

	// Return one of 8 circular results.
	TCHAR* Result = Str ? Str : appStaticString1024();
	if( this!=StopOuter )
	{
		*Result = 0;
		if( Outer!=StopOuter )
		{
			Outer->GetPathName( StopOuter, Result );
			appStrcat( Result, TEXT(".") );
		}
		appStrcat( Result, GetName() );
	}
	else appSprintf( Result, TEXT("None") );

	return Result;
}

//
// Return the object's full name.
//warning: Must be safe for NULL objects.
//
const TCHAR* UObject::GetFullName( TCHAR* Str ) const
{

	// Return one of 8 circular results.
	TCHAR* Result = Str ? Str : appStaticString1024();
	if( this )
	{
		appSprintf( Result, TEXT("%s "), GetClass()->GetName() );
		GetPathName( NULL, Result + appStrlen( Result ) );
	}
	else
	{
		appStrcpy( Result, TEXT("None") );
	}

	return Result;
}

//
// Destroy the object if necessary.
//
UBOOL UObject::ConditionalDestroy()
{
	if( Index!=INDEX_NONE && !(GetFlags() & RF_Destroyed) )
	{
		SetFlags( RF_Destroyed );
		ClearFlags( RF_DebugDestroy );
		Destroy();
		if( !(GetFlags()&RF_DebugDestroy) )
			appErrorf( TEXT("%s failed to route Destroy"), GetFullName() );
		return 1;
	}
	else return 0;
}

//
// Register if needed.
//
void UObject::ConditionalRegister()
{
	if( GetIndex()==INDEX_NONE )
	{
		// Verify this object is on the list to register.
		INT i;
		for( i=0; i<GObjRegistrants.Num(); i++ )
			if( GObjRegistrants(i)==this )
				break;
		check(i!=GObjRegistrants.Num());

		// Register it.
		Register();
	}
}

//
// PostLoad if needed.
//
void UObject::ConditionalPostLoad()
{
	if( GetFlags() & RF_NeedPostLoad )
	{
		ClearFlags( RF_NeedPostLoad | RF_DebugPostLoad );
		PostLoad();
		if( !(GetFlags() & RF_DebugPostLoad) )
 			appErrorf( TEXT("%s failed to route PostLoad"), GetFullName() );
	}
}

//
// UObject destructor.
//warning: Called at shutdown.
//
UObject::~UObject()
{

	//Magic=UOBJECT_FREE;

	// If not initialized, skip out.
	if( Index!=INDEX_NONE && GObjInitialized && !GIsCriticalError )
	{
		// Validate it.
		check(IsValid());

		// Destroy the object if necessary.
		ConditionalDestroy();

		// Remove object from table.
		UnhashObject( _LinkerIndex );
		GObjObjects(Index) = NULL;
		GObjAvailable.AddItem( Index );
	}

	// Free execution stack.
	if( StateFrame )
		delete StateFrame;

}

//
// Archive for counting memory usage.
//
class FArchiveCountMem : public FArchive
{
public:
	FArchiveCountMem( UObject* Src )
	: Num(0), Max(0)
	{
		Src->Serialize( *this );
	}
	SIZE_T GetNum()
	{
		return Num;
	}
	SIZE_T GetMax()
	{
		return Max;
	}
	void CountBytes( SIZE_T InNum, SIZE_T InMax )
	{
		Num += InNum;
		Max += InMax;
	}
protected:
	SIZE_T Num, Max;
};

//
// Archive for persistent CRC's.
//
class FArchiveCRC : public FArchive
{
public:
	FArchiveCRC( UObject* Src, UObject* InBase )
	: CRC( 0 )
	, Base( InBase )
	{
		ArIsSaving = ArIsPersistent = 1;
		Src->Serialize( *this );
	}
	SIZE_T GetCRC()
	{
		return CRC;
	}
	void Serialize( void* Data, INT Num )
	{
		CRC = appMemCrc( Data, Num, CRC );
	}
	void SerializeCapsString( const TCHAR* Str )
	{
		TCHAR TCh;
		while( *Str )
		{
			TCh = appToUpper(*Str++);
			*this << TCh;
		}
		TCh = 0;
		*this << TCh;
	}
	virtual FArchive& operator<<( class FName& N )
	{
		SerializeCapsString( *N );
		return *this;
	}
	virtual FArchive& operator<<( class UObject*& Res )
	{
		if( Res )
		{
			SerializeCapsString( Res->GetClass()->GetName() );
			SerializeCapsString( Res->GetPathName(Res->IsIn(Base) ? Base : NULL) );
		}
		else SerializeCapsString( TEXT("None") );
		return *this;
	}
protected:
	DWORD CRC;
	UObject* Base;
};

//
// Note that the object has been modified.
//
void UObject::Modify()
{

	// Perform transaction tracking.
	if( GUndo && (GetFlags() & RF_Transactional) )
		GUndo->SaveObject( this );

}

//
// UObject serializer.
//
void UObject::Serialize( FArchive& Ar )
{
	SetFlags( RF_DebugSerialize );

	//debugf(_T(">>> Serialize: %s"),GetFullName());

	// Make sure this object's class's data is loaded.
	if( Class != UClass::StaticClass() )
		Ar.Preload( Class );

	// Special info.
	if( (!Ar.IsLoading() && !Ar.IsSaving()) || Ar.IsTrans() )
		Ar << Name << Outer << Class;
	if( !Ar.IsLoading() && !Ar.IsSaving() )
		Ar << _Linker;

	// Execution stack.
	//!!how does the stack work in conjunction with transaction tracking?
	if( !Ar.IsTrans() )
	{
		if( GetFlags() & RF_HasStack )
		{
#if 1
// CDH: new state frame serialization, allows frames to be stacked

			// CDH: for backward compatibility during serialization, an unused probe
			//      mask bit 62 is used to determine whether more state frames exist
			// FIXME: Comment this in UnNames.h so no probe accidentally gets added in this slot
			INT MaskBit = 62;

			if (Ar.IsSaving())
			{
				// write out the state stack frames, from top descending
				if (!StateFrame)
					StateFrame = new(TEXT("ObjectStateFrame"))FStateFrame(this);
				for (FStateFrame* Fr = StateFrame; Fr; Fr = Fr->StateStack)
				{
					QWORD Mask = Fr->ProbeMask;
					if (Fr->StateStack)
						Mask &= ~((QWORD)1 << MaskBit);
					else
						Mask |= ((QWORD)1 << MaskBit);
					Ar << Fr->Node << Fr->StateNode << Mask << Fr->LatentAction;
					if (Fr->Node)
					{
						if (Fr->Code)
							check(Fr->Code>=&Fr->Node->Script(0) && Fr->Code<&Fr->Node->Script(Fr->Node->Script.Num()));
						INT Offset = Fr->Code ? Fr->Code - &Fr->Node->Script(0) : INDEX_NONE;
						Ar << AR_INDEX(Offset);
					}
				}
			}
			else if (Ar.IsLoading())
			{
				if (StateFrame)
					delete StateFrame;
				StateFrame = new(TEXT("ObjectStateFrame"))FStateFrame(this);
				FStateFrame* Fr = StateFrame;
				while (Fr)
				{
					QWORD Mask;
					Ar << Fr->Node << Fr->StateNode << Mask << Fr->LatentAction;
					Fr->ProbeMask = Mask | ((QWORD)1 << MaskBit);
					Fr->Code = NULL;
					if (Fr->Node)
					{
						Ar.Preload(Fr->Node);
						INT Offset;
						Ar << AR_INDEX(Offset);
						if (Offset != INDEX_NONE)
							if (Offset<0 || Offset>=Fr->Node->Script.Num())
								appErrorf(TEXT("%s: Offset mismatch: %i %i"), GetFullName(), Offset, Fr->Node->Script.Num());
						Fr->Code = Offset!=INDEX_NONE ? &Fr->Node->Script(Offset) : NULL;
					}					
					if (!(Mask & ((QWORD)1 << MaskBit)))
					{
						Fr->StateStack = new(TEXT("ObjectStateFrame"))FStateFrame(this);
						Fr = Fr->StateStack;
					}
					else
						Fr = NULL;
				}
			}
#else
// CDH: old state frame serialization, didn't allow for a state stack
			if( !StateFrame )
				StateFrame = new(TEXT("ObjectStateFrame")) FStateFrame( this );
			Ar << StateFrame->Node;
			Ar << StateFrame->StateNode;
			Ar << StateFrame->ProbeMask;
			Ar << StateFrame->LatentAction;
			if( StateFrame->Node )
			{
				Ar.Preload( StateFrame->Node );
				if( Ar.IsSaving() && StateFrame->Code )
					check(StateFrame->Code>=&StateFrame->Node->Script(0) && StateFrame->Code<&StateFrame->Node->Script(StateFrame->Node->Script.Num()));
				INT Offset = StateFrame->Code ? StateFrame->Code - &StateFrame->Node->Script(0) : INDEX_NONE;
				Ar << AR_INDEX(Offset);
				if( Offset!=INDEX_NONE )
					if( Offset<0 || Offset>=StateFrame->Node->Script.Num() )
						appErrorf( TEXT("%s: Offset mismatch: %i %i"), GetFullName(), Offset, StateFrame->Node->Script.Num() );
				StateFrame->Code = Offset!=INDEX_NONE ? &StateFrame->Node->Script(Offset) : NULL;
			}
			else StateFrame->Code = NULL;
#endif
		}
		else if( StateFrame )
		{
			delete StateFrame;
			StateFrame = NULL;
		}
	}

	// Serialize object properties which are defined in the class.
	if( Class != UClass::StaticClass() )
	{
		if( (Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTrans() )
			GetClass()->SerializeTaggedProperties( Ar, (BYTE*)this, Class );			
		else
			GetClass()->SerializeBin( Ar, (BYTE*)this );
	}

	// Memory counting.
	SIZE_T Size = GetClass()->GetPropertiesSize();
    Ar.CountBytes( Size, Size );
}

//
// Export text object properties.
//
void UObject::ExportProperties
(
	FOutputDevice&	Out,
	UClass*			ObjectClass,
	BYTE*			Object,
	INT				Indent,
	UClass*			DiffClass,
	BYTE*			Diff
)
{
	const TCHAR* ThisName = TEXT("(none)");
	guard(UObject::ExportProperties);
	check(ObjectClass!=NULL);

	for( TFieldIterator<UProperty> It(ObjectClass); It; ++It )
	{
		if( It->Port() )
		{
            //debugf( NAME_Log, TEXT( "Export Properties %s::%s" ), ObjectClass->GetFullName(), It->GetFullName() );

			ThisName = It->GetName();
			for( INT j=0; j<It->ArrayDim; j++ )
			{
				// Export single element.
				TCHAR Value[4096];
				if( It->ExportText( j, Value, Object, (DiffClass && DiffClass->IsChildOf(It.GetStruct())) ? Diff : NULL, PPF_Delimited ) )
				{
					if
					(	(It->IsA(UObjectProperty::StaticClass()))
					&&	(It->PropertyFlags & CPF_ExportObject) )
					{
						UObject* Obj = *(UObject **)((BYTE*)Object + It->Offset + j*It->ElementSize);
						if( Obj && !(Obj->GetFlags() & RF_TagImp) )
						{
							// Don't export more than once.
							UExporter::ExportToOutputDevice( Obj, NULL, Out, TEXT("T3D"), Indent+1 );
							Obj->SetFlags( RF_TagImp );
						}
					}
					if( It->ArrayDim == 1 )
						Out.Logf( TEXT("%s %s=%s\r\n"), appSpc(Indent), It->GetName(), Value );
					else
						Out.Logf( TEXT("%s %s(%i)=%s\r\n"), appSpc(Indent), It->GetName(), j, Value );
				}
			}
		}
	}
	unguardf(( TEXT("(%s)"), ThisName ));
}

//
// Initialize script execution.
//
void UObject::InitExecution()
{
	check(GetClass()!=NULL);

	if( StateFrame )
		delete StateFrame;
	StateFrame = new(TEXT("ObjectStateFrame"))FStateFrame( this );
	SetFlags( RF_HasStack );
}

//
// Command line.
//
UBOOL UObject::ScriptConsoleExec( const TCHAR* Str, FOutputDevice& Ar, UObject* Executor )
{
	// No script execution?
	if( !GIsScriptable )
		return 0;

	// Find UnrealScript exec function.
	TCHAR MsgStr[NAME_SIZE];
	FName Message;
	UFunction* Function;
	if
	(	!ParseToken(Str,MsgStr,ARRAY_COUNT(MsgStr),1)
	||	(Message=FName(MsgStr,FNAME_Find))==NAME_None 
	||	(Function=FindFunction(Message))==NULL 
	||	!(Function->FunctionFlags & FUNC_Exec) )
		return 0;

	// Parse all function parameters.
	BYTE* Parms = (BYTE*)appAlloca(Function->ParmsSize);
	appMemzero( Parms, Function->ParmsSize );
	UBOOL Failed = 0;
	INT Count = 0;
	for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It,Count++ )
	{
		if( Count==0 && Executor )
		{
			UObjectProperty* Op = Cast<UObjectProperty>(*It);
			if( Op && Executor->IsA(Op->PropertyClass) )
			{
				// First parameter is implicit reference to object executing the command.
				*(UObject**)(Parms + It->Offset) = Executor;
				continue;
			}
		}
		ParseNext( &Str );
		Str = It->ImportText( Str, Parms+It->Offset, PPF_Localized );
		if( !Str )
		{
			if( !(It->PropertyFlags & CPF_OptionalParm) )
			{
				Ar.Logf( LocalizeError("BadProperty"), *Message, It->GetName() );
				Failed = 1;
			}
			break;
		}
	}
	if( !Failed )
		ProcessEvent( Function, Parms );
	//!!destructframe see also UObject::ProcessEvent
	{for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
		It->DestroyValue( Parms + It->Offset );}

	// Success.
	return 1;
}

//
// Find an UnrealScript field.
//warning: Must be safe with class default metaobjects.
//
UField* __fastcall UObject::FindObjectField( FName InName, UBOOL Global )
{
	/* Fast, VF-hashed version */
	INT iHash = InName.GetIndex() & (UField::HASH_COUNT-1);

	// Search current state scope.
	if( StateFrame && StateFrame->StateNode && !Global )
		for( UField* Node=StateFrame->StateNode->VfHash[iHash]; Node; Node=Node->HashNext )
			if( Node->GetFName()==InName )
				return Node;

	// Search the global scope.
	for( UField* Node=GetClass()->VfHash[iHash]; Node; Node=Node->HashNext )
		if( Node->GetFName()==InName )
			return Node;

	// Not found.
	return NULL;
/*
    // Slow, unhashed version 
	// Search current state scope.
	if( StateFrame && StateFrame->StateNode && !Global )
	{
		for( TFieldIterator<UField> It(StateFrame->StateNode); It; ++It )
			if( It->GetFName()==InName )
				return *It;
			else
				debugf(TEXT("FindObjectField(%s): Traversed Name: %s (State scope)"),*InName,*It->GetFName());
	} else
	{
		debugf(TEXT("FindObjectField(%s): !( StateFrame && StateFrame->StateNode && !Global )"),*InName);
	}
	debugf(TEXT("Class: %s %s"),*GetFName(), *GetClass()->GetFName());

	{
		TFieldIterator<UField> It(GetClass());
		It.DumpAllFields(GetClass());
	}
	// Search the global scope.
	for( TFieldIterator<UField> It(GetClass()); It; ++It )
		if( It->GetFName()==InName )
		{
			debugf(TEXT("Returning 'IT' %s"),*It->GetFName());
			return *It;
		} else
		{
			debugf(TEXT("FindObjectField(%s): Traversed Name: %s (global scope)"),*InName,*It->GetFName());
		}


	// Not found.
	return NULL;
*/
}

UFunction* __fastcall UObject::FindFunctionChecked( FName InName, UBOOL Global )
{
	if( !GIsScriptable ) return NULL;
	UField* ObjectField = FindObjectField( InName, Global );
	if(!ObjectField)
		appErrorf( _T("Failed to find field %s in %s"), *InName, GetFullName() );

	UFunction* Result = Cast<UFunction>( ObjectField );
	if( !Result )
		appErrorf( _T("Failed to find function %s in %s"), *InName, GetFullName() );
	return Result;
}

__forceinline UFunction* __fastcall UObject::FindFunction( FName InName, UBOOL Global )
{
	return Cast<UFunction>(FindObjectField(InName,Global));
}
__forceinline UState* __fastcall UObject::FindState( FName InName )
{
	return Cast<UState>( FindObjectField( InName, 1 ) );
}

IMPLEMENT_CLASS(UObject);

/*-----------------------------------------------------------------------------
	UObject configuration.
-----------------------------------------------------------------------------*/

//
// Load configuration.
//warning: Must be safe on class-default metaobjects.
//
void UObject::LoadConfig( UBOOL Propagate, UClass* Class, const TCHAR* InFilename )
{
	if( !Class )
		Class = GetClass();
	if( !(Class->ClassFlags & CLASS_Config) )
		return;
	if( Propagate && Class->GetSuperClass() )
		LoadConfig( Propagate, Class->GetSuperClass(), InFilename );
	UBOOL PerObject = ((GetClass()->ClassFlags & CLASS_PerObjectConfig) && GetIndex()!=INDEX_NONE);
	const TCHAR* Filename
	=	InFilename
	?	InFilename
	:	(PerObject && Outer!=GObjTransientPkg)
	?	GetOuter()->GetName()
	:	*GetClass()->ClassConfigName;
	for( TFieldIterator<UProperty> It(Class); It; ++It )
	{
		if( It->PropertyFlags & CPF_Config )
		{
			TCHAR TempKey[256], Value[1024]=TEXT("");
			UClass*         BaseClass = (It->PropertyFlags & CPF_GlobalConfig) ? It->GetOwnerClass() : Class;
			const TCHAR*    Section   = PerObject ? GetName() : BaseClass->GetPathName();
			const TCHAR*    Key       = It->ArrayDim==1 ? It->GetName() : TempKey;
			UArrayProperty* Array     = Cast<UArrayProperty>( *It );
			UMapProperty*   Map       = Cast<UMapProperty>( *It );
			if( Array )
			{
				TMultiMap<FString,FString>* Sec = GConfig->GetSectionPrivate( Section, 0, 1, Filename );
				if( Sec )
				{
					TArray<FString> List;
					Sec->MultiFind( FString(Key), List );
					FArray* Ptr  = (FArray*)((BYTE*)this + It->Offset);
					INT     Size = Array->Inner->ElementSize;
					Array->DestroyValue( Ptr );
					Ptr->AddZeroed( Size, List.Num() );
					for( INT i=List.Num()-1,c=0; i>=0; i--,c++ )
						Array->Inner->ImportText( *List(i), (BYTE*)Ptr->GetData() + c*Size, 0 );
				}
			}
			else if( Map )
			{
				TMultiMap<FString,FString>* Sec = GConfig->GetSectionPrivate( Section, 0, 1, Filename );
				if( Sec )
				{
					TArray<FString> List;
					Sec->MultiFind( Key, List );
					//FArray* Ptr  = (FArray*)((BYTE*)this + It->Offset);
					//INT     Size = Array->Inner->ElementSize;
					//Map->DestroyValue( Ptr );//!!won't do dction
					//Ptr->AddZeroed( Size, List.Num() );
					//for( INT i=List.Num()-1,c=0; i>=0; i--,c++ )
					//	Array->Inner->ImportText( *List(i), (BYTE*)Ptr->GetData() + c*Size, 0 );
				}
			}
			else for( INT i=0; i<It->ArrayDim; i++ )
			{
				if( It->ArrayDim!=1 )
					appSprintf( TempKey, TEXT("%s[%i]"), It->GetName(), i );
				if( GConfig->GetString( Section, Key, Value, ARRAY_COUNT(Value), Filename ) )
					It->ImportText( Value, (BYTE*)this + It->Offset + i*It->ElementSize, 0 );					
			}
		}
	}
}

//
// Load localized text.
//warning: Must be safe on class-default metaobjects.
//
void UObject::LoadLocalized( UBOOL Propagate, UClass* Class )
{
	checkSlow(GetClass());
	if( !Class )
		Class = GetClass();
	if( !(Class->ClassFlags & CLASS_Localized) )
		return;
	if( GIsEditor )
		return;
	if( Propagate && Class->GetSuperClass() )
		LoadLocalized( Propagate, Class->GetSuperClass() );
	const TCHAR* PackageName = GetIndex()==INDEX_NONE ? Class->GetOuter()->GetName() : GetOuter()->GetName();
	const TCHAR* Section     = GetIndex()==INDEX_NONE ? Class->GetName()             : GetName();
	for( TFieldIterator<UProperty> It(Class); It; ++It )
	{
		if( It->PropertyFlags & CPF_Localized )
		{
			for( INT i=0; i<It->ArrayDim; i++ )
			{
				TCHAR TempKey[256];
				const TCHAR* Key = It->GetName();
				if( It->ArrayDim!=1 )
				{
					appSprintf( TempKey, TEXT("%s[%i]"), It->GetName(), i );
					Key = TempKey;
				}
				const TCHAR* Text = Localize( Section, Key, PackageName, NULL, 1 );
				if( *Text )
					It->ImportText( Text, (BYTE*)this + It->Offset + i*It->ElementSize, 0 );					
			}
		}
	}
}

//
// Save configuration.
//warning: Must be safe on class-default metaobjects.
//!!may benefit from hierarchical propagation, deleting keys that match superclass...not sure what's best yet.
//
void UObject::SaveConfig( DWORD Flags, const TCHAR* InFilename )
{
	UBOOL PerObject = ((GetClass()->ClassFlags & CLASS_PerObjectConfig) && GetIndex()!=INDEX_NONE);
	const TCHAR* Filename
	=	InFilename
	?	InFilename
	:	(PerObject && Outer!=GObjTransientPkg)
	?	GetOuter()->GetName()
	:	*GetClass()->ClassConfigName;
	for( TFieldIterator<UProperty> It(GetClass()); It; ++It )
	{
		if( (It->PropertyFlags & Flags)==Flags )
		{
			TCHAR TempKey[256], Value[1024]=TEXT("");
			const TCHAR*    Key     = It->GetName();
			const TCHAR*    Section = PerObject ? GetName() : (It->PropertyFlags & CPF_GlobalConfig) ? It->GetOwnerClass()->GetPathName() : GetClass()->GetPathName();
			UArrayProperty* Array   = Cast<UArrayProperty>( *It );
			UMapProperty*   Map     = Cast<UMapProperty>( *It );
			if( Array )
			{
				TMultiMap<FString,FString>* Sec = GConfig->GetSectionPrivate( Section, 1, 0, Filename );
				check(Sec);
				Sec->Remove( Key );
				FArray* Ptr  = (FArray*)((BYTE*)this + It->Offset);
				INT     Size = Array->Inner->ElementSize;
				for( INT i=0; i<Ptr->Num(); i++ )
				{
					TCHAR Buffer[1024]=TEXT("");
					BYTE* Dest = (BYTE*)Ptr->GetData() + i*Size;
					Array->Inner->ExportTextItem( Buffer, Dest, Dest, 0 );
					Sec->Add( Key, Buffer );
				}
			}
			else if( Map )
			{
				TMultiMap<FString,FString>* Sec = GConfig->GetSectionPrivate( Section, 1, 0, Filename );
				check(Sec);
				Sec->Remove( Key );
				//FArray* Ptr  = (FArray*)((BYTE*)this + It->Offset);
				//INT     Size = Array->Inner->ElementSize;
				//for( INT i=0; i<Ptr->Num(); i++ )
				//{
				//	TCHAR Buffer[1024]="";
				//	BYTE* Dest = (BYTE*)Ptr->GetData() + i*Size;
				//	Array->Inner->ExportTextItem( Buffer, Dest, Dest, 0 );
				//	Sec->Add( Key, Buffer );
				//}
			}
			else for( INT Index=0; Index<It->ArrayDim; Index++ )
			{
				if( It->ArrayDim!=1 )
				{
					appSprintf( TempKey, TEXT("%s[%i]"), It->GetName(), Index );
					Key = TempKey;
				}
				It->ExportText( Index, Value, (BYTE*)this, (BYTE*)this, 0 );
				GConfig->SetString( Section, Key, Value, Filename );
			}
		}
	}
	GConfig->Flush( 0 );
	UClass* BaseClass;
	for( BaseClass=GetClass(); BaseClass->GetSuperClass(); BaseClass=BaseClass->GetSuperClass() )
		if( !(BaseClass->GetSuperClass()->ClassFlags & CLASS_Config) )
			break;
	if( BaseClass )
		for( TObjectIterator<UClass> It; It; ++It )
			if( It->IsChildOf(BaseClass) )
				It->GetDefaultObject()->LoadConfig( 1 );//!!propagate??
}

//
// Reset configuration.
//
void UObject::ResetConfig( UClass* Class )
{
	const TCHAR* SrcFilename=NULL;
	if( Class->ClassConfigName==NAME_System )
		SrcFilename = TEXT("Default.ini");
	else if( Class->ClassConfigName==NAME_User )
		SrcFilename = TEXT("DefUser.ini");
	else
		return;
	TCHAR Buffer[32767];
	if( GConfig->GetSection( Class->GetPathName(), Buffer, ARRAY_COUNT(Buffer), SrcFilename  ) )
	{
		TCHAR* NewKey;
		for( TCHAR* Key=Buffer; *Key; Key=NewKey )
		{	
			NewKey = Key + appStrlen(Key) + 1;
			TCHAR* Value=appStrchr(Key,'=');
			if( Value )
			{
				*Value++ = 0;
				GConfig->SetString( Class->GetPathName(), Key, Value, *Class->ClassConfigName );
			}
		}
	}
	for( TObjectIterator<UClass> ItC; ItC; ++ItC )
		if( ItC->IsChildOf(Class) )
			ItC->GetDefaultObject()->LoadConfig( 1 );//!!propagate?
	for( TObjectIterator<UObject> ItO; ItO; ++ItO )
		if( ItO->IsA(Class) )
			{ItO->LoadConfig( 1 ); ItO->PostEditChange();}//!!propagate?
}


/*-----------------------------------------------------------------------------
	Mo Functions.
-----------------------------------------------------------------------------*/

//
// Object accessor.
//
UObject* UObject::GetIndexedObject( INT Index )
{
	if( Index>=0 && Index<GObjObjects.Num() )
		return GObjObjects(Index);
	else
		return NULL;
}

//
// Find an optional object.
//
UObject* UObject::StaticFindObject( UClass* ObjectClass, UObject* InObjectPackage, const TCHAR* InName, UBOOL ExactClass )
{

	// Resolve the object and package name.
	UObject* ObjectPackage = InObjectPackage!=ANY_PACKAGE ? InObjectPackage : NULL;
	if( !ResolveName( ObjectPackage, InName, 0, 0 ) )
		return NULL;

	// Make sure it's an existing name.
	FName ObjectName(InName,FNAME_Find);
	if( ObjectName==NAME_None )
		return NULL;

	const ANSICHAR *Str = appToAnsi(*ObjectName);

	// Find in the specified package.
	INT iHash = GetObjectHash( ObjectName, ObjectPackage ? ObjectPackage->GetIndex() : 0 );
	for( UObject* Hash=GObjHash[iHash]; Hash!=NULL; Hash=Hash->HashNext )
		if
		(	(Hash->GetFName()==ObjectName)
		&&	(Hash->Outer==ObjectPackage)
		&&	(ObjectClass==NULL || (ExactClass ? Hash->GetClass()==ObjectClass : Hash->IsA(ObjectClass))) )
			return Hash;

	// Find in any package.
	if( InObjectPackage==ANY_PACKAGE )
		for( FObjectIterator It; It; ++It )
			if
			(	(It->GetFName()==ObjectName)
			&&	(ObjectClass==NULL || (ExactClass ? It->GetClass()==ObjectClass : It->IsA(ObjectClass))) )
				return *It;

	// Not found.
	return NULL;
}

//
// Find an object; can't fail.
//
UObject* UObject::StaticFindObjectChecked( UClass* ObjectClass, UObject* ObjectParent, const TCHAR* InName, UBOOL ExactClass )
{
	UObject* Result = StaticFindObject( ObjectClass, ObjectParent, InName, ExactClass );
	if( !Result )
		appErrorf( LocalizeError("ObjectNotFound"), ObjectClass->GetName(), ObjectParent==ANY_PACKAGE ? TEXT("Any") : ObjectParent ? ObjectParent->GetName() : TEXT("None"), InName );
	return Result;
}

//
// Binary initialize object properties to zero or defaults.
//
void UObject::InitProperties( BYTE* Data, INT DataCount, UClass* DefaultsClass, BYTE* Defaults, INT DefaultsCount )
{
	check(DataCount>=sizeof(UObject));
	INT Inited = sizeof(UObject);

	// Find class defaults if no template was specified.
	//warning: At startup, DefaultsClass->Defaults.Num() will be zero for some native classes.	
	if( !Defaults && DefaultsClass && DefaultsClass->Defaults[CPD_Normal].Num() )
	{
		Defaults      = &DefaultsClass->Defaults[CPD_Normal](0);
		DefaultsCount =  DefaultsClass->Defaults[CPD_Normal].Num();
	}

	// Copy defaults.
	if( Defaults )
	{
		checkSlow(DefaultsCount>=Inited);
		checkSlow(DefaultsCount<=DataCount);
		appMemcpy( Data+Inited, Defaults+Inited, DefaultsCount-Inited );
		Inited = DefaultsCount;
	}

	// Zero-fill any remaining portion.
	if( Inited < DataCount )
		appMemzero( Data+Inited, DataCount-Inited );

	// Construct anything required.
	if( DefaultsClass )
	{
		for( UProperty* P=DefaultsClass->ConstructorLink; P; P=P->ConstructorLinkNext )
		{
			if( P->Offset < DefaultsCount )
			{
				appMemzero( Data + P->Offset, P->GetSize() );//bad for bools, but not a real problem because they aren't constructed!!
				P->CopyCompleteValue( Data + P->Offset, Defaults + P->Offset );
			}
		}
	}
}

//
// Destroy properties.
//
void UObject::ExitProperties( BYTE* Data, UClass* Class )
{
	UProperty* P=NULL;
	for( P=Class->ConstructorLink; P; P=P->ConstructorLinkNext )
    {
		P->DestroyValue( Data + P->Offset );
    }
}

//
// Init class default object.
//
void UObject::InitClassDefaultObject( UClass* InClass )
{
	// Init UObject portion.
	appMemset( this, 0, sizeof(UObject) );
	*(void**)this = *(void**)InClass;
	Class         = InClass;
	Index         = INDEX_NONE;
//	ExtPropData	  = NULL;

	// Init post-UObject portion.
	InitProperties( (BYTE*)this, InClass->GetPropertiesSize(), InClass->GetSuperClass(), NULL, 0 );
}

//
// Global property setting.
//
void UObject::GlobalSetProperty( const TCHAR* Value, UClass* Class, UProperty* Property, INT Offset, UBOOL Immediate )
{

	// Apply to existing objects of the class, with notification.
	if( Immediate )
	{
		for( FObjectIterator It; It; ++It )
		{
			if( It->IsA(Class) )
			{
				Property->ImportText( Value, (BYTE*)*It + Offset, PPF_Localized );
				It->PostEditChange();
			}
		}
	}

	// Apply to defaults.
	Property->ImportText( Value, &Class->Defaults[CPD_Normal](Offset), PPF_Localized );
	Class->GetDefaultObject()->SaveConfig();
}

/*-----------------------------------------------------------------------------
	Object registration.
-----------------------------------------------------------------------------*/

//
// Preregister an object.
//warning: Sometimes called at startup time.
//
void UObject::Register()
{
	check(GObjInitialized);

	// Get stashed registration info.
	const TCHAR* InOuter = *(const TCHAR**)&Outer;
	const TCHAR* InName  = *(const TCHAR**)&Name;

	// Set object properties.
	Outer        = CreatePackage(NULL,InOuter);
	Name         = InName;
	_LinkerIndex = INDEX_NONE;

	// Validate the object.
	if( Outer==NULL )
		appErrorf( TEXT("Autoregistered object %s is unpackaged"), GetFullName() );
	if( GetFName()==NAME_None )
		appErrorf( TEXT("Autoregistered object %s has invalid name"), GetFullName() );
	if( StaticFindObject( NULL, GetOuter(), GetName() ) )
		appErrorf( TEXT("Autoregistered object %s already exists"), GetFullName() );

	// Add to the global object table.
	AddObject( INDEX_NONE );

}

//
// Handle language change.
//
void UObject::LanguageChange()
{
	LoadLocalized();
}

/*-----------------------------------------------------------------------------
	StaticInit & StaticExit.
-----------------------------------------------------------------------------*/

void SerTest( FArchive& Ar, DWORD& Value, DWORD Max )
{
	DWORD NewValue=0;
	for( DWORD Mask=1; NewValue+Mask<Max && Mask; Mask*=2 )
	{
		BYTE B = ((Value&Mask)!=0);
		Ar.SerializeBits( &B, 1 );
		if( B )
			NewValue |= Mask;
	}
	Value = NewValue;
}

// JP_REMOVE!!	(put in dnScreenshot.h)
extern UBOOL		GWaitingForScreenshot;
extern UBOOL		GScreenshotReady;
extern UTexture		*GTempTexture;

// JP_REMOVE!! (put in dnSaveLoad.h)
extern UTexture		*GThumbnailTexture;

//
// Init the object manager and allocate tables.
//
void UObject::StaticInit()
{
	GObjNoRegister = 1;

	// Checks.
	check(sizeof(BYTE)==1);
	check(sizeof(SBYTE)==1);
	check(sizeof(_WORD)==2);
	check(sizeof(DWORD)==4);
	check(sizeof(QWORD)==8);
	check(sizeof(ANSICHAR)==1);
	check(sizeof(UNICHAR)==2);
	check(sizeof(SWORD)==2);
	check(sizeof(INT)==4);
	check(sizeof(SQWORD)==8);
	check(sizeof(UBOOL)==4);
	check(sizeof(FLOAT)==4);
	check(sizeof(DOUBLE)==8);
	check(ENGINE_MIN_NET_VERSION<=ENGINE_VERSION);

	// Development.
	GCheckConflicts = ParseParam(appCmdLine(),TEXT("CONFLICTS"));
	GNoGC           = ParseParam(appCmdLine(),TEXT("NOGC"));

	// Init hash.
	for( INT i=0; i<ARRAY_COUNT(GObjHash); i++ )
		GObjHash[i] = NULL;

	// Note initialized.
	GObjInitialized = 1;

	// Add all autoregistered classes.
	ProcessRegistrants();

	// Allocate special packages.
	GObjTransientPkg = new( NULL, TEXT("Transient") )UPackage;
	GObjTransientPkg->AddToRoot();

	// Load package remap.
	GObjPackageRemap = new TMultiMap<FName,FName>;

	// JP:
	// Allocate special global texture used for save games, and screenshots
	GTempTexture = new(GObjTransientPkg, TEXT("GTempTexture") )UTexture;
	GTempTexture->Format = TEXF_RGBA8;
	GTempTexture->Init(128, 128);
	GTempTexture->PostLoad();
	GTempTexture->AddToRoot();

	// JP:
	// Allocate special global texture used for save games, and screenshots
	GThumbnailTexture = new(GObjTransientPkg, TEXT("GThumbnailTexture") )UTexture;
	GThumbnailTexture->Format = TEXF_RGBA8;
	GThumbnailTexture->Init(128, 128);
	GThumbnailTexture->PostLoad();
	GThumbnailTexture->AddToRoot();

	debugf( NAME_Init, TEXT("Object subsystem initialized.") );
}

//
// Process all objects that have registered.
//
void UObject::ProcessRegistrants()
{
	if( ++GObjRegisterCount==1 )
	{
		// Make list of all objects to be registered.
		for( ; GAutoRegister; GAutoRegister=*(UObject **)&GAutoRegister->_LinkerIndex )
			GObjRegistrants.AddItem( GAutoRegister );
		for( INT i=0; i<GObjRegistrants.Num(); i++ )
			GObjRegistrants(i)->ConditionalRegister();
		GObjRegistrants.Empty();
		check(!GAutoRegister);
	}
	GObjRegisterCount--;
}

//
// Shut down the object manager.
//
void UObject::StaticExit()
{
	check(GObjLoaded.Num()==0);
	check(GObjRegistrants.Num()==0);
	check(!GAutoRegister);

	// Cleanup root.
	GObjTransientPkg->RemoveFromRoot();
	// JP:
	GTempTexture->RemoveFromRoot();
	// JP:
	GThumbnailTexture->RemoveFromRoot();

	// Tag all objects as unreachable.
	for( FObjectIterator It; It; ++It )
		It->SetFlags( RF_Unreachable | RF_TagGarbage );

	// Tag all names as unreachable.
	for( INT i=0; i<FName::GetMaxNames(); i++ )
		if( FName::GetEntry(i) )
			FName::GetEntry(i)->Flags |= RF_Unreachable;

	// Purge all objects.
	GExitPurge = 1;
	PurgeGarbage();
	GObjObjects.Empty();

	// Empty arrays to prevent falsely-reported memory leaks.
	GObjLoaded			.Empty();
	GObjObjects			.Empty();
	GObjAvailable		.Empty();
	GObjLoaders			.Empty();
	GObjRoot			.Empty();
	GObjRegistrants		.Empty();
	GObjPreferences		.Empty();
	GObjDrivers			.Empty();
    GObjModelData		.Empty();
    GObjModelDataGroups .Empty();

	delete GObjPackageRemap;

	GObjInitialized = 0;
	debugf( NAME_Exit, TEXT("Object subsystem successfully closed.") );
}

/*-----------------------------------------------------------------------------
	UObject Tick.
-----------------------------------------------------------------------------*/

//
// Mark one unit of passing time. This is used to update the object
// caching status. This must be called when the object manager is
// in a clean state (outside of all code which retains pointers to
// object data that was gotten).
//
void UObject::StaticTick()
{
	check(GObjBeginLoadCount==0);

	// Check natives.
	CORE_API extern int GNativeDuplicate;
	if( GNativeDuplicate )
		appErrorf( TEXT("Duplicate native registered: %i"), GNativeDuplicate );
}

/*-----------------------------------------------------------------------------
   Shutdown.
-----------------------------------------------------------------------------*/

//
// Make sure this object has been shut down.
//
void UObject::ConditionalShutdownAfterError()
{
	if( !(GetFlags() & RF_ErrorShutdown) )
	{
		SetFlags( RF_ErrorShutdown );
		try
		{
			ShutdownAfterError();
		}
		catch( ... )
		{
			debugf( NAME_Exit, TEXT("Double fault in object ShutdownAfterError") );
		}
	}
}

//
// After a critical error, shutdown all objects which require
// mission-critical cleanup, such as restoring the video mode,
// releasing hardware resources.
//
void UObject::StaticShutdownAfterError()
{
	if( GObjInitialized )
	{
		static UBOOL Shutdown=0;
		if( Shutdown )
			return;
		Shutdown = 1;
		debugf( NAME_Exit, TEXT("Executing UObject::StaticShutdownAfterError") );
		try
		{
			for( INT i=0; i<GObjObjects.Num(); i++ )
				if( GObjObjects(i) )
					GObjObjects(i)->ConditionalShutdownAfterError();
		}
		catch( ... )
		{
			debugf( NAME_Exit, TEXT("Double fault in object manager ShutdownAfterError") );
		}
	}
}

//
// Bind package to DLL.
//warning: Must only find packages in the \Unreal\System directory!
//
void UObject::BindPackage( UPackage* Pkg )
{
	if( !Pkg->DllHandle && !Pkg->GetOuter() && !Pkg->AttemptedBind )
	{
		TCHAR PathName[256];
		appSprintf( PathName, TEXT("%s%s"), appBaseDir(), Pkg->GetName() );
		Pkg->AttemptedBind  = 1;
		GObjNoRegister      = 0;
		Pkg->DllHandle      = appGetDllHandle( PathName );
		GObjNoRegister      = 1;
		if( Pkg->DllHandle )
		{
			debugf( NAME_Log, TEXT("Bound to %s%s"), Pkg->GetName(), DLLEXT );
			ProcessRegistrants();
			
			// CDH... post-bind initialization
			typedef void (*InitPackageFunc)();
			InitPackageFunc InitFunc = (InitPackageFunc)appGetDllExport(Pkg->DllHandle, TEXT("InitPackage"));
			if (InitFunc)
				InitFunc();
			// CDH...
		}
	}
}

/*-----------------------------------------------------------------------------
   Command line.
-----------------------------------------------------------------------------*/

//
// Archive for enumerating other objects referencing a given object.
//
class FArchiveShowReferences : public FArchive
{
public:
	FArchiveShowReferences( FOutputDevice& InAr, UObject* InOuter, UObject* InObj, TArray<UObject*>& InExclude )
	: DidRef( 0 ), Ar( InAr ), Parent( InOuter ), Obj( InObj ), Exclude( InExclude )
	{
		Obj->Serialize( *this );
	}
	UBOOL DidRef;
private:
	FArchive& operator<<( UObject*& Obj )
	{
		if( Obj && Obj->GetOuter()!=Parent )
		{
			INT i;
			for( i=0; i<Exclude.Num(); i++ )
				if( Exclude(i) == Obj->GetOuter() )
					break;
			if( i==Exclude.Num() )
			{
				if( !DidRef )
					Ar.Logf( TEXT("   %s references:"), Obj->GetFullName() );
				Ar.Logf( TEXT("      %s"), Obj->GetFullName() );
				DidRef=1;
			}
		}
		return *this;
	}
	FOutputDevice& Ar;
	UObject* Parent;
	UObject* Obj;
	TArray<UObject*>& Exclude;
};

//
// Archive for finding who references an object.
//
class FArchiveFindCulprit : public FArchive
{
public:
	FArchiveFindCulprit( UObject* InFind, UObject* Src )
	: Find(InFind), Count(0)
	{
		Src->Serialize( *this );
	}
	INT GetCount()
	{
		return Count;
	}
	FArchive& operator<<( class UObject*& Obj )
	{
		if( Obj==Find )
			Count++;
		return *this;
	}
protected:
	UObject* Find;
	INT Count;
};

//
// Archive for finding shortest path from root to a particular object.
// Depth-first search.
//
struct FTraceRouteRecord
{
	INT Depth;
	UObject* Referencer;
	FTraceRouteRecord( INT InDepth, UObject* InReferencer )
	: Depth(InDepth), Referencer(InReferencer)
	{}
};
class FArchiveTraceRoute : public FArchive
{
public:
	static TArray<UObject*> FindShortestRootPath( UObject* Obj )
	{
		TMap<UObject*,FTraceRouteRecord> Routes;
		FArchiveTraceRoute Rt( Routes );
		TArray<UObject*> Result;
		if( Routes.Find(Obj) )
		{
			Result.AddItem( Obj );
			for( ; ; )
			{
				FTraceRouteRecord* Rec = Routes.Find(Obj);
				if( Rec->Depth==0 )
					break;
				Obj = Rec->Referencer;
				Result.Insert(0);
				Result(0) = Obj;
			}
		}
		return Result;
	}
private:
	FArchiveTraceRoute( TMap<UObject*,FTraceRouteRecord>& InRoutes )
	: Routes(InRoutes), Depth(0), Prev(NULL)
	{
		{for( FObjectIterator It; It; ++It )
			It->SetFlags( RF_TagExp );}
		UObject::SerializeRootSet( *this, RF_Native, 0 );
		{for( FObjectIterator It; It; ++It )
			It->ClearFlags( RF_TagExp );}
	}
	FArchive& operator<<( class UObject*& Obj )
	{
		if( Obj )
		{
			FTraceRouteRecord* Rec = Routes.Find(Obj);
			if( !Rec || Depth<Rec->Depth )
				Routes.Set( Obj, FTraceRouteRecord(Depth,Prev) );
		}
		if( Obj && (Obj->GetFlags() & RF_TagExp) )
		{
			Obj->ClearFlags( RF_TagExp );
			UObject* SavedPrev = Prev;
			Prev = Obj;
			Depth++;
			Obj->Serialize( *this );
			Depth--;
			Prev = SavedPrev;
		}
		return *this;
	}
	TMap<UObject*,FTraceRouteRecord>& Routes;
	INT Depth;
	UObject* Prev;
};

//
// Show the inheretance graph of all loaded classes.
//
static void ShowClasses( UClass* Class, FOutputDevice& Ar, int Indent )
{
	Ar.Logf( TEXT("%s%s"), appSpc(Indent), Class->GetName() );
	for( TObjectIterator<UClass> It; It; ++It )
		if( It->GetSuperClass() == Class )
			ShowClasses( *It, Ar, Indent+2 );
}

struct FItem
{
	UClass*	Class;
	INT		Count;
	SIZE_T	Num, Max;
	FItem( UClass* InClass=NULL )
	: Class(InClass), Count(0), Num(0), Max(0)
	{}
	void Add( FArchiveCountMem& Ar )
	{Count++; Num+=Ar.GetNum(); Max+=Ar.GetMax();}
};
struct FSubItem
{
	UObject* Object;
	SIZE_T Num, Max;
	FSubItem( UObject* InObject, SIZE_T InNum, SIZE_T InMax )
	: Object( InObject ), Num( InNum ), Max( InMax )
	{}
};
static QSORT_RETURN CDECL CompareSubItems( const FSubItem* A, const FSubItem* B )
{
	return B->Max - A->Max;
}
static QSORT_RETURN CDECL CompareItems( const FItem* A, const FItem* B )
{
	return B->Max - A->Max;
}

UBOOL UObject::StaticExec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	const TCHAR *Str = Cmd;
	if( ParseCommand(&Str,TEXT("MEM")) )
	{
		GMalloc->DumpAllocs();
		return 1;
	}
	else if( ParseCommand(&Str,TEXT("DUMPNATIVES")) )
	{
		// Linux: Defined out because of error: "taking
		// the address of a bound member function to form
		// a pointer to member function, say '&Object::execUndefined'
		#if _MSC_VER
		for( INT i=0; i<EX_Max; i++ )
			if( GNatives[i] == &execUndefined )
				debugf( TEXT("Native index %i is available"), i );
		#endif
		return 1;
	}
	else if( ParseCommand(&Str,TEXT("GET")) )
	{
		// Get a class default variable.
		TCHAR ClassName[256], PropertyName[256];
		UClass* Class;
		UProperty* Property;
		if
		(	ParseToken( Str, ClassName, ARRAY_COUNT(ClassName), 1 )
		&&	(Class=FindObject<UClass>( ANY_PACKAGE, ClassName))!=NULL )
		{
			if
			(	ParseToken( Str, PropertyName, ARRAY_COUNT(PropertyName), 1 )
			&&	(Property=FindField<UProperty>( Class, PropertyName))!=NULL )
			{
				TCHAR Temp[256]=TEXT("");
				if( Class->Defaults[CPD_Normal].Num() )
					Property->ExportText( 0, Temp, &Class->Defaults[CPD_Normal](0), &Class->Defaults[CPD_Normal](0), PPF_Localized );
				Ar.Log( Temp );
			}
			else Ar.Logf( NAME_ExecWarning, TEXT("Unrecognized property %s"), PropertyName );
		}
		else Ar.Logf( NAME_ExecWarning, TEXT("Unrecognized class %s"), ClassName );
		return 1;
	}
	else if( ParseCommand(&Str,TEXT("SET")) )
	{
		// Set a class default variable.
		TCHAR ClassName[256], PropertyName[256];
		UClass* Class;
		UProperty* Property;
		if
		(	ParseToken( Str, ClassName, ARRAY_COUNT(ClassName), 1 )
		&&	(Class=FindObject<UClass>( ANY_PACKAGE, ClassName))!=NULL )
		{
			if
			(	ParseToken( Str, PropertyName, ARRAY_COUNT(PropertyName), 1 )
			&&	(Property=FindField<UProperty>( Class, PropertyName))!=NULL )
			{
				while( *Str==' ' )
					Str++;
				GlobalSetProperty( Str, Class, Property, Property->Offset, 1 );
			}
			else Ar.Logf( NAME_ExecWarning, TEXT("Unrecognized property %s"), PropertyName );
		}
		else Ar.Logf( NAME_ExecWarning, TEXT("Unrecognized class %s"), ClassName );
		return 1;
	}
	else if( ParseCommand(&Str,TEXT("OBJ")) )
	{
		if( ParseCommand(&Str,TEXT("GARBAGE")) )
		{
			// Purge unclaimed objects.
			UBOOL GSavedNoGC=GNoGC;
			GNoGC = 0;
			CollectGarbage( RF_Native | (GIsEditor ? RF_Standalone : 0) );
			GNoGC = GSavedNoGC;
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("MARK")) )
		{
			debugf( TEXT("Marking objects") );
			for( FObjectIterator It; It; ++It )
				It->SetFlags( RF_Marked );
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("MARKCHECK")) )
		{
			debugf( TEXT("Unmarked objects:") );
			for( FObjectIterator It; It; ++It )
				if( !(It->GetFlags() & RF_Marked) )
					debugf( TEXT("%s"), It->GetFullName() );
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("REFS")) )
		{
			UClass* Class;
			UObject* Object;
			if
			(	ParseObject<UClass>( Str, TEXT("CLASS="), Class, ANY_PACKAGE )
			&&	ParseObject(Str,TEXT("NAME="), Class, Object, ANY_PACKAGE ) )
			{
				Ar.Logf( TEXT("") );
				Ar.Logf( TEXT("Referencers of %s:"), Object->GetFullName() );
				for( FObjectIterator It; It; ++It )
				{
					FArchiveFindCulprit ArFind(Object,*It);
					if( ArFind.GetCount() )
						Ar.Logf( TEXT("   %s"), It->GetFullName() );
				}
				Ar.Logf(TEXT("") );
				Ar.Logf(TEXT("Shortest reachability from root to %s:"), Object->GetFullName() );
				TArray<UObject*> Rt = FArchiveTraceRoute::FindShortestRootPath(Object);
				for( INT i=0; i<Rt.Num(); i++ )
					Ar.Logf(TEXT("   %s%s"), Rt(i)->GetFullName(), i!=0 ? TEXT("") : (Rt(i)->GetFlags()&RF_Native)?TEXT(" (native)"):TEXT(" (root)") );
				if( !Rt.Num() )
					Ar.Logf(TEXT("   (Object is not currently rooted)"));
				Ar.Logf(TEXT("") );
			}
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("HASH")) )
		{
			// Hash info.
			FName::DisplayHash( Ar );
			INT ObjCount=0, HashCount=0;
			for( FObjectIterator It; It; ++It )
				ObjCount++;
			for( INT i=0; i<ARRAY_COUNT(GObjHash); i++ )
			{
				INT c=0;
				for( UObject* Hash=GObjHash[i]; Hash; Hash=Hash->HashNext )
					c++;
				if( c )
					HashCount++;
				//debugf( "%i: %i", i, c );
			}
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("CLASSES")) )
		{
			ShowClasses( StaticClass(), Ar, 0 );
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("DEPENDENCIES")) )
		{
			UPackage* Pkg;
			if( ParseObject<UPackage>(Str,TEXT("PACKAGE="),Pkg,NULL) )
			{
				TArray<UObject*> Exclude;
				for( int i=0; i<16; i++ )
				{
					TCHAR Temp[32];
					appSprintf( Temp, TEXT("EXCLUDE%i="), i );
					FName F;
					if( Parse(Str,Temp,F) )
						Exclude.AddItem( CreatePackage(NULL,*F) );
				}
				Ar.Logf( TEXT("Dependencies of %s:"), Pkg->GetPathName() );
				for( FObjectIterator It; It; ++It )
					if( It->GetOuter()==Pkg )
						FArchiveShowReferences ArShowReferences( Ar, Pkg, *It, Exclude );
			}
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("LIST")) )
		{
			Ar.Log( TEXT("Objects:") );
			Ar.Log( TEXT("") );
			UClass*   CheckType     = NULL;
			UPackage* CheckPackage  = NULL;
			UPackage* InsidePackage = NULL;
			ParseObject<UClass>  ( Str, TEXT("CLASS="  ), CheckType,     ANY_PACKAGE );
			ParseObject<UPackage>( Str, TEXT("PACKAGE="), CheckPackage,  NULL );
			ParseObject<UPackage>( Str, TEXT("INSIDE=" ), InsidePackage, NULL );
			TArray<FItem> List;
			TArray<FSubItem> Objects;
			FItem Total;
			for( FObjectIterator It; It; ++It )
			{
				if
				(	(CheckType     && It->IsA(CheckType))
				||	(CheckPackage  && It->GetOuter()==CheckPackage)
				||	(InsidePackage && It->IsIn(InsidePackage))
				||	(!CheckType && !CheckPackage && !InsidePackage) )
				{
					FArchiveCountMem Count( *It );
					INT i;
					for( i=0; i<List.Num(); i++ )
						if( List(i).Class == It->GetClass() )
							break;
					if( i==List.Num() )
						i = List.AddItem(FItem( It->GetClass() ));
					if( CheckType || CheckPackage || InsidePackage )
						new(Objects)FSubItem( *It, Count.GetNum(), Count.GetMax() );
					List(i).Add( Count );
					Total.Add( Count );
				}
			}
			if( Objects.Num() )
			{
				appQsort( &Objects(0), Objects.Num(), sizeof(Objects(0)), (QSORT_COMPARE)CompareSubItems );
				Ar.Logf( TEXT("%60s % 10s % 10s"), TEXT("Object"), TEXT("NumBytes"), TEXT("MaxBytes") );
				for( INT i=0; i<Objects.Num(); i++ )
					Ar.Logf( TEXT("%60s % 10i % 10i"), Objects(i).Object->GetFullName(), Objects(i).Num, Objects(i).Max );
				Ar.Log( TEXT("") );
			}
			if( List.Num() )
			{
				appQsort( &List(0), List.Num(), sizeof(List(0)), (QSORT_COMPARE)CompareItems );
				Ar.Logf(TEXT(" %30s % 6s % 10s  % 10s "), TEXT("Class"), TEXT("Count"), TEXT("NumBytes"), TEXT("MaxBytes") );
				for( INT i=0; i<List.Num(); i++ )
					Ar.Logf(TEXT(" %30s % 6i % 10iK % 10iK"), List(i).Class->GetName(), List(i).Count, List(i).Num/1024, List(i).Max/1024 );
				Ar.Log( TEXT("") );
			}
			Ar.Logf( TEXT("%i Objects (%.3fM / %.3fM)"), Total.Count, (FLOAT)Total.Num/1024.0/1024.0, (FLOAT)Total.Max/1024.0/1024.0 );
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("VFHASH")) )
		{
			Ar.Logf( TEXT("Class VfHashes:") );
			for( TObjectIterator<UState> It; It; ++It )
			{
				Ar.Logf( TEXT("%s:"), It->GetName() );
				for( INT i=0; i<UField::HASH_COUNT; i++ )
				{
					INT c=0;
					for( UField* F=It->VfHash[i]; F; F=F->HashNext )
						c++;
					Ar.Logf( TEXT("   %i: %i"), i, c );
				}
			}
			return 1;
		}
		else if( ParseCommand(&Str,TEXT("LINKERS")) )
		{
			Ar.Logf( TEXT("Linkers:") );
			for( INT i=0; i<GObjLoaders.Num(); i++ )
			{
				ULinkerLoad* Linker = CastChecked<ULinkerLoad>( GObjLoaders(i) );
				INT NameSize = 0;
				for( INT i=0; i<Linker->NameMap.Num(); i++ )
					if( Linker->NameMap(i) != NAME_None )
						NameSize += sizeof(FNameEntry) - (NAME_SIZE - appStrlen(*Linker->NameMap(i)) - 1) * sizeof(TCHAR);
				Ar.Logf
				(
					TEXT("%s (%s): Names=%i (%iK/%iK) Imports=%i (%iK) Exports=%i (%iK) Gen=%i Lazy=%i"),
					*Linker->Filename,
					Linker->LinkerRoot->GetFullName(),
					Linker->NameMap.Num(),
					Linker->NameMap.Num() * sizeof(FName) / 1024,
					NameSize / 1024,
					Linker->ImportMap.Num(),
					Linker->ImportMap.Num() * sizeof(FObjectImport) / 1024,
					Linker->ExportMap.Num(),
					Linker->ExportMap.Num() * sizeof(FObjectExport) / 1024,
					Linker->Summary.Generations.Num(),
					Linker->LazyLoaders.Num()
				);
			}
			return 1;
		}
		else return 0;
	}
	else if( ParseCommand(&Str,TEXT("GTIME")) )
	{
		debugf( TEXT("GTime = %f"), GTempDouble );
		return 1;
	}
	else return 0; // Not executed

}

/*-----------------------------------------------------------------------------
   File loading.
-----------------------------------------------------------------------------*/

//
// Safe load error-handling.
//
void UObject::SafeLoadError( DWORD LoadFlags, const TCHAR* Error, const TCHAR* Fmt, ... )
{
	// Variable arguments setup.
	TCHAR TempStr[4096];
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), Fmt );

	if( !(LoadFlags & LOAD_Quiet)  ) debugf( NAME_Warning, TempStr );
	if(   LoadFlags & LOAD_Throw   ) appThrowf( TEXT("%s"), Error   );
	if(   LoadFlags & LOAD_NoFail  ) appErrorf( TEXT("%s"), TempStr );
	if( !(LoadFlags & LOAD_NoWarn) ) GWarn->Logf( TEXT("%s"), TempStr );
}


//
// Find or create the linker for a package.
//
ULinkerLoad* UObject::GetPackageLinker
(
	UObject*		InOuter,
	const TCHAR*	InFilename,
	DWORD			LoadFlags,
	UPackageMap*	Sandbox,
	FGuid*			CompatibleGuid
)
{
	check(GObjBeginLoadCount);

	// See if there is already a linker for this package.
	ULinkerLoad* Result = NULL;
	if( InOuter )
		for( INT i=0; i<GObjLoaders.Num() && !Result; i++ )
			if( GetLoader(i)->LinkerRoot == InOuter )
				Result = GetLoader( i );

	// Try to load the linker.
	try
	{
		// See if the linker is already loaded.
		TCHAR NewFilename[256]=TEXT("");
		if( Result )
		{
			// Linker already found.
			appStrcpy( NewFilename, TEXT("") );
		}
		else if( !InFilename )
		{
			// Resolve filename from package name.
			if( !InOuter )
				appThrowf( LocalizeError("PackageResolveFailed") );
			if( !appFindPackageFile( InOuter->GetName(), CompatibleGuid, NewFilename ) )
			{
				// See about looking in the dll.
				if( (LoadFlags & LOAD_AllowDll) && InOuter->IsA(UPackage::StaticClass()) && ((UPackage*)InOuter)->DllHandle )
					return NULL;
				GWarn->Logf( TEXT("Failed resolving: %s"), StaticClass()->GetFullName() );
				appThrowf( LocalizeError("PackageNotFound"), InOuter->GetName() );
			}
		}
		else
		{
			// Verify that the file exists.
			if( !appFindPackageFile( InFilename, CompatibleGuid, NewFilename ) )
				appThrowf( LocalizeError("FileNotFound"), InFilename );

			// Resolve package name from filename.
			TCHAR Tmp[256], *T=Tmp;
			appStrcpy( Tmp, InFilename );
	
			while( 1 )
			{
				if( appStrchr(T,PATH_SEPARATOR[0]) )
					T = appStrchr(T,PATH_SEPARATOR[0])+1;
				else if( appStrchr(T,'/') )
					T = appStrchr(T,'/')+1;
				else if( appStrchr(T,':') )
					T = appStrchr(T,':')+1;
				else
					break;
			}
			if( appStrchr(T,'.') )
				*appStrchr(T,'.') = 0;

			UPackage* FilenamePkg = CreatePackage( NULL, T );

			// If no package specified, use package from file.
			if( InOuter==NULL )
			{
				if( !FilenamePkg )
					appThrowf( LocalizeError("FilenameToPackage"), InFilename );
				InOuter = FilenamePkg;
				for( INT i=0; i<GObjLoaders.Num() && !Result; i++ )
					if( GetLoader(i)->LinkerRoot == InOuter )
						Result = GetLoader(i);

			#if DNF
				// JEP...: If package file name is different, reset the loader
				if (Result)
				{
					if (appStrcmp(*Result->Filename, NewFilename))
					{
						debugf( TEXT("Resetting loader for package (%s, %s)"), *Result->Filename, NewFilename);
						ResetLoaders( InOuter, 0, 1 );
						Result = NULL;
					}
				}
				// ... JEP
			#endif
			}
			else if( InOuter != FilenamePkg )//!!should be tested and validated in new UnrealEd
			{
				// Loading a new file into an existing package, so reset the loader.
				debugf( TEXT("New File, Existing Package (%s, %s)"), InOuter->GetFullName(), FilenamePkg->GetFullName() );
				ResetLoaders( InOuter, 0, 1 );
			}
		}

		// Make sure the package is accessible in the sandbox.
		if( Sandbox && !Sandbox->SupportsPackage(InOuter) )
			appThrowf( LocalizeError("Sandbox"), InOuter->GetName() );

#if DNF
		// CDH: If this package has a merging extension, set the merge flag
		FString MergeExtStr;
		if (GConfig->GetString(TEXT("DNFTempMerge"), TEXT("TempMergeExt"), MergeExtStr))
		{
			TCHAR* fileExt = (TCHAR*)appFExt(NewFilename);
			if (fileExt && (!appStricmp(*MergeExtStr, fileExt)))
				LoadFlags |= LOAD_TempMerge;
		}
#endif

		// Create new linker.
		if( !Result )
			Result = new ULinkerLoad( InOuter, NewFilename, LoadFlags );

		// Verify compatibility.
		if( CompatibleGuid && Result->Summary.Guid!=*CompatibleGuid )
			appThrowf( LocalizeError("PackageVersion"), InOuter->GetName() );
	}
	catch( const TCHAR* Error )
	{
		SafeLoadError( LoadFlags, Error, LocalizeError("FailedLoad"), InFilename ? InFilename : InOuter ? InOuter->GetName() : TEXT("NULL"), Error );
	}

	// Success.
	return Result;
}

//
// Find or optionally create a package.
//
UPackage* UObject::CreatePackage( UObject* InOuter, const TCHAR* InName )
{

	ResolveName( InOuter, InName, 1, 0 );
	UPackage* Result = FindObject<UPackage>( InOuter, InName );
	if( !Result )
		Result = new( InOuter, InName, RF_Public )UPackage;
	return Result;

}

//
// Resolve a package and name.
//
UBOOL UObject::ResolveName( UObject*& InPackage, const TCHAR*& InName, UBOOL Create, UBOOL Throw )
{
	check(InName);

	// See if the name is specified in the .ini file.
	UBOOL SystemIni = (appStrnicmp( InName, TEXT("ini:"), 4 )==0);
	UBOOL UserIni   = (appStrnicmp( InName, TEXT("usr:"), 4 )==0);
	if( (SystemIni||UserIni) && appStrlen(InName)<1024 && appStrchr(InName,'.') )
	{
		// Get .ini key and section.
		TCHAR Section[256];
		appStrcpy( Section, InName+4 );
		TCHAR* Key = Section;
		while( appStrchr(Key,'.') )
			Key = appStrchr(Key,'.')+1;
		check(Key!=Section);
		Key[-1] = 0;

		// Look up name.
		TCHAR* Temp = appStaticString1024();
		if( !GConfig->GetString( Section, Key, Temp, 1024, SystemIni?TEXT("System"):TEXT("User") ) )
		{
			if( Throw )
				appThrowf( LocalizeError("ConfigNotFound"), InName );
			return 0;
		}
		InName = Temp;
	}

	// Handle specified packages.
	while( appStrchr(InName,'.') )
	{
		TCHAR PartialName[256];
		appStrcpy( PartialName, InName );
		*appStrchr( PartialName, '.' ) = 0;
		if( Create )
		{
			InPackage = CreatePackage( InPackage, PartialName );
		}
		else
		{
			UObject* NewPackage = FindObject<UPackage>( InPackage, PartialName );
			if( !NewPackage )
			{
				NewPackage = FindObject<UObject>( InPackage, PartialName );
				if( !NewPackage )
					return 0;
			}
			InPackage = NewPackage;
		}
		InName = appStrchr(InName,'.')+1;
	}
	return 1;
}

//
// Load an object.
//
UObject* UObject::StaticLoadObject( UClass* ObjectClass, UObject* InOuter, const TCHAR* InName, const TCHAR* Filename, DWORD LoadFlags, UPackageMap* Sandbox )
{
	guard(UObject::StaticLoadObject);
	check(ObjectClass);
	check(InName);

	// Try to load.
	UObject* Result=NULL;
	BeginLoad();
	try
	{
		// Create a new linker object which goes off and tries load the file.
		ULinkerLoad* Linker = NULL;
		ResolveName( InOuter, InName, 1, 1 );
		while( InOuter && InOuter->GetOuter() )//!!can only load top-level packages from files
			InOuter = InOuter->GetOuter();
		if( !(LoadFlags & LOAD_DisallowFiles) )
			Linker = GetPackageLinker( InOuter, Filename, LoadFlags | LOAD_Throw | LOAD_AllowDll, Sandbox, NULL );
		
		//!!this sucks because it supports wildcard sub-package matching of InName, which requires a long search.
		//!!also because linker classes require exact match
		if( Linker )
			Result = Linker->Create( ObjectClass, InName, LoadFlags, 0 );
		if( !Result )
			Result = StaticFindObject( ObjectClass, InOuter, InName );
//		if( !Result )
//			appThrowf( LocalizeError("ObjectNotFound"), ObjectClass->GetName(), InOuter ? InOuter->GetPathName() : TEXT("None"), InName );
		EndLoad();
	}
	catch( const TCHAR* Error )
	{
		EndLoad();
		if( InOuter && !(LoadFlags & LOAD_NoRemap) && !Filename )
		{
			// Try loading a remapped package.
			TArray<FName> Remaps;
			GObjPackageRemap->MultiFind( InOuter->GetFName(), Remaps );
			for( INT i=0; i<Remaps.Num(); i++ )
			{
				InOuter = CreatePackage( NULL, *Remaps(i) );
				Result  = StaticLoadObject( ObjectClass, InOuter, InName, Filename, LoadFlags|LOAD_NoRemap, Sandbox );
				if( Result )
					return Result;
			}
		}
		SafeLoadError( LoadFlags | LOAD_NoWarn | LOAD_Quiet, Error, LocalizeError("FailedLoadObject"), ObjectClass->GetName(), InOuter ? InOuter->GetName() : TEXT("None"), InName, Error );
	}

	return Result;
	unguardf(( TEXT("(%s %s.%s %s)"), ObjectClass->GetPathName(), InOuter ? InOuter->GetPathName() : TEXT("None"), InName, Filename ? Filename : TEXT("NULL") ));
}

//
// Load a class.
//
UClass* UObject::StaticLoadClass( UClass* BaseClass, UObject* InOuter, const TCHAR* InName, const TCHAR* Filename, DWORD LoadFlags, UPackageMap* Sandbox )
{
	check(BaseClass);
	try
	{
		UClass* Class = LoadObject<UClass>( InOuter, InName, Filename, LoadFlags | LOAD_Throw, Sandbox );
		if( Class && !Class->IsChildOf(BaseClass) )
			appThrowf( LocalizeError("LoadClassMismatch"), Class->GetFullName(), BaseClass->GetFullName() );
		return Class;
	}
	catch( const TCHAR* Error )
	{
		// Failed.
		SafeLoadError( LoadFlags, Error, Error );
		return NULL;
	}
}

//
// Load all objects in a package.
//
UObject* UObject::LoadPackage( UObject* InOuter, const TCHAR* Filename, DWORD LoadFlags )
{
	UObject* Result;

	// Try to load.
	BeginLoad();
	try
	{
		// Create a new linker object which goes off and tries load the file.
		ULinkerLoad* Linker = GetPackageLinker( InOuter, Filename ? Filename : InOuter->GetName(), LoadFlags | LOAD_Throw, NULL, NULL );

		if( !(LoadFlags & LOAD_Verify) )
			Linker->LoadAllObjects();

		Result = Linker->LinkerRoot;
		EndLoad();
	}
	catch( const TCHAR* Error )
	{
		EndLoad();
		SafeLoadError( LoadFlags, Error, LocalizeError("FailedLoadPackage"), Error );
		Result = NULL;
	}
	return Result;
}

//
// Verify a linker.
//
void UObject::VerifyLinker( ULinkerLoad* Linker )
{
	Linker->Verify();
}

//
// Begin loading packages.
//warning: Objects may not be destroyed between BeginLoad/EndLoad calls.
//
void UObject::BeginLoad()
{
	if( ++GObjBeginLoadCount == 1 )
	{
		// Validate clean load state.
		check(GObjLoaded.Num()==0);
		check(!GAutoRegister);
		for( INT i=0; i<GObjLoaders.Num(); i++ )
			check(GetLoader(i)->Success);
	}
}

//
// End loading packages.
//
void UObject::EndLoad()
{
	check(GObjBeginLoadCount>0);
	if( --GObjBeginLoadCount == 0 )
	{
		try
		{
			// Finish loading everything.
			//warning: Array may expand during iteration.
			debugfSlow( NAME_DevLoad, TEXT("Loading objects...") );
			for( INT i=0; i<GObjLoaded.Num(); i++ )
			{
				// Preload.
				UObject* Obj = GObjLoaded(i);
				{
					if( Obj->GetFlags() & RF_NeedLoad )
					{
						check(Obj->GetLinker());
						Obj->GetLinker()->Preload( Obj );
					}
				}
			}

			// Postload objects.
			INT OriginalNum = GObjLoaded.Num();
			for( i=0; i<GObjLoaded.Num(); i++ )
				GObjLoaded(i)->ConditionalPostLoad();
			check(GObjLoaded.Num()==OriginalNum);
			GObjLoaded.Empty();

			// Dissociate all linker import object references, since they 
			// may be destroyed, causing their pointers to become invalid.
			if( GImportCount )
			{
				for( INT i=0; i<GObjLoaders.Num(); i++ )
				{
					for( INT j=0; j<GetLoader(i)->ImportMap.Num(); j++ )
					{
						FObjectImport& Import = GetLoader(i)->ImportMap(j);
						if( Import.XObject && !(Import.XObject->GetFlags() & RF_Native) )
							Import.XObject = NULL;
					}
				}
			}
			GImportCount=0;
		}
		catch( const TCHAR* Error )
		{
			// Any errors here are fatal.
			appErrorf( Error );
		}
	}
}

//
// Empty the loaders.
//
void UObject::ResetLoaders( UObject* Pkg, UBOOL DynamicOnly, UBOOL ForceLazyLoad )
{
	// JEP ... 
	//	Make sure we clean up any packages that are linked together
	//		This is the little hack where saved game packages can import
	//		their models from the original package.  In this case the saved game
	//		package is the parent, and the original package is the child...
	ULinkerLoad		*Parent = NULL, *Child = NULL;

	{for( INT i=GObjLoaders.Num()-1; i>=0; i-- )
	{
		ULinkerLoad* Linker = CastChecked<ULinkerLoad>( GetLoader(i) );

		if (Linker->ChildLinker)
		{
			check(!Child);					// There should only be 1 child/parent relationship at a time (cause we always destroy them between level loads)
			Child = Linker->ChildLinker;
		}
		else if (Linker->ParentLinker)
		{
			check(!Parent);					// There should only be 1 child/parent relationship at a time (cause we always destroy them between level loads)
			Parent = Linker->ParentLinker;
		}
	}}

	check((!Child && !Parent) || (Child && Parent));		// Both must be set, or both must be NULL

	if (Parent)
	{
		check(Child->ParentLinker == Parent);	// Make sure they are properly connected
		check(!Child->ChildLinker);				// A child linker cannot have another child
		check(Parent->ChildLinker == Child);	// Make sure they are properly connected
		check(!Parent->ParentLinker);			// A parent linker cannot have another parent
				
		// Fully reset the child loader
		Child->DetachAllLazyLoaders( 1 );
		delete Child;

		// Fully reset the parent loader.
		Parent->DetachAllLazyLoaders( 1 );
		delete Parent;
	}
	// ... JEP

	for( INT i=GObjLoaders.Num()-1; i>=0; i-- )
	{
		ULinkerLoad* Linker = CastChecked<ULinkerLoad>( GetLoader(i) );
		if( Pkg==NULL || Linker->LinkerRoot==Pkg )
		{
			if( DynamicOnly )
			{
				// Reset runtime-dynamic objects.
				for( INT i=0; i<Linker->ExportMap.Num(); i++ )
				{
					UObject*& Object = Linker->ExportMap(i)._Object;
					if( Object && !(Object->GetClass()->ClassFlags & CLASS_RuntimeStatic))
						Linker->DetachExport( i );
				}
			}
			else
			{
				// Fully reset the loader.
				if( ForceLazyLoad )
					Linker->DetachAllLazyLoaders( 1 );
				delete Linker;
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	File saving.
-----------------------------------------------------------------------------*/

//
// Archive for tagging objects and names that must be exported
// to the file.  It tags the objects passed to it, and recursively
// tags all of the objects this object references.
//
class FArchiveSaveTagExports : public FArchive
{
public:
	FArchiveSaveTagExports( UObject* InOuter )
	: Parent(InOuter)
	{
		ArIsSaving = ArIsPersistent = 1;
	}
	FArchive& operator<<( UObject*& Obj )
	{
		if( Obj && Obj->IsIn(Parent) && !(Obj->GetFlags() & (RF_Transient|RF_TagExp)) )//&& !Obj->IsPendingKill() )
		if (!Obj->IsA(UModel::StaticClass()) || !GSaveLoadHack)						// JEP
		{
			// Set flags.
			Obj->SetFlags(RF_TagExp);
			if( !(Obj->GetFlags() & RF_NotForEdit  ) ) Obj->SetFlags(RF_LoadForEdit);
			if( !(Obj->GetFlags() & RF_NotForClient) ) Obj->SetFlags(RF_LoadForClient);
			if( !(Obj->GetFlags() & RF_NotForServer) ) Obj->SetFlags(RF_LoadForServer);

			// Recurse with this object's class and package.
			UClass*  Class  = Obj->GetClass();
			UObject* Parent = Obj->GetOuter();
			*this << Class << Parent;

			// Recurse with this object's children.
			Obj->Serialize( *this );
		}
		return *this;
	}
	UObject* Parent;
};

//
// QSort comparators.
//
static ULinkerSave* GTempSave;
INT CDECL LinkerNameSort( const void* A, const void* B )
{
	return GTempSave->MapName((FName*)B) - GTempSave->MapName((FName*)A);
}
INT CDECL LinkerImportSort( const void* A, const void* B )
{
	return GTempSave->MapObject(((FObjectImport*)B)->XObject) - GTempSave->MapObject(((FObjectImport*)A)->XObject);
}
INT CDECL LinkerExportSort( const void* A, const void* B )
{
	return GTempSave->MapObject(((FObjectExport*)B)->_Object) - GTempSave->MapObject(((FObjectExport*)A)->_Object);
}

//
// Archive for tagging objects and names that must be listed in the
// file's imports table.
//
class FArchiveSaveTagImports : public FArchive
{
public:
	DWORD ContextFlags;
	FArchiveSaveTagImports( ULinkerSave* InLinker, DWORD InContextFlags )
	: ContextFlags( InContextFlags ), Linker( InLinker )
	{
		ArIsSaving = ArIsPersistent = 1;
	}
	FArchive& operator<<( UObject*& Obj )
	{
		if( Obj && !Obj->IsPendingKill() )
		{
			if( !(Obj->GetFlags() & RF_Transient) || (Obj->GetFlags() & RF_Public) )
			{
				Linker->ObjectIndices(Obj->GetIndex())++;
				if( !(Obj->GetFlags() & RF_TagExp ) )
				{
					//debugf(NAME_Log,TEXT("JohnTest3 %s"), *Obj->GetFName());		// JEP
					
					Obj->SetFlags( RF_TagImp );
					if( !(Obj->GetFlags() & RF_NotForEdit  ) ) Obj->SetFlags(RF_LoadForEdit);
					if( !(Obj->GetFlags() & RF_NotForClient) ) Obj->SetFlags(RF_LoadForClient);
					if( !(Obj->GetFlags() & RF_NotForServer) ) Obj->SetFlags(RF_LoadForServer);
					UObject* Parent = Obj->GetOuter();
					if( Parent )
						*this << Parent;
				}
			}
		}
		return *this;
	}
	FArchive& operator<<( FName& Name )
	{
		Name.SetFlags( RF_TagExp | ContextFlags );
		Linker->NameIndices(Name.GetIndex())++;
		return *this;
	}
	ULinkerSave* Linker;
};

//
// Save one specific object into an Unrealfile.
//
UBOOL UObject::SavePackage( UObject* InOuter, UObject* Base, DWORD TopLevelFlags, const TCHAR* Filename, FOutputDevice* Error, ULinkerLoad* Conform )
{
	guard(UObject::SavePackage);
	check(InOuter);
	check(Filename);
	DWORD Time=0; clock(Time);

	// Tag parent flags.
	//!!should be user-controllable in UnrealEd.
	if( Cast<UPackage>( InOuter ) )
		Cast<UPackage>( InOuter )->PackageFlags |= PKG_AllowDownload;

	// Make temp file.
	TCHAR TempFilename[256];
	appStrcpy( TempFilename, Filename );
	INT c = appStrlen(TempFilename);
	while( c>0 && TempFilename[c-1]!=PATH_SEPARATOR[0] && TempFilename[c-1]!='/' && TempFilename[c-1]!=':' )
		c--;
	TempFilename[c]=0;
	appStrcat( TempFilename, TEXT("Save.tmp") );

	// Init.
	GWarn->StatusUpdatef( 0, 0, LocalizeProgress("Saving"), Filename );
	UBOOL Success = 0;

	// If we have a loader for the package, unload it to prevent conflicts.
	ResetLoaders( InOuter, 0, 1 );

	// Untag all objects and names.
	guard(Untag);
	for( FObjectIterator It; It; ++It )
		It->ClearFlags( RF_TagImp | RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
	for( INT i=0; i<FName::GetMaxNames(); i++ )
		if( FName::GetEntry(i) )
			FName::GetEntry(i)->Flags &= ~(RF_TagImp | RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer);
	unguard;

	// Export objects.
	//guard(TagExports);
	FArchiveSaveTagExports Ar( InOuter );
	if( Base )
		Ar << Base;
	for( FObjectIterator It; It; ++It )
	{
		if( (It->GetFlags() & TopLevelFlags) && It->IsIn(InOuter) )
		{
			UObject* Obj = *It;
			Ar << Obj;
		}
	}
	//unguard;

	ULinkerSave* Linker = NULL;
	try
	{
		// Allocate the linker.
		Linker = new ULinkerSave( InOuter, TempFilename );

		// Import objects and names.
		guard(TagImports);
		for( FObjectIterator It; It; ++It )
		{
			if( It->GetFlags() & RF_TagExp )
			//if ((It->GetFlags() & RF_TagExp) || It->IsA(UModel::StaticClass()))		// JEP
			{
				// Build list.
				FArchiveSaveTagImports Ar( Linker, It->GetFlags() & RF_LoadContextFlags );
				It->Serialize( Ar );
				UClass* Class = It->GetClass();
				Ar << Class;
				if( It->IsIn(GetTransientPackage()) )
					appErrorf( LocalizeError("TransientImport"), It->GetFullName() );
			}
		}
		unguard;

		// Export all relevant object, class, and package names.
		guard(ExportNames);
		for( FObjectIterator It; It; ++It )
		{
			if( It->GetFlags() & (RF_TagExp|RF_TagImp) )
			{
				It->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
				if( It->GetOuter() )
					It->GetOuter()->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
				if( It->GetFlags() & RF_TagImp )
				{
					It->Class->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
					check(It->Class->GetOuter());
					It->Class->GetOuter()->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
					// JEP: Modified so models can be imported
					if( !(It->GetFlags() & RF_Public) && It->Class->GetFName() != TEXT("Model"))
					//if( !(It->GetFlags() & RF_Public) && It->Class->GetFName() != TEXT("Model") && It->Class->GetFName() != TEXT("Brush"))
						appThrowf( LocalizeError("FailedSavePrivate"), Filename, It->GetFullName() );
				}
				else debugfSlow( NAME_DevSave, TEXT("Saving %s"), It->GetFullName() );
			}
		}
		unguard;

		// Write fixed-length file summary to overwrite later.
		guard(SaveSummary);
		if( Conform )
		{
			// Conform to previous generation of file.
			debugf( TEXT("Conformal save, relative to: %s, Generation %i"), *Conform->Filename, Conform->Summary.Generations.Num()+1 );
			Linker->Summary.Guid        = Conform->Summary.Guid;
			Linker->Summary.Generations = Conform->Summary.Generations;
		}
		else
		{
			// First generation file.
			Linker->Summary.Guid        = appCreateGuid();
			Linker->Summary.Generations = TArray<FGenerationInfo>();
		}
		new(Linker->Summary.Generations)FGenerationInfo( 0, 0 );
		*Linker << Linker->Summary;
		unguard;

		// Build NameMap.
		guard(BuildNameMap);
		Linker->Summary.NameOffset = Linker->Tell();
		for( INT i=0; i<FName::GetMaxNames(); i++ )
		{
			if( FName::GetEntry(i) )
			{
				FName Name( (EName)i );
				if( Name.GetFlags() & RF_TagExp )
					Linker->NameMap.AddItem( Name );
			}
		}
		unguard;

		// Sort names by usage count in order to maximize compression.
		guard(SortNames);
		GTempSave = Linker;
		INT FirstSort = 0;
		if( Conform )
		{
#if DO_GUARD_SLOW
			TArray<FName> Orig = Linker->NameMap;
#endif
			for( INT i=0; i<Conform->NameMap.Num(); i++ )
			{
				INT Index = Linker->NameMap.FindItemIndex(Conform->NameMap(i));
				if( Conform->NameMap(i)==NAME_None || Index==INDEX_NONE )
				{
					Linker->NameMap.Add();
					Linker->NameMap.Last() = Linker->NameMap(i);
				}
				else Exchange( Linker->NameMap(i), Linker->NameMap(Index) );
				Linker->NameMap(i) = Conform->NameMap(i);
			}
#if DO_GUARD_SLOW
			for( i=0; i<Conform->NameMap.Num(); i++ )
				check(Linker->NameMap(i)==Conform->NameMap(i));
			for( i=0; i<Orig.Num(); i++ )
				check(Linker->NameMap.FindItemIndex(Orig(i))!=INDEX_NONE);
#endif
			FirstSort = Conform->NameMap.Num();
		}
		appQsort( &Linker->NameMap(FirstSort), Linker->NameMap.Num()-FirstSort, sizeof(Linker->NameMap(0)), LinkerNameSort );
		unguard;

		// Save names.
		guard(SaveNames);
		Linker->Summary.NameCount = Linker->NameMap.Num();
		for( INT i=0; i<Linker->NameMap.Num(); i++ )
		{
			*Linker << *FName::GetEntry( Linker->NameMap(i).GetIndex() );
			Linker->NameIndices(Linker->NameMap(i).GetIndex()) = i;
		}
		unguard;

		// Build ImportMap.
		guard(BuildImportMap);

		for( FObjectIterator It; It; ++It )
			if( It->GetFlags() & RF_TagImp )
				new( Linker->ImportMap )FObjectImport( *It );

		Linker->Summary.ImportCount = Linker->ImportMap.Num();
		unguard;

		// Sort imports by usage count.
		guard(SortImports);
		GTempSave = Linker;
		appQsort( &Linker->ImportMap(0), Linker->ImportMap.Num(), sizeof(Linker->ImportMap(0)), LinkerImportSort );
		unguard;

		// Build ExportMap.
		guard(BuildExports);
		for( FObjectIterator It; It; ++It )
			if( It->GetFlags() & RF_TagExp )
				new( Linker->ExportMap )FObjectExport( *It );
		unguard;

		// Sort exports by usage count.
		guard(SortExports);
		INT FirstSort = 0;
		if( Conform )
		{
			TArray<FObjectExport> Orig = Linker->ExportMap;
			Linker->ExportMap.Empty( Linker->ExportMap.Num() );
			TArray<BYTE> Used; Used.AddZeroed( Orig.Num() );
			TMap<FString,INT> Map;
			{for( INT i=0; i<Orig.Num(); i++ )
				Map.Set( Orig(i)._Object->GetFullName(), i );}
			{for( INT i=0; i<Conform->ExportMap.Num(); i++ )
			{
				INT* Find = Map.Find( Conform->GetExportFullName(i,Linker->LinkerRoot->GetPathName()) );
				if( Find )
				{
					new(Linker->ExportMap)FObjectExport( Orig(*Find) );
					check(Linker->ExportMap.Last()._Object == Orig(*Find)._Object);
					Used( *Find ) = 1;
				}
				else new(Linker->ExportMap)FObjectExport( NULL );
			}}
			FirstSort = Conform->ExportMap.Num();
			{for( INT i=0; i<Used.Num(); i++ )
				if( !Used(i) )
					new(Linker->ExportMap)FObjectExport( Orig(i) );}
#if DO_GUARD_SLOW
			{for( INT i=0; i<Orig.Num(); i++ )
			{
				for( INT j=0; j<Linker->ExportMap.Num(); j++ )
					if( Linker->ExportMap(j)._Object == Orig(i)._Object )
						break;
				check(j<Linker->ExportMap.Num());
			}}
#endif
		}
		appQsort( &Linker->ExportMap(FirstSort), Linker->ExportMap.Num()-FirstSort, sizeof(Linker->ExportMap(0)), LinkerExportSort );
		Linker->Summary.ExportCount = Linker->ExportMap.Num();
		unguard;



		// Set linker reverse mappings.
		guard(SetLinkerMappings);
		{for( INT i=0; i<Linker->ExportMap.Num(); i++ )
			if( Linker->ExportMap(i)._Object )
				Linker->ObjectIndices(Linker->ExportMap(i)._Object->GetIndex()) = i+1;}
		{for( INT i=0; i<Linker->ImportMap.Num(); i++ )
			Linker->ObjectIndices(Linker->ImportMap(i).XObject->GetIndex()) = -i-1;}
		unguard;

		// Save exports.
		guard(SaveExports);
		for( INT i=0; i<Linker->ExportMap.Num(); i++ )
		{
			FObjectExport& Export = Linker->ExportMap(i);
			if( Export._Object )
			{
				// Set class index.
				if( !Export._Object->IsA(UClass::StaticClass()) )
				{
					Export.ClassIndex = Linker->ObjectIndices(Export._Object->GetClass()->GetIndex());
					check(Export.ClassIndex!=0);
				}
				if( Export._Object->IsA(UStruct::StaticClass()) )
				{
					UStruct* Struct = (UStruct*)Export._Object;
					if( Struct->SuperField )
					{
						Export.SuperIndex = Linker->ObjectIndices(Struct->SuperField->GetIndex());
						check(Export.SuperIndex!=0);	

					}
				}

				// Set package index.
				if( Export._Object->GetOuter() != InOuter )
				{
					check(Export._Object->GetOuter()->IsIn(InOuter));
					Export.PackageIndex = Linker->ObjectIndices(Export._Object->GetOuter()->GetIndex());
					check(Export.PackageIndex>0);
				}

				// Save it.
				Export.SerialOffset = Linker->Tell();
				Export._Object->Serialize( *Linker );
				Export.SerialSize = Linker->Tell() - Linker->ExportMap(i).SerialOffset;
			}
		}
		unguard;

		// Save the import map.
		guard(SaveImportMap);
		Linker->Summary.ImportOffset = Linker->Tell();
		for( INT i=0; i<Linker->ImportMap.Num(); i++ )
		{
			FObjectImport& Import = Linker->ImportMap( i );

			// Set the package index.
			if( Import.XObject->GetOuter() )
			{
				//check(!Import.XObject->GetOuter()->IsIn(InOuter));	// JEP
				check(!Import.XObject->GetOuter()->IsIn(InOuter) || Import.XObject->IsA(UModel::StaticClass()));	// JEP
				Import.PackageIndex = Linker->ObjectIndices(Import.XObject->GetOuter()->GetIndex());
				check(Import.PackageIndex<0);
			}

			// Save it.
			*Linker << Import;
		}
		unguard;

		// Save the export map.
		guard(SaveExportMap);
		Linker->Summary.ExportOffset = Linker->Tell();
		for( int i=0; i<Linker->ExportMap.Num(); i++ )
			*Linker << Linker->ExportMap( i );
		unguard;

		// Rewrite updated file summary.
		guard(RewriteSummary);
		GWarn->StatusUpdatef( 0, 0, TEXT("%s"), LocalizeProgress("Closing") );
		Linker->Summary.Generations.Last().ExportCount = Linker->Summary.ExportCount;
		Linker->Summary.Generations.Last().NameCount   = Linker->Summary.NameCount;
		Linker->Seek(0);
		*Linker << Linker->Summary;
		unguard;

		Success = 1;
	}
	catch( const TCHAR* Msg )
	{
		// Delete the temporary file.
		GFileManager->Delete( TempFilename );
		Error->Logf( NAME_Warning, TEXT("%s"), Msg );
	}
	if( Linker )
	{
		delete Linker;
	}
	unclock(Time);
	debugf( NAME_Log, TEXT("Save=%f"), GSecondsPerCycle*1000*Time );
	if( Success )
	{
		// Move the temporary file.
		debugf( NAME_Log, TEXT("Moving '%s' to '%s'"), TempFilename, Filename );
		if( !GFileManager->Move( Filename, TempFilename ) )
		{
			GFileManager->Delete( TempFilename );
			GWarn->Logf( LocalizeError("SaveWarning"), Filename );
			Success = 0;
		}
	}
	return Success;
	unguard;
}

/*-----------------------------------------------------------------------------
	Misc.
-----------------------------------------------------------------------------*/

//
// Return whether statics are initialized.
//
UBOOL UObject::GetInitialized()
{
	return UObject::GObjInitialized;
}

//
// Return the static transient package.
//
UPackage* UObject::GetTransientPackage()
{
	return UObject::GObjTransientPkg;
}

//
// Return the ith loader.
//
ULinkerLoad* UObject::GetLoader( INT i )
{
	return (ULinkerLoad*)GObjLoaders(i);
}

//
// Add an object to the root array. This prevents the object and all
// its descendents from being deleted during garbage collection.
//
void UObject::AddToRoot()
{
	GObjRoot.AddItem( this );
}

//
// Remove an object from the root array.
//
void UObject::RemoveFromRoot()
{
	GObjRoot.RemoveItem( this );
}

/*-----------------------------------------------------------------------------
	Object name functions.
-----------------------------------------------------------------------------*/

//
// Create a unique name by combining a base name and an arbitrary number string.  
// The object name returned is guaranteed not to exist.
//
FName UObject::MakeUniqueObjectName( UObject* Parent, UClass* Class )
{
	check(Class);

	TCHAR NewBase[NAME_SIZE], Result[NAME_SIZE];
	TCHAR TempIntStr[NAME_SIZE];

	appStrcpy( NewBase, Class->GetName() );
	appStrcat( NewBase, TEXT("___") );

	// Append numbers to base name.
	do
	{
		appSprintf( TempIntStr, TEXT("%i"), Class->ClassUnique++ );
		appStrncpy( Result, NewBase, NAME_SIZE-appStrlen(TempIntStr)-1 );
		appStrcat( Result, TempIntStr );
	} while( StaticFindObject( NULL, Parent, Result ) );

	return Result;
}

/*-----------------------------------------------------------------------------
	Object hashing.
-----------------------------------------------------------------------------*/

//
// Add an object to the hash table.
//
void UObject::HashObject()
{

	INT iHash       = GetObjectHash( Name, Outer ? Outer->GetIndex() : 0 );
	HashNext        = GObjHash[iHash];
	GObjHash[iHash] = this;

}

//
// Remove an object from the hash table.
//
void UObject::UnhashObject( INT OuterIndex )
{
	INT       iHash   = GetObjectHash( Name, OuterIndex );
	UObject** Hash    = &GObjHash[iHash];
	INT       Removed = 0;
	while( *Hash != NULL )
	{
		if( *Hash != this )
		{
			Hash = &(*Hash)->HashNext;
 		}
		else
		{
			*Hash = (*Hash)->HashNext;
			Removed++;
		}
	}
	check(Removed!=0);
	check(Removed==1);
}

/*-----------------------------------------------------------------------------
	Creating and allocating data for new objects.
-----------------------------------------------------------------------------*/

//
// Add an object to the table.
//
void UObject::AddObject( INT InIndex )
{

	// Find an available index.
	if( InIndex==INDEX_NONE )
	{
		if( GObjAvailable.Num() )
		{
			InIndex = GObjAvailable.Pop();
			check(GObjObjects(InIndex)==NULL);
		}
		else InIndex = GObjObjects.Add();
	}

	// Add to global table.
	GObjObjects(InIndex) = this;
	Index = InIndex;
	HashObject();

}

//
// Create a new object or replace an existing one.
// If Name is NAME_None, tries to create an object with an arbitrary unique name.
// No failure conditions.
//
UObject* UObject::StaticAllocateObject
(
	UClass*			InClass,
	UObject*		InOuter,
	FName			InName,
	DWORD			InFlags,
	UObject*		InTemplate,
	FOutputDevice*	Error,
	UObject*		Ptr
)
{
	guard(UObject::StaticAllocateObject);
	check(Error);
	check(!InClass || InClass->ClassWithin);
	check(!InClass || InClass->ClassConstructor);

	// Validation checks.
	if( !InClass )
	{
		Error->Logf( TEXT("Empty class for object %s"), *InName );
		return NULL;
	}
	if( InClass->GetIndex()==INDEX_NONE && GObjRegisterCount==0 )
	{
		Error->Logf( TEXT("Unregistered class for %s"), *InName );
		return NULL;
	}
	if( InClass->ClassFlags & CLASS_Abstract )
	{
		Error->Logf( LocalizeError("Abstract"), *InName, InClass->GetName() );
		return NULL;
	}
	if( !InOuter && InClass!=UPackage::StaticClass() )
	{
		Error->Logf( LocalizeError("NotPackaged"), InClass->GetName(), *InName );
		return NULL;
	}
	if( InOuter && !InOuter->IsA(InClass->ClassWithin) )
	{
		Error->Logf( LocalizeError("NotWithin"), InClass->GetName(), *InName, InOuter->GetClass()->GetName(), InClass->ClassWithin->GetName() );
		return NULL;
	}
	// CDH...
	if( InClass->ClassFlags & CLASS_Obsolete )
	{
		GWarn->Logf( TEXT("\n!!OBSOLETE!! Class %s is obsolete; please remove object(s) of this class as soon as possible."), InClass->GetFullName() );
	}

	// Compose name, if public and unnamed.
	//if( InName==NAME_None && (InFlags&RF_Public) )
	if( InName==NAME_None )//oldver?? what about compatibility matching of unnamed objects?
		InName = MakeUniqueObjectName( InOuter, InClass );

	// Check for name conflicts.
	if( GCheckConflicts && InName!=NAME_None )
		for( UObject* Hash=GObjHash[GetObjectHash(InName,InOuter?InOuter->GetIndex():0)]; Hash!=NULL; Hash=Hash->HashNext )
			if
			(	Hash->GetFName()==InName
			&&	Hash->Outer==InOuter
			&&	Hash->GetClass()!=InClass )
				debugf( NAME_Log, TEXT("CONFLICT: %s - %s"), Hash->GetFullName(), InClass->GetName() );

	// See if object already exists.
	UObject* Obj                        = StaticFindObject( InClass, InOuter, *InName );//oldver: Should use NULL instead of InClass to prevent conflicts by name rather than name-class.
	UClass*  Cls                        = Cast<UClass>( Obj );
	INT      Index                      = INDEX_NONE;
	UClass*  ClassWithin				= NULL;
	DWORD    ClassFlags                 = 0;
	void     (*ClassConstructor)(void*) = NULL;
	if( !Obj )
	{
		// Create a new object.
        Obj = Ptr ? Ptr : (UObject*)appMalloc( InClass->GetPropertiesSize(), *InName );
		GNumNewObjects++;		// JEP
	}
	else
	{
		// Replace an existing object without affecting the original's address or index.
		check(!Ptr || Ptr==Obj);
		debugfSlow( NAME_DevReplace, TEXT("Replacing %s"), Obj->GetName() );

		// Can only replace if class is identical.
		if( Obj->GetClass() != InClass )
			appErrorf( LocalizeError("NoReplace"), Obj->GetFullName(), InClass->GetName() );

		// Remember flags, index, and native class info.
		InFlags |= (Obj->GetFlags() & RF_Keep);
		Index    = Obj->Index;
		if( Cls )
		{
			ClassWithin		 = Cls->ClassWithin;
			ClassFlags       = Cls->ClassFlags & CLASS_Abstract;
			ClassConstructor = Cls->ClassConstructor;
		}

		// Destroy the object.
		Obj->~UObject();
		check(GObjAvailable.Num() && GObjAvailable.Last()==Index);
		GObjAvailable.Pop();
		GNumReplacedObjects++;		// JEP
	}

	// If class is transient, objects must be transient.
	if( InClass->ClassFlags & CLASS_Transient )
		InFlags |= RF_Transient;

	// Set the base properties.
	Obj->Index			 = INDEX_NONE;
	Obj->HashNext		 = NULL;
	Obj->StateFrame      = NULL;
	Obj->_Linker		 = NULL;
	Obj->_LinkerIndex	 = INDEX_NONE;
	Obj->Outer			 = InOuter;
	Obj->ObjectFlags	 = InFlags;
	Obj->Name			 = InName;
	Obj->Class			 = InClass;
//	Obj->ExtPropData	 = NULL;

	// Init the properties.	
	InitProperties( (BYTE*)Obj, InClass->GetPropertiesSize(), InClass, (BYTE*)InTemplate, InClass->GetPropertiesSize() );

	// Add to global table.
	Obj->AddObject( Index );
	check(Obj->IsValid());

	// If config is per-object, load it.
	if( InClass->ClassFlags & CLASS_PerObjectConfig )
	{
		Obj->LoadConfig();
		Obj->LoadLocalized();
	}

	// Restore class information if replacing native class.
	if( Cls )
	{
		Cls->ClassWithin	   = ClassWithin;
		Cls->ClassFlags       |= ClassFlags;
		Cls->ClassConstructor  = ClassConstructor;
	}

	// Success.
	return Obj;
	unguardf(( TEXT("(%s %s)"), InClass ? InClass->GetName() : TEXT("NULL"), *InName ));
}

//
// Construct an object.
//
UObject* UObject::StaticConstructObject
(
	UClass*			InClass,
	UObject*		InOuter,
	FName			InName,
	DWORD			InFlags,
	UObject*		InTemplate,
	FOutputDevice*	Error
)
{
	check(Error);

	// Allocate the object.
	UObject* Result = StaticAllocateObject( InClass, InOuter, InName, InFlags, InTemplate, Error );
	if( Result )
		(*InClass->ClassConstructor)( Result );
	return Result;

}

/*-----------------------------------------------------------------------------
   Garbage collection.
-----------------------------------------------------------------------------*/

//
// Serialize the global root set to an archive.
//
void UObject::SerializeRootSet( FArchive& Ar, DWORD KeepFlags, DWORD RequiredFlags )
{
	Ar << GObjRoot;
	for( FObjectIterator It; It; ++It )
	{
		if
		(	(It->GetFlags() & KeepFlags)
		&&	(It->GetFlags()&RequiredFlags)==RequiredFlags )
		{
			UObject* Obj = *It;
			Ar << Obj;
		}
	}
}

EXECVAR(int, ShowReferences, false);
//
// Archive for finding unused objects.
//
class FArchiveTagUsed : public FArchive
{
public:
	FArchiveTagUsed()
	: Context( NULL )
	{
		GGarbageRefCount=0;

		// Tag all objects as unreachable.
		for( FObjectIterator It; It; ++It )
			It->SetFlags( RF_Unreachable | RF_TagGarbage );

		// Tag all names as unreachable.
		for( INT i=0; i<FName::GetMaxNames(); i++ )
			if( FName::GetEntry(i) )
				FName::GetEntry(i)->Flags |= RF_Unreachable;

	}
private:
	FArchive& operator<<( UObject*& Obj )
	{
		GGarbageRefCount++;

#if DO_GUARD_SLOW
		if( Obj )
			check(Obj->IsValid());
#endif
		if( Obj && (Obj->GetFlags() & RF_EliminateObject) )
		{
			// Dereference it.
			Obj = NULL;
		}
		else if( Obj && (Obj->GetFlags() & RF_Unreachable) )
		{
			// Only recurse the first time object is claimed.
			guard(TestReach);
			Obj->ClearFlags( RF_Unreachable | RF_DebugSerialize );

			// Recurse.
			if( Obj->GetFlags() & RF_TagGarbage )
			{
				// Recurse down the object graph.
				UObject* OriginalContext = Context;
				Context = Obj;
				Obj->Serialize( *this );
				if( !(Obj->GetFlags() & RF_DebugSerialize) )
					appErrorf( TEXT("%s failed to route Serialize"), Obj->GetFullName() );
				Context = OriginalContext;
			}
			else
			{
				// NJS: A Bit of debugging code:
				if(ShowReferences)
					debugf( NAME_Log, TEXT("%s is referenced by %s"), Obj->GetFullName(), Context ? Context->GetFullName() : NULL );
			}
			unguardf(( TEXT("(%s)"), Obj->GetFullName() ));
		}
		return *this;
	}
	FArchive& operator<<( FName& Name )
	{

		Name.ClearFlags( RF_Unreachable );

		return *this;
	}
	UObject* Context;
};

//
// Purge garbage.
//
void UObject::PurgeGarbage()
{
	INT CountBefore=0, CountPurged=0;
	if( GNoGC )
	{
		debugf( NAME_Log, TEXT("Not purging garbage") );
		return;
	}
	debugf( NAME_Log, TEXT("Purging garbage - DispatchDestroy") );

	// Dispatch all Destroy messages.
	GMalloc->HeapCheck();
	for( INT i=0; i<GObjObjects.Num(); i++ )
	{
		guard(DispatchDestroy);
		CountBefore += (GObjObjects(i)!=NULL);
		if
		(	GObjObjects(i)
		&&	(GObjObjects(i)->GetFlags() & RF_Unreachable)
		&&  (!(GObjObjects(i)->GetFlags() & RF_Native) || GExitPurge) )
		{
			//debugf( NAME_Log, TEXT("DispatchDestroy (class %s, flags %X, name %s)"), GObjObjects(i)->GetClass()->GetName(), GObjObjects(i)->GetFlags(), GObjObjects(i)->GetName());
			debugfSlow( NAME_DevGarbage, TEXT("Garbage collected object %i: %s"), i, GObjObjects(i)->GetFullName() );
			
			//GMalloc->HeapCheck();
			//if (i == 0x11)
			//	appDebugBreak();

			GObjObjects(i)->ConditionalDestroy();
	
			//GMalloc->HeapCheck();
			
			CountPurged++;
		}
		unguardf(( TEXT("(%i: %s)"), i, GObjObjects(i)->GetFullName() ));
	}

	debugf(NAME_Log, TEXT("Purging garbage - DeleteGarbage") );
	// Purge all unreachable objects.
	//warning: Can't use FObjectIterator here because classes may be destroyed before objects.
	FName DeleteName=NAME_None;
	FName DeleteClassName=NAME_None;
	DWORD DeleteFlags=0;
	guard(DeleteGarbage);
	//debugf(NAME_Log, TEXT("DEBUGPROP: Beginning HeapCheck") );
	//GMalloc->HeapCheck();
	//debugf(NAME_Log, TEXT("DEBUGPROP: Done HeapCheck") );
	for( INT i=0; i<GObjObjects.Num(); i++ )
	{
		//debugf(NAME_Log, TEXT("DEBUGPROP: Obj %d"), i );

		if (!GObjObjects(i))
			continue;
		check(GObjObjects(i)->IsValid());

		guard(DeleteObject);
		if
		(	GObjObjects(i)
		&&	(GObjObjects(i)->GetFlags() & RF_Unreachable) 
		&& !(GObjObjects(i)->GetFlags() & RF_Native) )
		{			
			DeleteName = GObjObjects(i)->GetFName();
			DeleteFlags = GObjObjects(i)->GetFlags();
			//if (GObjObjects(i)->GetClass())
			//	DeleteClassName = GObjObjects(i)->GetClass()->GetFName();
/*
			if (i == 13790)
			{
				for (UObject* Bogo=GObjObjects(i); Bogo; Bogo = Bogo->GetOuter())
					debugf(TEXT("BOGO: %s"), Bogo->GetName());
				debugf(TEXT("BOGO: DONE"));
			}
*/
//	GMalloc->HeapCheck();

			//guard(DeleteObject);
			delete GObjObjects(i);
			//unguardf(( TEXT("(Index: %i)"), i ));
		}
		unguardf(( TEXT("(Index: %i)"), i ));
	}
	unguardf(( TEXT("(Name: %s, Class: %s, Flags: %X)"), *DeleteName, *DeleteClassName, DeleteFlags ));

	debugf(NAME_Log, TEXT("Purging garbage - DeleteNames") );
	// Purge all unreachable names.
	//debugf(NAME_Log, TEXT("DEBUGPROP: Beginning HeapCheck") );
	//GMalloc->HeapCheck();
	//debugf(NAME_Log, TEXT("DEBUGPROP: Done HeapCheck") );
	for( i=0; i<FName::GetMaxNames(); i++ )
	{
		FNameEntry* Name = FName::GetEntry(i);
		if
		(	(Name)
		&&	(Name->Flags & RF_Unreachable)
		&& !(Name->Flags & RF_Native     ) )
		{
			debugfSlow( NAME_DevGarbage, TEXT("Garbage collected name %i: %s"), i, Name->Name );
			FName::DeleteEntry(i);
		}
	}

	debugf(TEXT("Garbage: objects: %i->%i; refs: %i"),CountBefore,CountBefore-CountPurged,GGarbageRefCount);
}

//
// Delete all unreferenced objects.
//
void UObject::CollectGarbage( DWORD KeepFlags )
{
	debugf( NAME_Log, TEXT("Collecting garbage") );

	// Tag and purge garbage.
	FArchiveTagUsed TagUsedAr;
	SerializeRootSet( TagUsedAr, KeepFlags, RF_TagGarbage );

	// Purge it.
	PurgeGarbage();

	debugf( NAME_Log, TEXT("Garbage collection complete.") );

}

//
// Returns whether an object is referenced, not counting the
// one reference at Obj. No side effects.
//
UBOOL UObject::IsReferenced( UObject*& Obj, DWORD KeepFlags, UBOOL IgnoreReference )
{

	// Remember it.
	UObject* OriginalObj = Obj;
	if( IgnoreReference )
		Obj = NULL;

	// Tag all garbage.
	FArchiveTagUsed TagUsedAr;
	OriginalObj->ClearFlags( RF_TagGarbage );
	SerializeRootSet( TagUsedAr, KeepFlags, RF_TagGarbage );

	// Stick the reference back.
	Obj = OriginalObj;

	// Return whether this is tagged.
	return (Obj->GetFlags() & RF_Unreachable)==0;
}

//
// Attempt to delete an object. Only succeeds if unreferenced.
//
UBOOL UObject::AttemptDelete( UObject*& Obj, DWORD KeepFlags, UBOOL IgnoreReference )
{
	if( !(Obj->GetFlags() & RF_Native) && !IsReferenced( Obj, KeepFlags, IgnoreReference ) )
	{
		PurgeGarbage();
		return 1;
	}
	else return 0;
}

/*-----------------------------------------------------------------------------
	Importing and exporting.
-----------------------------------------------------------------------------*/

//
// Import an object from a file.
//
void UObject::ParseParms( const TCHAR* Parms )
{
	if( !Parms )
		return;
	for( TFieldIterator<UProperty> It(GetClass()); It; ++It )
	{
		if( It->GetOuter()!=UObject::StaticClass() )
		{
			FString Value;
			if( Parse(Parms,*(FString(It->GetName())+TEXT("=")),Value) )
			{
				It->ImportText( *Value, (BYTE*)this + It->Offset, PPF_Localized );
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	Driver support.
-----------------------------------------------------------------------------*/

//
// Cache a list of globally registered objects into memory.
//
void UObject::CacheDrivers( UBOOL ForceRefresh )
{
	TCHAR Buffer[32767];
	if( ForceRefresh || appStricmp(GObjCachedLanguage,UObject::GetLanguage())!=0 )
	{
		appStrcpy( GObjCachedLanguage, UObject::GetLanguage() );
		GObjPreferences.Empty();
		GObjDrivers.Empty();
		{
		for( INT i=0; i<GSys->Paths.Num(); i++ )
		{
			TCHAR Filename[256];
			appSprintf( Filename, TEXT("%s%s"), appBaseDir(), *GSys->Paths(i) );
			TCHAR* Tmp = appStrstr( Filename, TEXT("*.") );
			if( Tmp )
			{
				appSprintf( Tmp, TEXT("*.int") );
				TArray<FString> Files = GFileManager->FindFiles( Filename, 1, 0 );
				for( INT j=0; j<Files.Num(); j++ )
				{
					appSprintf( Tmp, TEXT("%s%s"), appBaseDir(), *Files(j) );
					TCHAR* End = Tmp + appStrlen( Tmp ) - 4;
					appSprintf( End, TEXT(".%s"), UObject::GetLanguage() );
					UBOOL Success = GConfig->GetSection( TEXT("Public"), Buffer, ARRAY_COUNT(Buffer), Tmp );
					if( !Success )
					{
						appSprintf( End, TEXT(".int") );
						Success = GConfig->GetSection( TEXT("Public"), Buffer, ARRAY_COUNT(Buffer), Tmp );
					}
					if( Success )
					{
						TCHAR* Next;
						for( TCHAR* Key=Buffer; *Key; Key=Next )
						{
							Next = Key + appStrlen(Key) + 1;
							TCHAR* Value = appStrchr(Key,'=');
							if( Value )
							{
								*Value++ = 0;
								if( *Value=='(' )
									*Value++=0;
								if( *Value && Value[appStrlen(Value)-1]==')' )
									Value[appStrlen(Value)-1]=0;
								if( appStricmp(Key,TEXT("Object"))==0 )
								{
									FRegistryObjectInfo& Info = *new(GObjDrivers)FRegistryObjectInfo;
									Parse( Value, TEXT("Name="), Info.Object );
									Parse( Value, TEXT("Class="), Info.Class );
									Parse( Value, TEXT("MetaClass="), Info.MetaClass );
									Parse( Value, TEXT("Description="), Info.Description );
									Parse( Value, TEXT("Autodetect="), Info.Autodetect );
								}
								else if( appStricmp(Key,TEXT("Preferences"))==0 )
								{
									FPreferencesInfo& Info = *new(GObjPreferences)FPreferencesInfo;
									Parse( Value, TEXT("Caption="), Info.Caption );
									Parse( Value, TEXT("Parent="), Info.ParentCaption );
									Parse( Value, TEXT("Class="), Info.Class );
									Parse( Value, TEXT("Category="), Info.Category );
									ParseUBOOL( Value, TEXT("Immediate="), Info.Immediate );
									Info.Category.SetFlags( RF_Native );
								}
							}
						}
					}
				}
			}
		}
		}
		
        {
		for( INT i=0; i<GSys->Paths.Num(); i++ )
		{
			TCHAR Filename[256];
			appSprintf( Filename, TEXT("%s%s"), appBaseDir(), *GSys->Paths(i) );
			TCHAR* Tmp = appStrstr( Filename, TEXT("*.") );
			if( Tmp )
			{
				appSprintf( Tmp, TEXT("*.mds") );
				TArray<FString> Files = GFileManager->FindFiles( Filename, 1, 0 );
				for( INT j=0; j<Files.Num(); j++ )
				{
					appSprintf( Tmp, TEXT("%s%s"), appBaseDir(), *Files(j) );
					
					UBOOL Success = GConfig->GetSection( TEXT("Public"), Buffer, ARRAY_COUNT(Buffer), Tmp );

                    if( Success )
					{
						TCHAR* Next;
						for( TCHAR* Key=Buffer; *Key; Key=Next )
						{
							Next = Key + appStrlen(Key) + 1;
							TCHAR* Value = appStrchr(Key,'=');
							if( Value )
							{
								*Value++ = 0;
								if( *Value=='(' )
									*Value++=0;
								if( *Value && Value[appStrlen(Value)-1]==')' )
									Value[appStrlen(Value)-1]=0;

								if( appStricmp(Key,TEXT("Object"))==0 )
								{
									FModelDataSpecification& Info = *new(GObjModelData)FModelDataSpecification;
                                    FString newParent;
                                    FString ParentString;
                                    const TCHAR *Parents;

									Parse( Value, TEXT("Name="),        Info.Object );
									Parse( Value, TEXT("Class="),       Info.Class );									                                    
                                    Parse( Value, TEXT("Parent="),      ParentString );

                                    Parents = *ParentString;
                                     
                                    while( ParseToken( Parents, newParent, false ) )
                                    {
                                        Info.Parent.AddUniqueItem( newParent );
                                    }

                                    Parse( Value, TEXT("Category="),    Info.Category );
									Parse( Value, TEXT("Description="), Info.Description );
									Parse( Value, TEXT("ExtraData="),   Info.ExtraData );
								}
                                else if( appStricmp(Key,TEXT("Group"))==0 )
								{
									FModelDataGroup& Info = *new(GObjModelDataGroups)FModelDataGroup;
                                    FString newMember;
                                    FString MemberString;
                                    const TCHAR *Members;

									Parse( Value, TEXT("Name="),    Info.Group );
                                    Parse( Value, TEXT("Members="), MemberString );

                                    Members = *MemberString;
                                    
                                    while( ParseToken( Members, newMember, false ) )
                                    {
                                        Info.Members.AddUniqueItem( newMember );                                        
                                    }

									Parse( Value, TEXT("Description="), Info.Description );									
								}
							}
						}
					}
				}
			}
		}
		}

		TArray<FString> CoreLangs = GFileManager->FindFiles( TEXT("Core.*"), 1, 0 );
		{
			for( INT i=0; i<CoreLangs.Num(); i++ )
			{
				FString Ext;
				if( CoreLangs(i).Split(FString(TEXT(".")),NULL,&Ext,1) )
				{
					Ext = FString(TEXT(".")) + Ext;
					if( Ext!=DLLEXT && Ext!=TEXT(".u") && Ext!=TEXT(".ilk") )
					{
						FRegistryObjectInfo& Info = *new(GObjDrivers)FRegistryObjectInfo;
						Info.Object		= appStrchr(*CoreLangs(i),'.')+1;
						Info.Class		= TEXT("Class");
						Info.MetaClass	= ULanguage::StaticClass()->GetPathName();
					}
				}
			}
		}
	}
}

//
// Get a list of globally registered objects complying to 
// the specified class and optional metaclass (base class of UClasses).
//
void UObject::GetRegistryObjects
(
	TArray<FRegistryObjectInfo>&	Results,
	UClass*							Class,
	UClass*							MetaClass,
	UBOOL							ForceRefresh
)
{
	check(Class);
	check(Class!=UClass::StaticClass() || MetaClass);
	CacheDrivers( ForceRefresh );
	const TCHAR* ClassName = Class->GetName();
	const TCHAR* MetaClassName = MetaClass ? MetaClass->GetPathName() : TEXT("");
	for( INT i=0; i<GObjDrivers.Num(); i++ )
		if
		(	appStricmp(*GObjDrivers(i).Class, ClassName)==0
		&&	appStricmp(*GObjDrivers(i).MetaClass, MetaClassName)==0 )
			new(Results)FRegistryObjectInfo(GObjDrivers(i));
}

//
// Get a list of everything that should show up in the hierarchical
// preferences window.
//
void UObject::GetPreferences( TArray<FPreferencesInfo>& Results, const TCHAR* ParentCaption, UBOOL ForceRefresh )
{
	CacheDrivers( ForceRefresh );
	Results.Empty();
	for( INT i=0; i<GObjPreferences.Num(); i++ )
		if( appStricmp(*GObjPreferences(i).ParentCaption,ParentCaption)==0 )
			new(Results)FPreferencesInfo(GObjPreferences(i));
}

// Get a list of matching model data specs for the specified parent and category
void UObject::GetModelDataSpecifications( TArray<FModelDataSpecification>& Results, const TCHAR* Class, const TCHAR* Category, UBOOL ForceRefresh )
{
	CacheDrivers( ForceRefresh );
	Results.Empty();
	for( INT i=0; i<GObjModelData.Num(); i++ )
    {
		if ( appStricmp(*GObjModelData(i).Class,Class)==0 ) // Compare class
        {
            if ( appStrlen( Category ) )
            {
                if ( appStricmp(*GObjModelData(i).Category,Category) ) // Compare Category if specified
                    continue;
            }
    		new(Results)FModelDataSpecification(GObjModelData(i));
        }
    }
}

// Get a list of matching model data specs for the specified parent and category
void UObject::GetModelDataGroups( TArray<FModelDataGroup>& Results, const TCHAR* GroupName, UBOOL ForceRefresh )
{
	CacheDrivers( ForceRefresh );
	Results.Empty();
	for( INT i=0; i<GObjModelDataGroups.Num(); i++ )
    {
		if ( appStricmp(*GObjModelDataGroups(i).Group,GroupName)==0 ) // Compare class
        {
    		new(Results)FModelDataGroup(GObjModelDataGroups(i));
        }
    }
}

/*-----------------------------------------------------------------------------
	UTextBuffer implementation.
-----------------------------------------------------------------------------*/

UTextBuffer::UTextBuffer( const TCHAR* InText )
: Text( InText )
{}
void UTextBuffer::Serialize( const TCHAR* Data, EName Event )
{
	Text += (TCHAR*)Data;
}
void UTextBuffer::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	Ar << Pos << Top << Text;
}
IMPLEMENT_CLASS(UTextBuffer);

/*-----------------------------------------------------------------------------
	UEnum implementation.
-----------------------------------------------------------------------------*/

UEnum::UEnum( UEnum* InSuperEnum )
: UField( InSuperEnum )
{}
void UEnum::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);
	Ar << Names;
}
IMPLEMENT_CLASS(UEnum);

/*-----------------------------------------------------------------------------
	FCompactIndex implementation.
-----------------------------------------------------------------------------*/

//
// FCompactIndex serializer.
//
FArchive& operator<<( FArchive& Ar, FCompactIndex& I )
{
	if( !Ar.IsLoading() && !Ar.IsSaving() )
	{
		Ar << I.Value;
	}
	else
	{
		INT   Original = I.Value;
		DWORD V        = Abs(I.Value);
		BYTE  B0       = ((I.Value>=0) ? 0 : 0x80) + ((V < 0x40) ? V : ((V & 0x3f)+0x40));
		I.Value        = 0;
		Ar << B0;
		if( B0 & 0x40 )
		{
			V >>= 6;
			BYTE B1 = (V < 0x80) ? V : ((V & 0x7f)+0x80);
			Ar << B1;
			if( B1 & 0x80 )
			{
				V >>= 7;
				BYTE B2 = (V < 0x80) ? V : ((V & 0x7f)+0x80);
				Ar << B2;
				if( B2 & 0x80 )
				{
					V >>= 7;
					BYTE B3 = (V < 0x80) ? V : ((V & 0x7f)+0x80);
					Ar << B3;
					if( B3 & 0x80 )
					{
						V >>= 7;
						BYTE B4 = V;
						Ar << B4;
						I.Value = B4;
					}
					I.Value = (I.Value << 7) + (B3 & 0x7f);
				}
				I.Value = (I.Value << 7) + (B2 & 0x7f);
			}
			I.Value = (I.Value << 7) + (B1 & 0x7f);
		}
		I.Value = (I.Value << 6) + (B0 & 0x3f);
		if( B0 & 0x80 )
			I.Value = -I.Value;
		if( Ar.IsSaving() && I.Value!=Original )
			appErrorf( TEXT("Mismatch: %08X %08X"), I.Value, Original );
	}
	return Ar;
}

/*-----------------------------------------------------------------------------
	Implementations.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(ULinker);
IMPLEMENT_CLASS(ULinkerLoad);
IMPLEMENT_CLASS(ULinkerSave);
IMPLEMENT_CLASS(USubsystem);

/*-----------------------------------------------------------------------------
	UCommandlet.
-----------------------------------------------------------------------------*/

UCommandlet::UCommandlet()
: HelpCmd(E_NoInit), HelpOneLiner(E_NoInit), HelpUsage(E_NoInit), HelpWebLink(E_NoInit)
{}
INT UCommandlet::Main( const TCHAR* Parms )
{
	return eventMain( Parms );
}
void UCommandlet::execMain( FFrame& Stack, RESULT_DECL )
{

	P_GET_STR(Parms);
	P_FINISH;

	*(INT*)Result = Main( *Parms );

}
IMPLEMENT_FUNCTION( UCommandlet, INDEX_NONE, execMain );
IMPLEMENT_CLASS(UCommandlet);

/*-----------------------------------------------------------------------------
	UPackage.
-----------------------------------------------------------------------------*/

UPackage::UPackage()
{

	// Bind to a matching DLL, if any.
	BindPackage( this );

}
void UPackage::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
}
void UPackage::Destroy()
{
	if( DllHandle )
	{
		debugf( NAME_Log, TEXT("Unbound to %s%s"), GetName(), DLLEXT );
		//Danger: If package is garbage collected before its autoregistered classes,
		//the autoregistered classes will be unmapped from memory.
		//appFreeDllHandle( DllHandle );
	}
	Super::Destroy();
}
void* UPackage::GetDllExport( const TCHAR* ExportName, UBOOL Checked )
{
	void* Result;
	if( !DllHandle )
	{
		if( Checked && !ParseParam(appCmdLine(),TEXT("nobind")) )
			appErrorf( LocalizeError("NotDll"), GetName(), ExportName );
		Result = NULL;
	}
	else
	{
		Result = appGetDllExport( DllHandle, ExportName );
		if( !Result && Checked && !ParseParam(appCmdLine(),TEXT("nobind")) )
			appErrorf( LocalizeError("NotInDll"), ExportName, GetName() );
		if( Result )
			debugfSlow( NAME_DevBind, TEXT("Found %s in %s%s"), ExportName, GetName(), DLLEXT );
	}
	return Result;
}
IMPLEMENT_CLASS(UPackage);

/*-----------------------------------------------------------------------------
	UTextBufferFactory.
-----------------------------------------------------------------------------*/

void UTextBufferFactory::StaticConstructor()
{

	SupportedClass = UTextBuffer::StaticClass();
	bCreateNew     = 0;
	bText          = 1;
	new(Formats)FString( TEXT("txt;Text files") );

}
UTextBufferFactory::UTextBufferFactory()
{}
UObject* UTextBufferFactory::FactoryCreateText
(
	UClass*				Class,
	UObject*			InOuter,
	FName				InName,
	DWORD				InFlags,
	UObject*			Context,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{

	// Import.
	UTextBuffer* Result = new(InOuter,InName,InFlags)UTextBuffer;
	Result->Text = Buffer;
	return Result;

}
IMPLEMENT_CLASS(UTextBufferFactory);

/*----------------------------------------------------------------------------
	ULanguage.
----------------------------------------------------------------------------*/

const TCHAR* UObject::GetLanguage()
{
	return GLanguage;
}
void UObject::SetLanguage( const TCHAR* LangExt )
{
	if( appStricmp(LangExt,GLanguage)!=0 )
	{
		appStrcpy( GLanguage, LangExt );
		appStrcpy( GNone,  LocalizeGeneral( TEXT("None"),  TEXT("Core")) );
		appStrcpy( GTrue,  LocalizeGeneral( TEXT("True"),  TEXT("Core")) );
		appStrcpy( GFalse, LocalizeGeneral( TEXT("False"), TEXT("Core")) );
		appStrcpy( GYes,   LocalizeGeneral( TEXT("Yes"),   TEXT("Core")) );
		appStrcpy( GNo,    LocalizeGeneral( TEXT("No"),    TEXT("Core")) );
		for( FObjectIterator It; It; ++It )
			It->LanguageChange();
	}
}
IMPLEMENT_CLASS(ULanguage);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/

