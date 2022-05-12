/*=============================================================================
	UnGame.cpp: Unreal game engine.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

#include "..\..\Cannibal\CannibalUnr.h"

/*-----------------------------------------------------------------------------
	Object class implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UGameEngine);

/*-----------------------------------------------------------------------------
	cleanup!!
-----------------------------------------------------------------------------*/

// JP_REMOVE!! (put in dnSaveLoad.h)
extern UBOOL		GAutoSaveGame;

extern UBOOL		dnSaveLoad_GSaveGame;
extern ESaveType	dnSaveLoad_GSaveType;
extern INT			dnSaveLoad_GSaveListIndex;
extern FString		dnSaveLoad_GSaveDescription;
extern UTexture		*dnSaveLoad_GSaveScreenshot;

FString			dnSaveLoad_BuildSaveGameFileNameFromListIndex(ESaveType SaveType, INT Index);
void			dnSaveLoad_IncreaseLoads(UGameEngine *Engine, ESaveType SaveType, INT Index);
UBOOL			dnSaveLoad_SaveGame(UGameEngine *Engine, ULevel *Level, ESaveType SaveType, INT Num, const TCHAR *Desc, UTexture *Screenshot);

// JP_REMOVE!! (put in dnScreenshot.h)
extern	UTexture	*GTempTexture;
extern UBOOL		GWaitingForScreenshot;
extern UBOOL		GScreenshotReady;
void				dnScreenshot_FillScreenshotTexture(UViewport *Viewport, UBOOL bFrontBuffer);

void UGameEngine::PaintProgress()
{
	// JEP: Don't tint screen while showing progress
	//FVector LoadFog(.0,.0,.0);
	//FVector LoadScale(.1,.1,.1);
	//FVector LoadScale(.7,.7,.7);

	UViewport* Viewport = Client->Viewports(0);
	//Exchange(Viewport->Actor->FlashFog,LoadFog);
	//Exchange(Viewport->Actor->FlashScale,LoadScale);

	Draw( Viewport );

	//Exchange(Viewport->Actor->FlashFog,LoadFog);
	//Exchange(Viewport->Actor->FlashScale,LoadScale);
}

INT UGameEngine::ChallengeResponse( INT Challenge )
{
	return (Challenge*237) ^ (0x93fe92Ce) ^ (Challenge>>16) ^ (Challenge<<16);
}

void UGameEngine::UpdateConnectingMessage()
{
	if( GPendingLevel && Client && Client->Viewports.Num() )
	{
		APlayerPawn* Actor = Client->Viewports(0)->Actor;
		if( Actor->ProgressTimeOut<Actor->Level->TimeSeconds )
		{
			TCHAR Msg1[256], Msg2[256];
			if( GPendingLevel->DemoRecDriver )
			{
				appSprintf( Msg1, TEXT("") );
				appSprintf( Msg2, *GPendingLevel->URL.Map );
			}
			else
			{
				appSprintf( Msg1, LocalizeProgress("ConnectingText") );
				appSprintf( Msg2, LocalizeProgress("ConnectingURL"), *GPendingLevel->URL.Host, *GPendingLevel->URL.Map );
			}
			SetProgress( Msg1, Msg2, 60.0 );
		}
	}
}
void UGameEngine::BuildServerMasterMap( UNetDriver* NetDriver, ULevel* InLevel )
{
	check(NetDriver);
	check(InLevel);
	BeginLoad();
	{
		// Init LinkerMap.
		check(InLevel->GetLinker());
		NetDriver->MasterMap->AddLinker( InLevel->GetLinker() );

		// Load server-required packages.
		for( INT i=0; i<ServerPackages.Num(); i++ )
		{
			debugf( TEXT("Server Package: %s"), *ServerPackages(i) );
			ULinkerLoad* Linker = GetPackageLinker( NULL, *ServerPackages(i), LOAD_NoFail, NULL, NULL );
			if( NetDriver->MasterMap->AddLinker( Linker )==INDEX_NONE )
				debugf( TEXT("   (server-side only)") );
		}

		// Add GameInfo's package to map.
		check(InLevel->GetLevelInfo());
		check(InLevel->GetLevelInfo()->Game);
		check(InLevel->GetLevelInfo()->Game->GetClass()->GetLinker());
		NetDriver->MasterMap->AddLinker( InLevel->GetLevelInfo()->Game->GetClass()->GetLinker() );

		// Precompute linker info.
		NetDriver->MasterMap->Compute();
	}
	EndLoad();
}

/*-----------------------------------------------------------------------------
	Game init and exit.
-----------------------------------------------------------------------------*/

//
// Construct the game engine.
//
UGameEngine::UGameEngine()
: LastURL(TEXT(""))
, ServerActors( E_NoInit )
, ServerPackages( E_NoInit )
{}

//
// Class creator.
//
void UGameEngine::StaticConstructor()
{
	UArrayProperty* A = new(GetClass(),TEXT("ServerActors"),RF_Public)UArrayProperty( CPP_PROPERTY(ServerActors), TEXT("Settings"), CPF_Config );
	A->Inner = new(A,TEXT("StrProperty0"),RF_Public)UStrProperty;

	UArrayProperty* B = new(GetClass(),TEXT("ServerPackages"),RF_Public)UArrayProperty( CPP_PROPERTY(ServerPackages), TEXT("Settings"), CPF_Config );
	B->Inner = new(B,TEXT("StrProperty0"),RF_Public)UStrProperty;
}

//
// Initialize the game engine.
//
void UGameEngine::Init()
{
	check(sizeof(*this)==GetClass()->GetPropertiesSize());

	// Call base.
	UEngine::Init();

	// Init variables.
	GLevel = NULL;

	// Delete temporary files in cache.
	appCleanFileCache();

	// If not a dedicated server.
	if( GIsClient )
	{	
		// Init client.
		UClass* ClientClass = StaticLoadClass( UClient::StaticClass(), NULL, TEXT("ini:Engine.Engine.ViewportManager"), NULL, LOAD_NoFail, NULL );
		Client = ConstructObject<UClient>( ClientClass );
		Client->Init( this );

		// Init rendering.
		UClass* RenderClass = URender::StaticClass();//StaticLoadClass( URenderBase::StaticClass(), NULL, TEXT("ini:Engine.Engine.Render"), NULL, LOAD_NoFail, NULL );
		Render = ConstructObject<URender>( RenderClass );
		Render->Init( this );
	}

	// Load the entry level.
	FString Error;
	if( Client )
	{
		if( !LoadMap( FURL(TEXT("Entry")), NULL, NULL, Error ) )
			appErrorf( LocalizeError("FailedBrowse"), TEXT("Entry"), *Error );
		Exchange( GLevel, GEntry );
	}

	// Create default URL.
	FURL DefaultURL;
	DefaultURL.LoadURLConfig( TEXT("DefaultPlayer"), TEXT("User") );

	// Enter initial world.
	TCHAR Parm[4096]=TEXT("");
	const TCHAR* Tmp = appCmdLine();
	if
	(	!ParseToken( Tmp, Parm, ARRAY_COUNT(Parm), 0 )
	||	(appStricmp(Parm,TEXT("SERVER"))==0 && !ParseToken( Tmp, Parm, ARRAY_COUNT(Parm), 0 ))
	||	Parm[0]=='-' )
		appStrcpy( Parm, *FURL::DefaultLocalMap );
	FURL URL( &DefaultURL, Parm, TRAVEL_Partial );
	if( !URL.Valid )
		appErrorf( LocalizeError("InvalidUrl"), Parm );
	UBOOL Success = Browse( URL, NULL, Error );

	// If waiting for a network connection, go into the starting level.
	if( !Success && Error==TEXT("") && appStricmp( Parm, *FURL::DefaultLocalMap )!=0 )
		Success = Browse( FURL(&DefaultURL,*FURL::DefaultLocalMap,TRAVEL_Partial), NULL, Error );

	// Handle failure.
	if( !Success )
		appErrorf( LocalizeError("FailedBrowse"), Parm, *Error );

	// Open initial Viewport.
	if( Client )
	{
		// Init input.!!Temporary
		UInput::StaticInitInput();

		// Create viewport.
		UViewport* Viewport = Client->NewViewport( NAME_None );

		// Create console.
		UClass* ConsoleClass = StaticLoadClass( UConsole::StaticClass(), NULL, TEXT("ini:Engine.Engine.Console"), NULL, LOAD_NoFail, NULL );
		Viewport->Console = ConstructObject<UConsole>( ConsoleClass );
		Viewport->Console->_Init( Viewport );

		// Spawn play actor.
		FString Error;
		if( !GLevel->SpawnPlayActor( Viewport, ROLE_SimulatedProxy, URL, Error ) )
			appErrorf( TEXT("%s"), *Error );
		Viewport->Input->Init( Viewport );
		Viewport->OpenWindow( 0, 0, (INT) INDEX_NONE, (INT) INDEX_NONE, (INT) INDEX_NONE, (INT) INDEX_NONE );
		GLevel->DetailChange( Viewport->RenDev->HighDetailActors );
		InitAudio();
		if( Audio )
			Audio->SetViewport( Viewport );
	}
	debugf( NAME_Init, TEXT("Game engine initialized") );
}

