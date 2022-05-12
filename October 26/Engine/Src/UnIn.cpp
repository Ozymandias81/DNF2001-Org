/*=============================================================================
	UnIn.cpp: Unreal input system.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Internal.
-----------------------------------------------------------------------------*/

class FInputVarCache
{
public:
	INT Count;
	UProperty* Properties[ZEROARRAY];
	static FInputVarCache* Get( UClass* Class, FCacheItem*& Item )
	{
		QWORD CacheId = MakeCacheID( CID_InputMap, Class );
		FInputVarCache* Result = (FInputVarCache*)GCache.Get(CacheId,Item);
		if( !Result )
		{
			INT Count=0, Temp=0;
			for( TFieldIterator<UProperty> It(Class); It; ++It )
				if( It->PropertyFlags & CPF_Input )
					Count++;
			Result = (FInputVarCache*)GCache.Create(CacheId,Item,sizeof(FInputVarCache)+Count*sizeof(UProperty*));
			Result->Count = Count;
			for( It=TFieldIterator<UProperty>(Class); It; ++It )
				if( It->PropertyFlags & CPF_Input )
					Result->Properties[Temp++] = *It;
		}
		return Result;
	}
};

/*-----------------------------------------------------------------------------
	Implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UInput);

/*-----------------------------------------------------------------------------
	UInput creation and destruction.
-----------------------------------------------------------------------------*/

//
// Temporary.
// !!should be moved to an UnrealScript definition.
//
void UInput::StaticInitInput()
{
	FArchive ArDummy;

	// Create input alias struct.
	UStruct* AliasStruct = new(StaticClass(),TEXT("Alias"))UStruct( NULL );	
    AliasStruct->SetPropertiesSize( sizeof(FName) + sizeof(FString));
	new(AliasStruct,TEXT("Alias"),  RF_Public)UNameProperty( EC_CppProperty, 0,             TEXT(""), CPF_Config );
	new(AliasStruct,TEXT("Command"),RF_Public)UStrProperty ( EC_CppProperty, sizeof(FName), TEXT(""), CPF_Config );
	AliasStruct->Link( ArDummy, 0 );

	// Add alias list to class.
	UStructProperty* Q = new(StaticClass(),TEXT("Aliases"),RF_Public)UStructProperty( CPP_PROPERTY(Aliases), TEXT("Aliases"), CPF_Config, AliasStruct );
	Q->ArrayDim = ALIAS_MAX;

	// Add key list.
	UEnum* InputKeys = FindObjectChecked<UEnum>( AActor::StaticClass(), TEXT("EInputKey") );
	for( INT i=0; i<IK_MAX; i++ )
	{
		if( InputKeys->Names(i)!=NAME_None )
		{
			const TCHAR* Str = *InputKeys->Names(i);
			new(StaticClass(),Str+3,RF_Public)UStrProperty( EC_CppProperty, (INT)&((UInput*)NULL)->Bindings[i], TEXT("RawKeys"), CPF_Config );
		}
	}
	StaticClass()->Link( ArDummy, 0 );

	// Load defaults.
	StaticClass()->GetDefaultObject()->LoadConfig( 1 );
}

//
// Constructor.
//
UInput::UInput()
{
	InputKeys = FindObjectChecked<UEnum>( AActor::StaticClass(), TEXT("EInputKey") );
}

//
// Class initializer.
//
void UInput::StaticConstructor()
{
}

//
// Serialize
//
void UInput::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	Ar << InputKeys;
}

//
// Find a button.
//
BYTE* UInput::FindButtonName( AActor* Actor, const TCHAR* ButtonName ) const
{
	check(Viewport);
	check(Actor);
	FName Button( ButtonName, FNAME_Find );
	if( Button != NAME_None )
	{
		FCacheItem* Item;
		FInputVarCache* Cache = FInputVarCache::Get( Actor->GetClass(), Item );
		for( INT i=0; i<Cache->Count; i++ )
			if
			(	Cache->Properties[i]->GetFName()==Button
			&&	Cast<UByteProperty>(Cache->Properties[i]) )
				break;
		Item->Unlock();
		if( i<Cache->Count )
			return (BYTE*)Actor + Cache->Properties[i]->Offset;
	}
	return NULL;
}