//
// Pre exit.
//
void UGameEngine::Exit()
{
	Super::Exit();

	// Exit net.
	verify(GLevel);
	if( GLevel->NetDriver )
	{
		delete GLevel->NetDriver;
		GLevel->NetDriver = NULL;
	}
}

//
// Game exit.
//
void UGameEngine::Destroy()
{
	// Game exit.
	if( GPendingLevel )
		CancelPending();
	GLevel = NULL;
	debugf( NAME_Exit, TEXT("Game engine shut down") );

	Super::Destroy();
}

//
// Progress text.
//
void UGameEngine::SetProgress( const TCHAR* Str1, const TCHAR* Str2, FLOAT Seconds )
{
	if( Client && Client->Viewports.Num() )
	{
		APlayerPawn* Actor = Client->Viewports(0)->Actor;
		Actor->ProgressMessage[0] = Str1;
		Actor->ProgressColor[0].R = 255;
		Actor->ProgressColor[0].G = 255;
		Actor->ProgressColor[0].B = 255;

		Actor->ProgressMessage[1] = Str2;
		Actor->ProgressColor[1].R = 255;
		Actor->ProgressColor[1].G = 255;
		Actor->ProgressColor[1].B = 255;

		Actor->ProgressTimeOut    = Actor->Level->TimeSeconds + Seconds;
	}
}

/*-----------------------------------------------------------------------------
	Command line executor.
-----------------------------------------------------------------------------*/

//
// This always going to be the last exec handler in the chain. It
// handles passing the command to all other global handlers.
//
UBOOL UGameEngine::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	const TCHAR* Str=Cmd;
	
	if( ParseCommand( &Str, TEXT("DIR")))
	{
		/*
		TCHAR DirectorySpec[256];

		appStrcpy(DirectorySpec,Str);
		
		Ar.Logf( TEXT("Directory of %s:"),DirectorySpec);
		TArray<FString> Filenames;
		Filenames=appFindFiles(DirectorySpec);

		for(int i=0;i<Filenames.Num();i++)
		{
			// Print info stored in map files: 
			if(appStrstr(*(TCHAR **)&Filenames(i),TEXT(".dnf"))||
			   appStrstr(*(TCHAR **)&Filenames(i),TEXT(".DNF")))
			{
				TCHAR FullPath[1024];

				appStrcpy(FullPath,TEXT("..\\MAPS\\"));
				appStrcat(FullPath,*(TCHAR **)&Filenames(i));

				TitleAuthor ta;
				if(GetTitleAuthor( FullPath, &ta ))
				{
					if(*ta.Author)
						Ar.Logf(TEXT("%s - %s (%s)"),*(TCHAR **)&Filenames(i),ta.Title,ta.Author);
					else
						Ar.Logf(TEXT("%s - %s"),*(TCHAR **)&Filenames(i),ta.Title);

					continue;
				}

			} 
			Ar.Logf(*(char **)&Filenames(i));
		}
		*/
		return 1;
	} else
	if( ParseCommand( &Str, TEXT("OPEN") ) 
	  ||ParseCommand( &Str, TEXT("MAP") ) )
	{
		FString Error, SURL = FString::Printf(TEXT("%s?noauto"), Str);

		if( Client && Client->Viewports.Num() )
			SetClientTravel( Client->Viewports(0), *SURL, 0, TRAVEL_Partial );
		else
		if( !Browse( FURL(&LastURL,Str,TRAVEL_Partial), NULL, Error ) && Error!=TEXT("") )
			Ar.Logf( TEXT("Open failed: %s"), *Error );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("START") ) )
	{
		FString Error;
		if( Client && Client->Viewports.Num() )
			SetClientTravel( Client->Viewports(0), Str, 0, TRAVEL_Absolute );
		else
		if( !Browse( FURL(&LastURL,Str,TRAVEL_Absolute), NULL, Error ) && Error!=TEXT("") )
			Ar.Logf( TEXT("Start failed: %s"), *Error );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("SERVERTRAVEL") ) && (GIsServer && !GIsClient) )
	{
		GLevel->GetLevelInfo()->eventServerTravel(Str,0);
		return 1;
	}
	else if( (GIsServer && !GIsClient) && ParseCommand( &Str, TEXT("SAY") ) )
	{
		GLevel->GetLevelInfo()->eventBroadcastMessage(Str,1,NAME_None);
		return 1;
	}
	else if( ParseCommand(&Str, TEXT("DISCONNECT")) )
	{
		FString Error;
		if( Client && Client->Viewports.Num() )
			SetClientTravel( Client->Viewports(0), TEXT("?failed"), 0, TRAVEL_Absolute );
		else
		if( !Browse( FURL(&LastURL,TEXT("?failed"),TRAVEL_Absolute), NULL, Error ) && Error!=TEXT("") )
			Ar.Logf( TEXT("Disconnect failed: %s"), *Error );
		return 1;
	}
	else if( ParseCommand(&Str, TEXT("RECONNECT")) )
	{
		FString Error;
		if( Client && Client->Viewports.Num() )
			SetClientTravel( Client->Viewports(0), *LastURL.String(), 0, TRAVEL_Absolute );
		else
		if( !Browse( FURL(LastURL), NULL, Error ) && Error!=TEXT("") )
			Ar.Logf( TEXT("Reconnect failed: %s"), *Error );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("GETCURRENTTICKRATE") ) )
	{
		Ar.Logf( TEXT("%f"), CurrentTickRate );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("GETMAXTICKRATE") ) )
	{
		Ar.Logf( TEXT("%f"), GetMaxTickRate() );
		return 1;
	}
	else if( ParseCommand( &Str, TEXT("GSPYLITE") ) )
	{
		FString Error;
		appLaunchURL( TEXT("GSpyLite.exe"), TEXT(""), &Error );
		return 1;
	}
	else if( ParseCommand(&Str,TEXT("SAVEGAME")) )
	{
		if( appIsDigit(Str[0]) )
			SaveGame( appAtoi(Str) );
		return 1;
	}
	else if( ParseCommand( &Cmd, TEXT("CANCEL") ) )
	{
		if( GPendingLevel )
			SetProgress( LocalizeProgress("CancelledConnect"), TEXT(""), 2.0 );
		else
			SetProgress( TEXT(""), TEXT(""), 0.0 );
		CancelPending();
		return 1;
	}
	else if( GLevel && GLevel->Exec( Cmd, Ar ) )
	{
		return 1;
	}
	else if( GLevel && GLevel->GetLevelInfo()->Game && GLevel->GetLevelInfo()->Game->ScriptConsoleExec(Cmd,Ar,NULL) )
	{
		return 1;
	}
	else if( UEngine::Exec( Cmd, Ar ) )
	{
		return 1;
	}
	else return 0;
}

/*-----------------------------------------------------------------------------
	Serialization.
-----------------------------------------------------------------------------*/

//
// Serializer.
//
void UGameEngine::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << GLevel << GEntry << GPendingLevel;
}

/*-----------------------------------------------------------------------------
	Game entering.
-----------------------------------------------------------------------------*/

//
// Cancel pending level.
//
void UGameEngine::CancelPending()
{
	if( GPendingLevel )
	{
		delete GPendingLevel;
		GPendingLevel = NULL;
	}
}

//
// Match Viewports to actors.
//
static void MatchViewportsToActors( UClient* Client, ULevel* Level, const FURL& URL )
{
	for( INT i=0; i<Client->Viewports.Num(); i++ )
	{
		FString Error;
		UViewport* Viewport = Client->Viewports(i);
		debugf( NAME_Log, TEXT("Spawning new actor for Viewport %s"), Viewport->GetName() );
		if( !Level->SpawnPlayActor( Viewport, ROLE_SimulatedProxy, URL, Error ) )
			appErrorf( TEXT("%s"), *Error );
	}
}

//
// Browse to a specified URL, relative to the current one.
//
UBOOL UGameEngine::Browse( FURL URL, const TMap<FString,FString>* TravelInfo, FString& Error )
{
	Error = TEXT("");
	const TCHAR* Option;

	// Convert .unreal link files.
	const TCHAR* LinkStr = TEXT(".unreal");//!!
	if( appStrstr(*URL.Map,LinkStr)-*URL.Map==appStrlen(*URL.Map)-appStrlen(LinkStr) )
	{
		debugf( TEXT("Link: %s"), *URL.Map );
		FString NewUrlString;
		if( GConfig->GetString( TEXT("Link")/*!!*/, TEXT("Server"), NewUrlString, *URL.Map ) )
		{
			// Go to link.
			URL = FURL( NULL, *NewUrlString, TRAVEL_Absolute );//!!
		}
		else
		{
			// Invalid link.
			Error = FString::Printf( LocalizeError("InvalidLink"), *URL.Map );
			return 0;
		}
	}

	// Crack the URL.
	debugf( TEXT("Browse: %s"), *URL.String() );

	// Handle it.
	if( !URL.Valid )
	{
		// Unknown URL.
		Error = FString::Printf( LocalizeError("InvalidUrl"), *URL.String() );
		return 0;
	}
	else if( URL.HasOption(TEXT("failed")) || URL.HasOption(TEXT("entry")) )
	{
		// Handle failure URL.
		debugf( NAME_Log, LocalizeError("AbortToEntry") );
		if( GLevel && GLevel!=GEntry )
		{
			if( GLevel->BrushTracker )
			{
				delete GLevel->BrushTracker;
				GLevel->BrushTracker = NULL;
			}
			ResetLoaders( GLevel->GetOuter(), 1, 0 );
		}
		NotifyLevelChange();
		GLevel = GEntry;
		GLevel->GetLevelInfo()->LevelAction = LEVACT_None;
		check(Client && Client->Viewports.Num());
		MatchViewportsToActors( Client, GLevel, URL );
		if( Audio )
			Audio->SetViewport( Audio->GetViewport() );
		//CollectGarbage( RF_Native ); // Causes texture corruption unless you flush.
		if( URL.HasOption(TEXT("failed")) )
		{
			if( !GPendingLevel )
				SetProgress( LocalizeError("ConnectionFailed"), TEXT(""), 6.0 );
		}
		return 1;
	}
	else if( URL.HasOption(TEXT("pop")) )
	{
		// Pop the hub.
		if( GLevel && GLevel->GetLevelInfo()->HubStackLevel>0 )
		{
			TCHAR Filename[256], SavedPortal[256];
			appSprintf( Filename, TEXT("%s") PATH_SEPARATOR TEXT("Game%i.usa"), *GSys->SavePath, GLevel->GetLevelInfo()->HubStackLevel-1 );
			appStrcpy( SavedPortal, *URL.Portal );
			URL = FURL( &URL, Filename, TRAVEL_Partial );
			URL.Portal = SavedPortal;
		}
		else return 0;
	}
	else if( URL.HasOption(TEXT("restart")) )
	{
		// Handle restarting.
		URL = LastURL;
	}
	else if( (Option=URL.GetOption(TEXT("load="),NULL))!=NULL )
	{
		// Handle loadgame.
		FString Error, Temp=FString::Printf( TEXT("%s") PATH_SEPARATOR TEXT("Save%i.usa?load"), *GSys->SavePath, appAtoi(Option) );
		if( LoadMap(FURL(&LastURL,*Temp,TRAVEL_Partial),NULL,NULL,Error) )
		{
			// Copy the hub stack.
			for( INT i=0; i<GLevel->GetLevelInfo()->HubStackLevel; i++ )
			{
				TCHAR Src[256], Dest[256];//!!
				appSprintf( Src, TEXT("%s") PATH_SEPARATOR TEXT("Save%i%i.usa"), *GSys->SavePath, appAtoi(Option), i );
				appSprintf( Dest, TEXT("%s") PATH_SEPARATOR TEXT("Game%i.usa"), *GSys->SavePath, i );
				GFileManager->Copy( Src, Dest );
			}
			while( 1 )
			{
				Temp = FString::Printf( TEXT("%s") PATH_SEPARATOR TEXT("Game%i.usa"), *GSys->SavePath, i++ );
				if( GFileManager->FileSize(*Temp)<=0 )
					break;
				GFileManager->Delete( *Temp );
			}
			LastURL = GLevel->URL;
			return 1;
		}
		else return 0;
	}
	else if( (Option=URL.GetOption(TEXT("load2="),NULL))!=NULL )	// JP...
	{
		// JP - This does not handle HubStackLevel, but it will be easy to add, just have not done it yet

		// Handle loadgame.
		const TCHAR	*Option2;
		INT			Index, SaveType;
		FString		Temp;
		FString		Error;
		
		Option2 = URL.GetOption(TEXT("load2type="), NULL);

		if (!Option2)
		{
			debugf( NAME_Log, TEXT("Browse: LoadGame FAILED (no type)"));
			return 0;
		}

		Index = appAtoi(Option);
		SaveType = appAtoi(Option2);
		// Call on the dnSaveLoad module to build the file name for this save game file
		Temp = FString::Printf(TEXT("%s?load"), *dnSaveLoad_BuildSaveGameFileNameFromListIndex((ESaveType)SaveType, Index));

		debugf( NAME_Log, TEXT("Browse: LoadGame BEGIN: %s"), *Temp);

		GSaveLoadHack = true;

		if( LoadMap(FURL(&LastURL,*Temp,TRAVEL_Partial),NULL,NULL,Error) )
		{
			GSaveLoadHack = false;

			LastURL = GLevel->URL;
			// Tell the dnSaveLoad module to update the number of loads on this saved game
			dnSaveLoad_IncreaseLoads(this, (ESaveType)SaveType, Index);
			debugf( NAME_Log, TEXT("Browse: LoadGame SUCCESS: %s"), *Temp);
			return 1;
		}
		else 
		{
			GSaveLoadHack = false;
			debugf( NAME_Log, TEXT("Browse: LoadGame FAILED: %s"), *Temp);
			return 0;
		}
	}																// ...JP

	// Handle normal URL's.
	if( URL.IsLocalInternal() )
	{
		// Local map file.
		return LoadMap( URL, NULL, TravelInfo, Error )!=NULL;
	}
	else if( URL.IsInternal() && GIsClient )
	{
		// Network URL.
		if( GPendingLevel )
			CancelPending();
		GPendingLevel = new UNetPendingLevel( this, URL );
		if( !GPendingLevel->NetDriver )
		{
			SetProgress( TEXT("Networking Failed"), *GPendingLevel->Error, 6.0 );
			delete GPendingLevel;
			GPendingLevel = NULL;
		}
		return 0;
	}
	else if( URL.IsInternal() )
	{
		// Invalid.
		Error = LocalizeError("ServerOpen");
		return 0;
	}
	else
	{
		// External URL.
		appLaunchURL( *URL.String(), TEXT(""), &Error );
		return 0;
	}
}

//
// Notify that level is changing
//
void UGameEngine::NotifyLevelChange()
{
	if( Client && Client->Viewports.Num() && Client->Viewports(0)->Console )
		Client->Viewports(0)->Console->eventNotifyLevelChange();
	if( Client && Client->Viewports.Num() && Client->Viewports(0)->Canvas )
		Client->Viewports(0)->Canvas->NotifyLevelChange();
}

// JEP...
static void PreCacheMesh(UDukeMesh *Mesh)
{
#if 1
	UDukeMeshInstance	*Inst;

	Inst = Cast<UDukeMeshInstance>(Mesh->GetInstance(NULL));
#endif
}
// ...JEP