//
// Find an axis.
//
FLOAT* UInput::FindAxisName( AActor* Actor, const TCHAR* ButtonName ) const
{
	check(Viewport);
	check(Actor);
	FName Button( ButtonName, FNAME_Find );
	if( Button != NAME_None )
	{
		FCacheItem* Item;
		FInputVarCache* Cache = FInputVarCache::Get( Actor->GetClass(), Item );
		for( INT i=0; i<Cache->Count; i++ )
			if
			(	Cache->Properties[i]->GetFName()==Button
			&&	Cast<UFloatProperty>(Cache->Properties[i]) )
				break;
		Item->Unlock();
		if( i<Cache->Count )
			return (FLOAT*)((BYTE*)Actor + Cache->Properties[i]->Offset);
	}
	return NULL;
}

//
// Execute input commands.
//
void UInput::ExecInputCommands( const TCHAR* Cmd, FOutputDevice& Ar )
{
	TCHAR Line[256];
	while( ParseLine( &Cmd, Line, ARRAY_COUNT(Line)) )
	{
		const TCHAR* Str = Line;
		if( Action==IST_Press || (Action==IST_Release && ParseCommand(&Str,TEXT("OnRelease"))) )
			Viewport->Exec( Str, Ar );
		else
			Exec( Str, Ar );
	}
}