//
// Load a map.
//
ULevel* UGameEngine::LoadMap( const FURL& URL, UPendingLevel* Pending, const TMap<FString,FString>* TravelInfo, FString& Error )
{
	Error = TEXT("");
	debugf( NAME_Log, TEXT("LoadMap: %s"), *URL.String() );
	GInitRunaway();

	//debugf( NAME_Log, TEXT("GSecondsPerCycle: %f"), (float)(1/GSecondsPerCycle));		// JEP

	double TotalTime = 	appSeconds();		// JEP

	// Remember current level's stack level.
	INT SavedHubStackLevel = GLevel ? GLevel->GetLevelInfo()->HubStackLevel : 0;

	// JEP:
	DOUBLE	SavedTotalGameTimeSeconds = GLevel ? GLevel->TotalGameTimeSeconds : 0;
	INT		SavedNumLoads = GLevel ? GLevel->GetLevelInfo()->NumLoads : 0;
	INT		SavedNumSaves = GLevel ? GLevel->GetLevelInfo()->NumSaves : 0;

	// Display loading screen.
	if( Client && Client->Viewports.Num() && GLevel )
	{
		GLevel->GetLevelInfo()->LevelLoadState = 0;
		//GLevel->GetLevelInfo()->LevelAction = LEVACT_Loading;
		GLevel->GetLevelInfo()->LevelAction = LEVACT_LoadingNoLogo;
		GLevel->GetLevelInfo()->Pauser = TEXT("");
		PaintProgress();
		if( Audio )
			Audio->SetViewport( Audio->GetViewport() );
		GLevel->GetLevelInfo()->LevelAction = LEVACT_None;
	}

	// Get network package map.
	UPackageMap* PackageMap = NULL;
	if( Pending )
		PackageMap = Pending->GetDriver()->ServerConnection->PackageMap;

	// JEP: Make sure this loader does not conflict with the new save/load optimizations
	//	What happens, is sometimes you will load a saved game that references the current map.
	//	When this happens, the Child/Parent relationship has already been setup at verify time, 
	//	and when the loader is reset after this, it mucks all this up
	if (GLevel)
		ResetLoaders( GLevel->GetOuter(), 1, 0 );		

	// Verify that we can load all packages we need.
	UObject* MapParent = NULL;
	try
	{
		BeginLoad();
		if( Pending )
		{
			// Verify that we can load everything needed for client in this network level.
			for( INT i=0; i<PackageMap->List.Num(); i++ )
				PackageMap->List(i).Linker = GetPackageLinker
				(
					PackageMap->List(i).Parent,
					NULL,
					LOAD_Verify | LOAD_Throw | LOAD_NoWarn | LOAD_NoVerify,
					NULL,
					&PackageMap->List(i).Guid
				);
			for( i=0; i<PackageMap->List.Num(); i++ )
				VerifyLinker( PackageMap->List(i).Linker );
			if( PackageMap->List.Num() )
				MapParent = PackageMap->List(0).Parent;
		}
		LoadObject<ULevel>( MapParent, TEXT("MyLevel"), *URL.Map, LOAD_Verify | LOAD_Throw | LOAD_NoWarn, NULL );
		EndLoad();

#if DEMOVERSION
		// If we area demo, prevent third party maps from being loaded.
		if( !Pending || !Pending->DemoRecDriver )
		{
			FString FileName(FString(TEXT("../Maps/"))+URL.Map);
			if( FileName.Right(4).Caps() != TEXT(".UNR"))
				FileName = FileName + TEXT(".unr");
			INT FileSize = GFileManager->FileSize( *FileName );
			debugf(TEXT("Looking for file: %s %d"), *FileName, FileSize);
			if( //FileSize != 0 &&
				( FileName.Caps() != TEXT("../MAPS/DM-TURBINEDEMO.UNR")	|| FileSize != 2135105 ) &&
				( FileName.Caps() != TEXT("../MAPS/DM-PHOBOSDEMO.UNR")	|| FileSize != 1618994 ) &&
				( FileName.Caps() != TEXT("../MAPS/DM-MORPHEUSDEMO.UNR")|| FileSize != 1193759 ) &&
				( FileName.Caps() != TEXT("../MAPS/DM-TEMPESTDEMO.UNR")	|| FileSize != 2152238 ) &&
				( FileName.Caps() != TEXT("../MAPS/CTF-CORETDEMO.UNR")	|| FileSize != 3498978 ) &&
				( FileName.Caps() != TEXT("../MAPS/DOM-SESMARDEMO.UNR")	|| FileSize != 2155658 ) &&
				( FileName.Caps() != TEXT("../MAPS/ENTRY.UNR")			|| FileSize != 34822 ) &&
				( FileName.Caps() != TEXT("../MAPS/UT-LOGO-MAP.UNR")	|| FileSize != 34884 ) )
			{
				Error = TEXT("Sorry, only the retail version of UT can load third party maps.");
				SetProgress( LocalizeError(TEXT("UrlFailed"),TEXT("Core")), *Error, 6.0 );
				return NULL;
			}
		}
#endif
	}
	catch( TCHAR* CatchError )
	{
		// Safely failed loading.
		EndLoad();
		Error = CatchError;
		SetProgress( LocalizeError(TEXT("UrlFailed"),TEXT("Core")), CatchError, 6.0 );
		return NULL;
	}

	debugf(_T("EndLoadMap"));
	// Notify of the level change, before we dissociate Viewport actors
	if( GLevel )
		NotifyLevelChange();

	// Dissociate Viewport actors.
	if( Client )
	{
		for( INT i=0; i<Client->Viewports.Num(); i++ )
		{
			APlayerPawn* Actor          = Client->Viewports(i)->Actor;
			ULevel*      Level          = Actor->GetLevel();
			Actor->Player               = NULL;
			Client->Viewports(i)->Actor = NULL;
			Level->DestroyActor( Actor );
		}
	}

	// Clean up game state.
	if( GLevel )
	{
		// Shut down.
		//ResetLoaders( GLevel->GetOuter(), 1, 0 );		// JEP: Commented out (done before verify now)

		if( GLevel->BrushTracker )
		{
			delete GLevel->BrushTracker;
			GLevel->BrushTracker = NULL;
		}
		if( GLevel->NetDriver )
		{
			delete GLevel->NetDriver;
			GLevel->NetDriver = NULL;
		}
		if( GLevel->DemoRecDriver )
		{
			delete GLevel->DemoRecDriver;
			GLevel->DemoRecDriver = NULL;
		}
		if( URL.HasOption(TEXT("push")) )
		{
			// Save the current level minus players actors.
			GLevel->CleanupDestroyed( 1 );
			TCHAR Filename[256];
			appSprintf( Filename, TEXT("%s") PATH_SEPARATOR TEXT("Game%i.usa"), *GSys->SavePath, SavedHubStackLevel );
			SavePackage( GLevel->GetOuter(), GLevel, 0, Filename, GLog );
		}
		GLevel = NULL;
	}

	// JEP ...
	INT OldGNumPreloads = GNumPreloads;
	INT OldGPreloadSize = GPreloadSize;
	INT OldGTexturePreloadSize = GTexturePreloadSize;
	double OldGSerializeTime = GPreloadSerializeTime;
	// ... JEP

	// Load the level and all objects under it, using the proper Guid.
	GLevel = LoadObject<ULevel>( MapParent, TEXT("MyLevel"), *URL.Map, LOAD_NoFail, NULL );

	// JEP ...
	{
		debugf( NAME_Log, TEXT("LoadMap NumPreloads: %i"), GNumPreloads - OldGNumPreloads);
		debugf( NAME_Log, TEXT("LoadMap PreloadSize: %i"), GPreloadSize - OldGPreloadSize);
		debugf( NAME_Log, TEXT("LoadMap TexturePreloadSize: %i"), GTexturePreloadSize - OldGTexturePreloadSize);
		debugf( NAME_Log, TEXT("LoadMap SerializeTime: %f"), (float)(GPreloadSerializeTime - OldGSerializeTime));
	}
	// ... JEP

	// If pending network level.
	if( Pending )
	{
		// If playing this network level alone, ditch the pending level.
		if( Pending && Pending->LonePlayer )
			Pending = NULL;

		// Setup network package info.
		PackageMap->Compute();
		for( INT i=0; i<PackageMap->List.Num(); i++ )
			if( PackageMap->List(i).LocalGeneration!=PackageMap->List(i).RemoteGeneration )
				Pending->NetDriver->ServerConnection->Logf( TEXT("HAVE GUID=%s GEN=%i"), PackageMap->List(i).Guid.String(), PackageMap->List(i).LocalGeneration );
	}

	// Verify classes.
	VERIFY_CLASS_OFFSET( A, Actor,       Owner         );
	VERIFY_CLASS_OFFSET( A, Actor,       TimerCounter  );
	VERIFY_CLASS_OFFSET( A, PlayerPawn,  Player        );
	VERIFY_CLASS_OFFSET( A, PlayerPawn,  MaxStepHeight );

	// Get LevelInfo.
	check(GLevel);
	ALevelInfo* Info = GLevel->GetLevelInfo();
	Info->ComputerName = appComputerName();

	// Handle pushing.
	Info->HubStackLevel
	=	URL.HasOption(TEXT("load")) ? Info->HubStackLevel
	:	URL.HasOption(TEXT("push")) ? SavedHubStackLevel+1
	:	URL.HasOption(TEXT("pop" )) ? Max(SavedHubStackLevel-1,0)
	:	URL.HasOption(TEXT("peer")) ? SavedHubStackLevel
	:	                              0;

	// Handle pending level.
	if( Pending )
	{
		check(Pending==GPendingLevel);

		// Hook network driver up to level.
		GLevel->NetDriver = Pending->NetDriver;
		if( GLevel->NetDriver )
			GLevel->NetDriver->Notify = GLevel;

		// Hook demo playback driver to level
		GLevel->DemoRecDriver = Pending->DemoRecDriver;
		if( GLevel->DemoRecDriver )
			GLevel->DemoRecDriver->Notify = GLevel;

		// Setup level.
		GLevel->GetLevelInfo()->NetMode = NM_Client;
	}
	else check(!GLevel->NetDriver);

	// Set level info.
	if( !URL.GetOption(TEXT("load"),NULL) )
		GLevel->URL = URL;
	Info->EngineVersion = FString::Printf( TEXT("%i"), ENGINE_VERSION );
	Info->MinNetVersion = FString::Printf( TEXT("%i"), ENGINE_MIN_NET_VERSION );
	GLevel->Engine = this;
	if( TravelInfo )
		GLevel->TravelInfo = *TravelInfo;

	// Purge unused objects and flush caches.
	if( appStricmp(GLevel->GetOuter()->GetName(),TEXT("Entry"))!=0 )
	{
		debugf( NAME_Log, TEXT("Purging Unused Objects.") );

		Flush(0);
		{
			for( TObjectIterator<AActor> It; It; ++It )
				if( It->IsIn(GLevel->GetOuter()) )
					It->SetFlags( RF_EliminateObject );
		}
		{
			for( INT i=0; i<GLevel->Actors.Num(); i++ )
				if( GLevel->Actors(i) )
					GLevel->Actors(i)->ClearFlags( RF_EliminateObject );
		}
		CollectGarbage( RF_Native );
		debugf( NAME_Log, TEXT("Unused Objects Purged.") );

	}

	// Init collision.
	debugf( NAME_Log, TEXT("Setting Actor Collision.") );
	GLevel->SetActorCollision( 1 );

	// Setup zone distance table for sound damping. Fast enough: Approx 3 msec.
	debugf( NAME_Log, TEXT("Setting up sound damping table.") );
	QWORD OldConvConn[64];
	QWORD ConvConn[64];
	for( INT i=0; i<64; i++ )
	{
		for( INT j=0; j<64; j++ )
		{
			OldConvConn[i] = GLevel->Model->Zones[i].Connectivity;
			if( i == j )
				GLevel->ZoneDist[i][j] = 0;
			else
				GLevel->ZoneDist[i][j] = 255;
		}
	}
	for( i=1; i<64; i++ )
	{
		for( INT j=0; j<64; j++ )
			for( INT k=0; k<64; k++ )
				if( (GLevel->ZoneDist[j][k] > i) && ((OldConvConn[j] & ((QWORD)1 << k)) != 0) )
					GLevel->ZoneDist[j][k] = i;
		for( j=0; j<64; j++ )
			ConvConn[j] = 0;
		for( j=0; j<64; j++ )
			for( INT k=0; k<64; k++ )
				if( (OldConvConn[j] & ((QWORD)1 << k)) != 0 )
					ConvConn[j] = ConvConn[j] | OldConvConn[k];
		for( j=0; j<64; j++ )
			OldConvConn[j] = ConvConn[j];
	}

	// Update the LevelInfo's time.
	debugf( NAME_Log, TEXT("Updating the level info's time.") );
	GLevel->UpdateTime(Info);

	// Init the game info.
	debugf( NAME_Log, TEXT("Init the game info.") );
	TCHAR Options[1024]=TEXT("");
	TCHAR GameClassName[256]=TEXT("");
	Error=TEXT("");
	for( i=0; i<URL.Op.Num(); i++ )
	{
		appStrcat( Options, TEXT("?") );
		appStrcat( Options, *URL.UnEscape( *URL.Op(i) ) );
		Parse( *URL.Op(i), TEXT("GAME="), GameClassName, ARRAY_COUNT(GameClassName) );
	}
	if( GLevel->IsServer() && !Info->Game )
	{
		// Get the GameInfo class.
		debugf( NAME_Log, TEXT("Get the GameInfo clas.") );

		UClass* GameClass=NULL;
		if( !GameClassName[0] )
		{
			GameClass=Info->DefaultGameType;
			if( !GameClass )
				GameClass = StaticLoadClass( AGameInfo::StaticClass(), NULL, Client ? TEXT("ini:Engine.Engine.DefaultGame") : TEXT("ini:Engine.Engine.DefaultServerGame"), NULL, LOAD_NoFail, PackageMap );
		}
		else GameClass = StaticLoadClass( AGameInfo::StaticClass(), NULL, GameClassName, NULL, LOAD_NoFail, PackageMap );

		// Spawn the GameInfo.
		debugf( NAME_Log, TEXT("Game class is '%s'"), GameClass->GetName() );
		Info->Game = (AGameInfo*)GLevel->SpawnActor( GameClass );
		check(Info->Game!=NULL);
	}

	// Listen for clients.
	if( !Client || URL.HasOption(TEXT("Listen")) )
	{
		debugf( NAME_Log, TEXT("Listen for clients.") );

		if( GPendingLevel )
		{
			check(!Pending);
			delete GPendingLevel;
			GPendingLevel = NULL;
		}
		if( !GLevel->Listen( Error ) )
			appErrorf( LocalizeError("ServerListen"), *Error );
	}

	// Init detail.
	Info->bHighDetailMode = 1;
	if
	(	Client
	&&	Client->Viewports.Num()
	&&	Client->Viewports(0)->RenDev
	&&	!Client->Viewports(0)->RenDev->HighDetailActors )
		Info->bHighDetailMode = 0;

	// Init level gameplay info.
	GLevel->iFirstDynamicActor = 0;
	if( !Info->bBegunPlay )
	{
		// Lock the level.
		debugf( NAME_Log, TEXT("Bringing %s up for play (%i)..."), GLevel->GetFullName(), appRound(GetMaxTickRate()) );
		GLevel->TimeSeconds = 0;
		GLevel->GetLevelInfo()->TimeSeconds = 0;

		// Init touching actors.
		for( INT i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) )
				GLevel->Actors(i)->Touching.Empty();

		// Kill off actors that aren't interesting to the client.
		if( !GLevel->IsServer() )
		{
			for( INT i=0; i<GLevel->Actors.Num(); i++ )
			{
				AActor* Actor = GLevel->Actors(i);
				if( Actor )
				{
					if( Actor->bStatic || Actor->bNoDelete )
						Exchange( Actor->Role, Actor->RemoteRole );
					else
						GLevel->DestroyActor( Actor );
				}
			}
		}

		// Remove lights that don't affect meshes:
		/*
		for( i=0; i<GLevel->Actors.Num(); i++ )
		{
			AActor* Actor = GLevel->Actors(i);
			if( Actor )
			{
				if(!Actor->AffectMeshes)
					GLevel->Actors(i)=NULL;
					//GLevel->DestroyActor( Actor );
			}
		}
		*/

		// Init scripting.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) )
				GLevel->Actors(i)->InitExecution();

		// Enable actor script calls.
		Info->bBegunPlay = 1;
		Info->bStartup = 1;

		VERIFY_CLASS_OFFSET(A,ZoneInfo,originalFogColor);
		
		// Init the game.
		if( Info->Game )
			Info->Game->eventInitGame( Options, Error );

		// Send PreBeginPlay.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) && !GLevel->Actors(i)->bSpawnInitialized2 )
				GLevel->Actors(i)->eventPreBeginPlay();

		// Set BeginPlay.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) && !GLevel->Actors(i)->bSpawnInitialized2 )
				GLevel->Actors(i)->eventBeginPlay();

		// Set zones.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) && !GLevel->Actors(i)->bSpawnInitialized2 )
				GLevel->SetActorZone( GLevel->Actors(i), 1, 1 );

		// Begin scripting.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) && !GLevel->Actors(i)->bSpawnInitialized2 )
				GLevel->Actors(i)->eventSetInitialState();

		// Post begin play.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) && !GLevel->Actors(i)->bSpawnInitialized2 )
				GLevel->Actors(i)->eventPostBeginPlay();

		// Check for encroachment.
		for( i=0; i<GLevel->Actors.Num(); i++ )
			if( GLevel->Actors(i) && GLevel->CheckEncroachment( GLevel->Actors(i), GLevel->Actors(i)->Location, GLevel->Actors(i)->Rotation, 1 ) )
				GLevel->DestroyActor( GLevel->Actors(i) );

		// Find bases
		for( i=0; i<GLevel->Actors.Num(); i++ )
		{
			if( GLevel->Actors(i) && !GLevel->Actors(i)->bSpawnInitialized2 ) 
			{
				if ( GLevel->Actors(i)->AttachTag != NAME_None )
				{
					//find actor to attach self onto
					for( INT j=0; j<GLevel->Actors.Num(); j++ )
					{
						if( GLevel->Actors(j) && (GLevel->Actors(j)->Tag == GLevel->Actors(i)->AttachTag) )
						{
							GLevel->Actors(i)->SetBase(GLevel->Actors(j), 0);
							break;
						}
					}
				}
				else if( !GLevel->Actors(i)->Base && GLevel->Actors(i)->bCollideWorld 
				 && (GLevel->Actors(i)->IsA(ADecoration::StaticClass()) || GLevel->Actors(i)->IsA(AInventory::StaticClass()) || GLevel->Actors(i)->IsA(APawn::StaticClass())) 
				 &&	((GLevel->Actors(i)->Physics == PHYS_None) || (GLevel->Actors(i)->Physics == PHYS_Rotating)) )
				{
					 GLevel->Actors(i)->FindBase();
					 if ( GLevel->Actors(i)->Base == Info )
						 GLevel->Actors(i)->SetBase(NULL, 0);
				}
			}
		}
		Info->bStartup = 0;
	}
	else GLevel->TimeSeconds = GLevel->GetLevelInfo()->TimeSeconds;

	// Rearrange actors: static first, then others.
	TArray<AActor*> Actors;
	Actors.AddItem(GLevel->Actors(0));
	Actors.AddItem(GLevel->Actors(1));
	for( i=2; i<GLevel->Actors.Num(); i++ )
		if( GLevel->Actors(i) && GLevel->Actors(i)->bStatic )
			Actors.AddItem( GLevel->Actors(i) );
	GLevel->iFirstDynamicActor=Actors.Num();
	for( i=2; i<GLevel->Actors.Num(); i++ )
		if( GLevel->Actors(i) && !GLevel->Actors(i)->bStatic )
			Actors.AddItem( GLevel->Actors(i) );
	GLevel->Actors.Empty();
	GLevel->Actors.Add( Actors.Num() );
	for( i=0; i<Actors.Num(); i++ )
		GLevel->Actors(i) = Actors(i);

	// Cleanup profiling.
#if DO_GUARD_SLOW
	for( TObjectIterator<UFunction> It; It; ++It )
		It->Calls = It->Cycles=0;
	GTicks=1;
#endif

	// Client init.
	if( Client )
	{
		// Match Viewports to actors.
		MatchViewportsToActors( Client, GLevel->IsServer() ? GLevel : GEntry, URL );

		// Init brush tracker.
		if( appStricmp(GLevel->GetOuter()->GetName(),TEXT("Entry"))!=0 )//!!
			GLevel->BrushTracker = GNewBrushTracker( GLevel );

		// Set up audio.
		if( Audio )
			Audio->SetViewport( Audio->GetViewport() );

		// Reset viewports.
		for( INT i=0; i<Client->Viewports.Num(); i++ )
		{
			UViewport* Viewport = Client->Viewports(i);
			Viewport->Input->ResetInput();
			if( Viewport->RenDev )
				Viewport->RenDev->Flush(1);
		}
	}

	// Init detail.
	GLevel->DetailChange( Info->bHighDetailMode );

	// JEP... Handle carrying over levelinfo values from level to level, as well as autosave
	if (!URL.HasOption(TEXT("load")) && !URL.HasOption(TEXT("load2")))
	{
		if (URL.Map.Left(5) != TEXT("Entry") && LastURL.Map.Left(5) != TEXT("Entry"))
		{
			// If not loading a saved game, and not coming from entry level, carry over total time
			GLevel->TotalGameTimeSeconds = SavedTotalGameTimeSeconds;
			Info->TotalGameTimeSeconds = GLevel->TotalGameTimeSeconds;
				
			// Carry over numloads
			Info->NumLoads = SavedNumLoads;
			Info->NumSaves = SavedNumSaves;

			if (!URL.HasOption(TEXT("noauto")) && !URL.HasOption(TEXT("restart")) && Info->NetMode == NM_Standalone && !Info->Game->bDeathMatch)
			{
				// Autosave
				GAutoSaveGame = true;
				GWaitingForScreenshot = true;
				GScreenshotReady = false;

			}
		}
		else
		{
			// Reset total game time
			Info->GameTimeSeconds = 0.0f;
			GLevel->GameTimeSeconds = 0.0;
			
			// Reset num loads
			Info->NumLoads = 0;
			Info->NumSaves = 0;
		}
		
		// Unless we are loading, we ALWAYS Reset the normal game time
		Info->GameTimeSeconds = 0.0f;
		GLevel->GameTimeSeconds = 0.0;
	}
	else
	{
		// We are loading a map, since we don't manually serialize the savegame time in ULevel, copy it from
		//	the LeveInfo (which is auto serialized because it's a script class)
		GLevel->GameTimeSeconds = Info->GameTimeSeconds;
		GLevel->TotalGameTimeSeconds = Info->TotalGameTimeSeconds;
	}
	// ... JEP

	// JEP... (Unlock the map)
	if (URL.Map.Left(5) != TEXT("Entry") && Info->NetMode == NM_Standalone && !Info->Game->bDeathMatch)
	{		
		// Create a MapLocations object, find the map, enable it, and save the config
		//	NOTEZ - We only need to create this maplocations object once... but for now...
		AMapLocations *MapLoc = (AMapLocations*)StaticConstructObject(AMapLocations::StaticClass());

		if (MapLoc)
		{
			for (INT i = 0; i< MapLoc->NumMaps; i++)
			{
				if (appStrstr(*MapLoc->Maps[i].URL, *URL.Map))
				{
					// Found it
					MapLoc->Maps[i].Enabled = true;
					//debugf( NAME_Log, TEXT("UnLocking Map: %s"), *URL.Map);
					break;
				}
			}

			if (i < MapLoc->NumMaps)
				MapLoc->SaveConfig();		// Map was found, and enabled, save the config

			// Done
			MapLoc->ConditionalDestroy();
			delete MapLoc;
		}
	}
	// ...JEP

	// Remember the URL.
	LastURL = URL;

	// Remember DefaultPlayer options.
	if( GIsClient )
	{
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Name" ),  TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Team" ),  TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Mesh"),   TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Face" ),  TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Arms" ),  TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Torso" ), TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Legs" ),  TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Voice" ), TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Icon" ),  TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("OverrideClass" ), TEXT("User") );
		URL.SaveURLConfig( TEXT("DefaultPlayer"), TEXT("Spectate" ), TEXT("User") );
	}

	// Notify the level info that we just finished loading it and play is just about to commence.
	if(Info)
		Info->eventFinishedLoading();

	// JEP
	TotalTime = appSeconds() - TotalTime;
	debugf( NAME_Log, TEXT("LoadMap TotalTime: %f"), (float)TotalTime);

	// JEP...
	#if 1
	{
		for( TObjectIterator<UDukeMesh> It; It; ++It )
			PreCacheMesh(*It);
	}
	#endif
	// ...JEP

	// Successfully started local level.
	return GLevel;
}