//
// Init.
//
void UInput::Init( UViewport* InViewport )
{
	// Set objects.
	Viewport = InViewport;

	// Reset.
	ResetInput();

	debugf( NAME_Init, TEXT("Input system initialized for %s"), Viewport->GetName() );
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

//
// Execute a command.
//
UBOOL UInput::Exec( const TCHAR* Str, FOutputDevice& Ar )
{
	TCHAR Temp[256];
	static UBOOL InAlias=0;
	if( ParseCommand( &Str, TEXT("BUTTON") ) )
	{
		// Normal button.
		BYTE* Button;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	(Button=FindButtonName(Viewport->Actor,Temp))!=NULL )
		{
			if( GetInputAction() == IST_Press )
				*Button = 1;
			else if( GetInputAction()==IST_Release && *Button )
				*Button = 0;
		}
		else Ar.Log( TEXT("Bad Button command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("PULSE") ) )
	{
		// Normal button.
		BYTE* Button;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	(Button=FindButtonName(Viewport->Actor,Temp))!=NULL )
		{
			if( GetInputAction() == IST_Press )
				*Button = 1;
		}
		else Ar.Log( TEXT("Bad Button command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("TOGGLE") ) )
	{
		// Toggle button.
		BYTE* Button;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	((Button=FindButtonName(Viewport->Actor,Temp))!=NULL) )
		{
			if( GetInputAction() == IST_Press )
				*Button ^= 0x80;
		}
		else Ar.Log( TEXT("Bad Toggle command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("AXIS") ) )
	{
		// Axis movement.
		FLOAT* Axis;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	(Axis=FindAxisName(Viewport->Actor,Temp))!=NULL )
		{
			FLOAT Speed=1.0;
			Parse( Str, TEXT("SPEED="), Speed );
			if( GetInputAction() == IST_Axis )
			{
				*Axis += GetInputDelta() * Speed;
			}
			else if( GetInputAction() == IST_Hold )
			{
				*Axis = GetInputDelta() * Speed;
			}
		}
		else Ar.Logf( TEXT("Bad Axis command") );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("KEYNAME") ) )
	{
		INT keyNo = appAtoi(Str);
		Ar.Log( GetKeyName(EInputKey(keyNo)) );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("KEYBINDING") ) )
	{
		EInputKey iKey;
		if( FindKeyName(Str,iKey) )
			Ar.Log( *Bindings[iKey] );

		return 1;
	}
	else if( !InAlias && ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) )
	{
		FName Name(Temp,FNAME_Find);
		if( Name!=NAME_None )
		{
			for( INT i=0; i<ARRAY_COUNT(Aliases); i++ )
			{
				if( Aliases[i].Alias==Name )
				{
					InAlias=1;
					ExecInputCommands( *Aliases[i].Command, Ar );
					InAlias=0;
					return 1;
				}
			}
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------------
	Key and axis movement processing.
-----------------------------------------------------------------------------*/

//
// Preprocess input to maintain key tables.
//
UBOOL UInput::PreProcess( EInputKey iKey, EInputAction State, FLOAT Delta )
{
	switch( State )
	{
		case IST_Press:
			if( KeyDownTable[iKey] )
				return 0;
			KeyDownTable[iKey] = 1;
			return 1;
		case IST_Release:
			if( !KeyDownTable[iKey] )
				return 0;
			KeyDownTable[iKey] = 0;
			return 1;
		default:
			return 1;
	}
}

//
// Process input. Returns 1 if handled, 0 if not.
//
UBOOL UInput::Process( FOutputDevice& Ar, EInputKey iKey, EInputAction State, FLOAT Delta )
{
	check(iKey>=0&&iKey<IK_MAX);

	// Make sure there is a binding.
	if( Bindings[iKey].Len() )
	{
		// Process each line of the binding string.
		SetInputAction( State, Delta );
		ExecInputCommands( *Bindings[iKey], Ar );
		SetInputAction( IST_None );
		return 1;
	}
	else return 0;
}

/*-----------------------------------------------------------------------------
	Input reading.
-----------------------------------------------------------------------------*/

//
// Read input for the viewport.
//
void UInput::ReadInput( FLOAT DeltaSeconds, FOutputDevice& Ar )
{
	if (GIsRunning)
	{	
		FCacheItem*     Item  = NULL;
		FInputVarCache* Cache = FInputVarCache::Get( Viewport->Actor->GetClass(), Item );

		// Update everything with IST_Hold.
		if( DeltaSeconds!=-1.0 )
			for( INT i=0; i<IK_MAX; i++ )
				if( KeyDownTable[i] )
					Process( *GLog, (EInputKey)i, IST_Hold, DeltaSeconds );

		// Scale the axes.
		FLOAT Scale = DeltaSeconds!=-1.0 ? 20.0/DeltaSeconds : 0.0;
		for( INT i=0; i<Cache->Count; i++ )
			if( Cast<UFloatProperty>(Cache->Properties[i]) )
                *(FLOAT*)((BYTE*)Viewport->Actor + Cache->Properties[i]->Offset) *= Scale;				

		Item->Unlock();
	}
}


/*-----------------------------------------------------------------------------
	Input resetting.
-----------------------------------------------------------------------------*/

//
// Reset the input system's state.
//
void UInput::ResetInput()
{
	check(Viewport);

#if 1		
	// (JEP) Carry key status over 
	// (was making it quickload twice when hitting F9 from the main menu)
	//	I traced it down to the fact that when it switched levels, GetKeyState was returning that the key was
	//	still pressed, but KeyDownTable had been reset, so it issued a another KeyDown message, which tried to load
	//	the level again...
	for( INT i=0; i<IK_MAX; i++ )
	{
		if(GetKeyState(i) & 0x8000)
			KeyDownTable[i] = 1;
		else
			KeyDownTable[i] = 0;
	}
#else
	// Reset all keys.
	for( INT i=0; i<IK_MAX; i++ )
		KeyDownTable[i] = 0;
#endif

	// Reset all input bytes.
	for( TFieldIterator<UByteProperty> ItB(Viewport->Actor->GetClass()); ItB; ++ItB )
		if( ItB->PropertyFlags & CPF_Input )
	        *(BYTE *)((BYTE*)Viewport->Actor + ItB->Offset) = 0;			

	// Reset all input floats.
	for( TFieldIterator<UFloatProperty> ItF(Viewport->Actor->GetClass()); ItF; ++ItF )
		if( ItF->PropertyFlags & CPF_Input )
			*(FLOAT *)((BYTE*)Viewport->Actor + ItF->Offset) = 0;

	// Set the state.
	SetInputAction( IST_None );

	// Reset viewport input.
	Viewport->UpdateInput( 1 );
}

/*-----------------------------------------------------------------------------
	Utility functions.
-----------------------------------------------------------------------------*/

//
// Return the name of a key.
//
const TCHAR* UInput::GetKeyName( EInputKey Key ) const
{
	if( Key>=0 && Key<IK_MAX )
		if( appStrlen(*InputKeys->Names(Key)) > 3 )
			return *InputKeys->Names(Key)+3;
	return TEXT("");
}

//
// Find the index of a named key.
//
UBOOL UInput::FindKeyName( const TCHAR* KeyName, EInputKey& iKey ) const
{
	TCHAR Temp[256];
	appSprintf( Temp, TEXT("IK_%s"), KeyName );
	FName N( Temp, FNAME_Find );
	if( N != NAME_None )
		return InputKeys->Names.FindItem( N, *(INT*)&iKey );
	return 0;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