/*-----------------------------------------------------------------------------
	Game Viewport functions.
-----------------------------------------------------------------------------*/

//
// Draw a global view.
//
void UGameEngine::Draw( UViewport* Viewport, UBOOL Blit, BYTE* HitData, INT* HitSize )
{
	// If not up and running yet, don't draw.
	if( !GIsRunning )
		return;
	UpdateConnectingMessage();

	// Get view location.
	AActor*      ViewActor    = Viewport->Actor;
	FVector      ViewLocation = ViewActor->Location;
	FRotator     ViewRotation = ViewActor->Rotation;
	Viewport->Actor->eventPlayerCalcView( ViewActor, ViewLocation, ViewRotation );
	check(ViewActor);

	Viewport->RenDev->ViewLocation=ViewLocation;
	Viewport->RenDev->ViewRotation=ViewRotation;

	// Precaching message.
	BYTE SavedAction = ViewActor->Level->LevelAction;
	if( Viewport->RenDev->PrecacheOnFlip && !Viewport->bSuspendPrecaching )
		ViewActor->Level->LevelAction = LEVACT_Precaching;

	// See if viewer is inside world.
	DWORD LockFlags=0;

	if (ViewActor->Region.Zone->FogEnabled)	// Set fog flag if a fog zone.
	{
		LockFlags = LOCKR_LightDiminish;
	}

	FCheckResult Hit;
	if( !GLevel->Model->PointCheck(Hit,NULL,ViewLocation,FVector(0,0,0),0) )
		LockFlags |= LOCKR_ClearScreen;

#if defined(LEGEND) //MWP
	if( Viewport->Actor->IsA( APlayerPawn::StaticClass() ) )
	{
		// call the PlayerPawn Render Control Interface (RCI) to assess clear-screen operations
		if( Viewport->Actor->ClearScreen() )
		{
			LockFlags |= LOCKR_ClearScreen;
		}

		// call the PlayerPawn Render Control Interface (RCI) to assess lighting recomputation
		//
		// WARNING: RecomputeLighting() should *not* return false regularly, or rendering 
		//          performance will be severly compromised
		if( Viewport->Actor->RecomputeLighting() )
		{
			Flush();
		}
	}
#endif

	// Lock the Viewport.
	check(Render);
	FPlane FlashScale = Client->ScreenFlashes ? 0.5*Viewport->Actor->FlashScale : FVector(0.5,0.5,0.5);
	FPlane FlashFog   = Client->ScreenFlashes ? Viewport->Actor->FlashFog : FVector(0,0,0);
	FlashScale.X = Clamp( FlashScale.X, 0.f, 1.f );
	FlashScale.Y = Clamp( FlashScale.Y, 0.f, 1.f );
	FlashScale.Z = Clamp( FlashScale.Z, 0.f, 1.f );
	FlashFog.X   = Clamp( FlashFog.X  , 0.f, 1.f );
	FlashFog.Y   = Clamp( FlashFog.Y  , 0.f, 1.f );
	FlashFog.Z   = Clamp( FlashFog.Z  , 0.f, 1.f );
	
	//if( Viewport->Lock(FlashScale,FlashFog,FPlane(0,0,0,0),LockFlags,HitData,HitSize) )
	if( Viewport->Lock(ViewActor->Region.Zone->FogColor,ViewActor->Region.Zone->FogDensity,ViewActor->Region.Zone->FogDistance,
		FlashScale,FlashFog,FPlane(0,0,0,0),LockFlags /*| LOCKR_ClearScreen*/,HitData,HitSize) )

	{
		// Setup rendering coords.
		FSceneNode* Frame = Render->CreateMasterFrame( Viewport, ViewLocation, ViewRotation, NULL );

		// Update level audio.
		if( Audio )
		{
			clock(GLevel->AudioTickCycles);
			Audio->Update( ViewActor->Region, Frame->Coords );
			unclock(GLevel->AudioTickCycles);
		}

		// Render.
		Render->PreRender( Frame );
		Viewport->Canvas->Render = Render;
		if( Viewport->Console )
			Viewport->Console->PreRender( Frame );
		Viewport->Canvas->Update( Frame );
		Viewport->Actor->eventPreRender( Viewport->Canvas );
#if defined(LEGEND) //MWP
		INT SaveXB = Frame->XB, SaveYB = Frame->YB, SaveX = Frame->X, SaveY = Frame->Y;
		Frame->XB += Viewport->Canvas->OrgX;
		Frame->YB += Viewport->Canvas->OrgY;
		Frame->X = Viewport->Canvas->ClipX;
		Frame->Y = Viewport->Canvas->ClipY;
		Frame->ComputeRenderSize();
#endif
		// JEP:
		if( Frame->X>0 && Frame->Y>0 && (!Viewport->Console || Viewport->Console->GetDrawWorld() || GWaitingForScreenshot))
		{
			Render->DrawWorld( Frame );
			
			if (GWaitingForScreenshot)
			{
				GWaitingForScreenshot = false;
				dnScreenshot_FillScreenshotTexture(Viewport, false);
				GScreenshotReady = true;
			}
		}

#if defined(LEGEND) //MWP
		Frame->XB = SaveXB, Frame->YB = SaveYB, Frame->X = SaveX, Frame->Y = SaveY;
		Frame->ComputeRenderSize();
#endif
		Viewport->RenDev->EndFlash();
		Viewport->RenDev->SetTextureClampMode( 1 );
		Viewport->Actor->eventPostRender( Viewport->Canvas );
		if( Viewport->Console )
		{
			Viewport->Console->PostRender( Frame );
			Viewport->Console->eventPostRender( Viewport->Canvas );
		}
		Viewport->RenDev->SetTextureClampMode( 0 );
		if( Audio )
			Audio->PostRender( Frame );

		Viewport->Canvas->Render = 0;
		Render->PostRender( Frame );
		Viewport->Unlock( Blit );
		Render->FinishMasterFrame();
	}
	ViewActor->Level->LevelAction = SavedAction;

	// Precache now if desired.
	if( Viewport->RenDev->PrecacheOnFlip && !Viewport->bSuspendPrecaching )
	{
		Viewport->RenDev->PrecacheOnFlip = 0;
		Render->Precache( Viewport );
	}
}

void ExportTravel( FOutputDevice& Out, AActor* Actor )
{
	guard(ExportTravel);
	debugf( TEXT("Exporting travelling actor of class %s"), Actor->GetClass()->GetPathName() );//!!xyzzy
	check(Actor);
	if( !Actor->bTravel )
		return;
	Out.Logf( TEXT("Class=%s Name=%s\r\n{\r\n"), Actor->GetClass()->GetPathName(), Actor->GetName() );
	for( TFieldIterator<UProperty> It(Actor->GetClass()); It; ++It )
	{
		for( INT Index=0; Index<It->ArrayDim; Index++ )
		{
			TCHAR Value[1024];
			if
			(	(It->PropertyFlags & CPF_Travel)
			&&	It->ExportText( Index, Value, (BYTE*)Actor, &Actor->GetClass()->Defaults[CPD_Normal](0), 0 ) )
			{
				Out.Log( It->GetName() );
				if( It->ArrayDim!=1 )
					Out.Logf( TEXT("[%i]"), Index );
				Out.Log( TEXT("=") );
				UObjectProperty* Ref = Cast<UObjectProperty>( *It );
				if( Ref && Ref->PropertyClass->IsChildOf(AActor::StaticClass()) )
				{
					UObject* Obj = *(UObject**)( (BYTE*)Actor + It->Offset + Index*It->ElementSize );
					Out.Logf( TEXT("%s\r\n"), Obj ? Obj->GetName() : TEXT("None") );
				}
				Out.Logf( TEXT("%s\r\n"), Value );
			}
		}
	}
	Out.Logf( TEXT("}\r\n") );
	unguard;
}

//
// Jumping viewport.
//
void UGameEngine::SetClientTravel( UPlayer* Player, const TCHAR* NextURL, UBOOL bItems, ETravelType TravelType )
{
	check(Player);

	UViewport* Viewport    = CastChecked<UViewport>( Player );
	Viewport->TravelURL    = NextURL;
	Viewport->TravelType   = TravelType;
	Viewport->bTravelItems = bItems;
}

/*-----------------------------------------------------------------------------
	Tick.
-----------------------------------------------------------------------------*/

//
// Get tick rate limitor.
//
FLOAT UGameEngine::GetMaxTickRate()
{
	static UBOOL LanPlay = ParseParam(appCmdLine(),TEXT("lanplay"));
	if( GLevel && GLevel->NetDriver && !GIsClient )
		return Clamp( LanPlay ? GLevel->NetDriver->LanServerMaxTickRate : GLevel->NetDriver->NetServerMaxTickRate, 10, 120 );
	else if( GLevel && GLevel->NetDriver && GLevel->NetDriver->ServerConnection )
		return GLevel->NetDriver->ServerConnection->CurrentNetSpeed/64;
	else if( GLevel && GLevel->DemoRecDriver && !GLevel->DemoRecDriver->ServerConnection )
		return Clamp( LanPlay ? GLevel->NetDriver->LanServerMaxTickRate : GLevel->DemoRecDriver->NetServerMaxTickRate, 10, 120 );
	else
		return 0;
}

//
// Update everything.
//
void __fastcall UGameEngine::Tick( FLOAT DeltaSeconds )
{
	INT LocalTickCycles=0;
	clock(LocalTickCycles);

	// If all viewports closed, time to exit.
	if( Client && Client->Viewports.Num()==0 )
	{
		debugf( TEXT("All Windows Closed") );
		appRequestExit( 0 );
		return;
	}

	// Update subsystems.
	UObject::StaticTick();				
	GCache.Tick();

	// Tick animation system.
	OMacActor::Tick();

	// Update the level.
	GameCycles=0;
	clock(GameCycles);
	if( GLevel )
	{
		// Decide whether to drop high detail because of frame rate
		if ( Client )
		{
			GLevel->GetLevelInfo()->bDropDetail = (DeltaSeconds > 1.f/Clamp(Client->MinDesiredFrameRate,1.f,100.f));
			GLevel->GetLevelInfo()->bAggressiveLOD = (DeltaSeconds > 1.f/Clamp(Client->MinDesiredFrameRate - 5.f,1.f,100.f));
		}
		// tick the level
		GLevel->Tick( LEVELTICK_All, DeltaSeconds );
	}
	if( GEntry && GEntry!=GLevel )
		GEntry->Tick( LEVELTICK_All, DeltaSeconds );
	if( Client && Client->Viewports.Num() && Client->Viewports(0)->Actor->GetLevel()!=GLevel )
		Client->Viewports(0)->Actor->GetLevel()->Tick( LEVELTICK_All, DeltaSeconds );
	unclock(GameCycles);

	// Handle saving
	if (dnSaveLoad_GSaveGame)
	{
		dnSaveLoad_SaveGame(this, 
							GLevel, 
							dnSaveLoad_GSaveType, 
							dnSaveLoad_GSaveListIndex, 
							*dnSaveLoad_GSaveDescription, 
							dnSaveLoad_GSaveScreenshot);

		dnSaveLoad_GSaveGame = false;
		return;
	}

	// Handle server travelling.
	if( GLevel && GLevel->GetLevelInfo()->NextURL!=TEXT("") )
	{
		if( (GLevel->GetLevelInfo()->NextSwitchCountdown-=DeltaSeconds) <= 0.0 )
		{
			// Travel to new level, and exit.
			TMap<FString,FString> TravelInfo;
			if( GLevel->GetLevelInfo()->NextURL==TEXT("?RESTART") )
			{
				TravelInfo = GLevel->TravelInfo;
			}
			else if( GLevel->GetLevelInfo()->bNextItems )
			{
				TravelInfo = GLevel->TravelInfo;
				for( INT i=0; i<GLevel->Actors.Num(); i++ )
				{
					APlayerPawn* P = Cast<APlayerPawn>( GLevel->Actors(i) );
					if( P && P->Player )
					{
						// Export items and self.
						FStringOutputDevice PlayerTravelInfo;
						ExportTravel( PlayerTravelInfo, P );
						for( AActor* Inv=P->Inventory; Inv; Inv=Inv->Inventory )
							ExportTravel( PlayerTravelInfo, Inv );
						TravelInfo.Set( *P->PlayerReplicationInfo->PlayerName, *PlayerTravelInfo );

						// Prevent local ClientTravel from taking place, since it will happen automatically.
						if( Cast<UViewport>( P->Player ) )
							Cast<UViewport>( P->Player )->TravelURL = TEXT("");
					}
				}
			}
			debugf( TEXT("Server switch level: %s"), *GLevel->GetLevelInfo()->NextURL );
			FString Error;
			Browse( FURL(&LastURL,*GLevel->GetLevelInfo()->NextURL,TRAVEL_Relative), &TravelInfo, Error );
			GLevel->GetLevelInfo()->NextURL = TEXT("");
			return;
		}
	}

	// Handle client travelling.
	if( Client && Client->Viewports.Num() && Client->Viewports(0)->TravelURL!=TEXT("") )
	{
		// Travel to new level, and exit.
		UViewport* Viewport = Client->Viewports( 0 );
		TMap<FString,FString> TravelInfo;

		// Export items.
		if( appStricmp(*Viewport->TravelURL,TEXT("?RESTART"))==0 )
		{
			TravelInfo = GLevel->TravelInfo;
		}
		else if( Viewport->bTravelItems )
		{
			debugf( TEXT("Export travel for: %s"), *Viewport->Actor->PlayerReplicationInfo->PlayerName );
			FStringOutputDevice PlayerTravelInfo;
			ExportTravel( PlayerTravelInfo, Viewport->Actor );
			for( AActor* Inv=Viewport->Actor->Inventory; Inv; Inv=Inv->Inventory )
				ExportTravel( PlayerTravelInfo, Inv );

			// Export other stuff...
			for ( INT i=0; i<GLevel->Actors.Num(); i++ )
			{
				// If this is a bWillTravel actor, export it to travel.
				AActor* ThisActor = GLevel->Actors(i);
				if ( ThisActor && ThisActor->bWillTravel )
					ExportTravel( PlayerTravelInfo, ThisActor );
			}

			TravelInfo.Set( *Viewport->Actor->PlayerReplicationInfo->PlayerName, *PlayerTravelInfo );
		}
		FString Error;
		Browse( FURL(&LastURL,*Viewport->TravelURL,Viewport->TravelType), &TravelInfo, Error );
		Viewport->TravelURL=TEXT("");

		return;
	}

	// Update the pending level.
	if( GPendingLevel )
	{
		GPendingLevel->Tick( DeltaSeconds );
		if( GPendingLevel->Error!=TEXT("") )
		{
			// Pending connect failed.
			SetProgress( LocalizeError("ConnectionFailed"), *GPendingLevel->Error, 4.0 );
			debugf( NAME_Log, LocalizeError("Pending"), *GPendingLevel->URL.String(), *GPendingLevel->Error );
			delete GPendingLevel;
			GPendingLevel = NULL;
		}
		else if( GPendingLevel->Success && !GPendingLevel->FilesNeeded && !GPendingLevel->SentJoin )
		{
			// Attempt to load the map.
			FString Error;
			LoadMap( GPendingLevel->URL, GPendingLevel, NULL, Error );
			if( Error!=TEXT("") )
			{
				SetProgress( LocalizeError("ConnectionFailed"), *Error, 4.0 );
			}
			else if( !GPendingLevel->LonePlayer )
			{
				// Show connecting message, cause precaching to occur.
				GLevel->GetLevelInfo()->LevelAction = LEVACT_Connecting;
				GEntry->GetLevelInfo()->LevelAction = LEVACT_Connecting;
				if( Client )
					Client->Tick();

				// Send join.
				GPendingLevel->SendJoin();
				GPendingLevel->NetDriver = NULL;
				GPendingLevel->DemoRecDriver = NULL;
			}

			// Kill the pending level.
			delete GPendingLevel;
			GPendingLevel = NULL;
		}
	}

	// Render everything.
	INT LocalClientCycles=0;
	if( Client )
	{
		clock(LocalClientCycles);
		Client->Tick();
		unclock(LocalClientCycles);
	}
	ClientCycles=LocalClientCycles;

	unclock(LocalTickCycles);
	TickCycles=LocalTickCycles;
	GTicks++;
	
	// JEP: Handle autosave
	if (GAutoSaveGame && GScreenshotReady)
	{
		dnSaveLoad_SaveGame(this, GLevel, SAVE_Auto, -1, *GLevel->GetLevelInfo()->LocationName, GTempTexture);
		GAutoSaveGame = false;
	}
}

/*-----------------------------------------------------------------------------
	Saving the game.
-----------------------------------------------------------------------------*/

//
// Save the current game state to a file.
//
void UGameEngine::SaveGame( INT Position )
{
	TCHAR Filename[256];
	GFileManager->MakeDirectory( *GSys->SavePath, 0 );
	appSprintf( Filename, TEXT("%s") PATH_SEPARATOR TEXT("Save%i.usa"), *GSys->SavePath, Position );
	GLevel->GetLevelInfo()->LevelAction=LEVACT_Saving;
	PaintProgress();
	GWarn->BeginSlowTask( LocalizeProgress("Saving"), 1, 0 );
	if( GLevel->BrushTracker )
	{
		delete GLevel->BrushTracker;
		GLevel->BrushTracker = NULL;
	}
	GLevel->CleanupDestroyed( 1 );
	if( SavePackage( GLevel->GetOuter(), GLevel, 0, Filename, GLog ) )
	{
		// Copy the hub stack.
		for( INT i=0; i<GLevel->GetLevelInfo()->HubStackLevel; i++ )
		{
			TCHAR Src[256], Dest[256];
			appSprintf( Src, TEXT("%s") PATH_SEPARATOR TEXT("Game%i.usa"), *GSys->SavePath, i );
			appSprintf( Dest, TEXT("%s") PATH_SEPARATOR TEXT("Save%i%i.usa"), *GSys->SavePath, Position, i );
			GFileManager->Copy( Src, Dest );
		}
		while( 1 )
		{
			appSprintf( Filename, TEXT("%s") PATH_SEPARATOR TEXT("Save%i%i.usa"), *GSys->SavePath, Position, i++ );
			if( GFileManager->FileSize(Filename)<=0 )
				break;
			GFileManager->Delete( Filename );
		}
	}
	for( INT i=0; i<GLevel->Actors.Num(); i++ )
		if( Cast<AMover>(GLevel->Actors(i)) )
			Cast<AMover>(GLevel->Actors(i))->SavedPos = FVector(-1,-1,-1);
	GLevel->BrushTracker = GNewBrushTracker( GLevel );
	GWarn->EndSlowTask();
	GLevel->GetLevelInfo()->LevelAction=LEVACT_None;
	
	GCache.Flush();
}

/*-----------------------------------------------------------------------------
	Mouse feedback.
-----------------------------------------------------------------------------*/

//
// Mouse delta while dragging.
//
void UGameEngine::MouseDelta( UViewport* Viewport, DWORD ClickFlags, FLOAT DX, FLOAT DY )
{
	if
	(	(ClickFlags & MOUSE_FirstHit)
	&&	Client
	&&	Client->Viewports.Num()==1
	&&	GLevel
	&&	!Client->Viewports(0)->IsFullscreen()
	&&	GLevel->GetLevelInfo()->Pauser==TEXT("")
	&&  !Viewport->bShowWindowsMouse )
	{
		Viewport->SetMouseCapture( 1, 1, 1 );
	}
	else if( (ClickFlags & MOUSE_LastRelease) && !Client->CaptureMouse )
	{
		Viewport->SetMouseCapture( 0, 0, 0 );
	}
}

//
// Absolute mouse position.
//
void UGameEngine::MousePosition( UViewport* Viewport, DWORD ClickFlags, FLOAT X, FLOAT Y )
{
	if( Viewport )
	{
		Viewport->WindowsMouseX = X;
		Viewport->WindowsMouseY = Y;
	}
}

//
// Mouse clicking.
//
void UGameEngine::Click( UViewport* Viewport, DWORD ClickFlags, FLOAT X, FLOAT Y )
{
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
