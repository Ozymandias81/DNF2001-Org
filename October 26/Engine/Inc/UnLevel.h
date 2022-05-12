/*=============================================================================
	UnLevel.h: ULevel definition.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Network notification sink.
-----------------------------------------------------------------------------*/

//
// Accepting connection responses.
//
enum EAcceptConnection
{
	ACCEPTC_Reject,	// Reject the connection.
	ACCEPTC_Accept, // Accept the connection.
	ACCEPTC_Ignore, // Ignore it, sending no reply, while server travelling.
};

//
// The net code uses this to send notifications.
//
class ENGINE_API FNetworkNotify
{
public:
	virtual EAcceptConnection NotifyAcceptingConnection()=0;
	virtual void NotifyAcceptedConnection( class UNetConnection* Connection )=0;
	virtual UBOOL NotifyAcceptingChannel( class UChannel* Channel )=0;
	virtual ULevel* NotifyGetLevel()=0;
	virtual void NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text )=0;
	virtual UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID )=0;
	virtual void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* Error )=0;
	virtual void NotifyProgress( const TCHAR* Str1, const TCHAR* Str2, FLOAT Seconds )=0;
};

/*-----------------------------------------------------------------------------
	FCollisionHashBase.
-----------------------------------------------------------------------------*/

class FCollisionHashBase
{
public:
	// FCollisionHashBase interface.
	virtual ~FCollisionHashBase() {};
	virtual void Tick()=0;
	virtual void AddActor( AActor *Actor )=0;
	virtual void RemoveActor( AActor *Actor )=0;
	virtual FCheckResult* ActorLineCheck( FMemStack& Mem, FVector End, FVector Start, FVector Extent, BYTE ExtraNodeFlags, UBOOL bMeshAccurate )=0;
	virtual FCheckResult* ActorPointCheck( FMemStack& Mem, FVector Location, FVector Extent, DWORD ExtraNodeFlags )=0;
	virtual FCheckResult* ActorRadiusCheck( FMemStack& Mem, FVector Location, FLOAT Radius, DWORD ExtraNodeFlags )=0;
	virtual FCheckResult* ActorEncroachmentCheck( FMemStack& Mem, AActor* Actor, FVector Location, FRotator Rotation, DWORD ExtraNodeFlags )=0;
	virtual void CheckActorNotReferenced( AActor* Actor )=0;
};

ENGINE_API FCollisionHashBase* GNewCollisionHash();

/*-----------------------------------------------------------------------------
	ULevel base.
-----------------------------------------------------------------------------*/

//
// A game level.
//
class ENGINE_API ULevelBase : public UObject, public FNetworkNotify
{
	DECLARE_ABSTRACT_CLASS(ULevelBase,UObject,0)

	// Database.
	TTransArray<AActor*> Actors;

	// Variables.
	class UNetDriver*	NetDriver;
	class UEngine*		Engine;
	FURL				URL;
	class UNetDriver*	DemoRecDriver;

	// Constructors.
	ULevelBase( UEngine* InOwner, const FURL& InURL=FURL(NULL) );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();

	// FNetworkNotify interface.
	void NotifyProgress( const TCHAR* Str1, const TCHAR* Str2, FLOAT Seconds );

protected:
	ULevelBase()
	: Actors( this )
	{}
};

/*-----------------------------------------------------------------------------
	ULevel class.
-----------------------------------------------------------------------------*/

//
// Trace actor options.
//
enum ETraceActorFlags
{
	// Bitflags.
	TRACE_Pawns         = 0x01, // Check collision with pawns.
	TRACE_Movers        = 0x02, // Check collision with movers.
	TRACE_Level         = 0x04, // Check collision with level geometry.
	TRACE_ZoneChanges   = 0x08, // Check collision with soft zone boundaries.
	TRACE_Others        = 0x10, // Check collision with all other kinds of actors.
	TRACE_OnlyProjActor = 0x20, // Check collision with other actors only if they are projectile targets

	// Combinations.
	TRACE_VisBlocking   = TRACE_Level | TRACE_Movers,
	TRACE_AllColliding  = TRACE_Pawns | TRACE_Movers | TRACE_Level | TRACE_Others,
	TRACE_AllEverything = TRACE_Pawns | TRACE_Movers | TRACE_Level | TRACE_ZoneChanges | TRACE_Others,
	TRACE_ProjTargets	= TRACE_OnlyProjActor | TRACE_AllColliding,
};

//
// Level updating.
//
#if _MSC_VER
enum ELevelTick
{
	LEVELTICK_TimeOnly		= 0,	// Update the level time only.
	LEVELTICK_ViewportsOnly	= 1,	// Update time and viewports.
	LEVELTICK_All			= 2,	// Update all.
};
#endif

//
// The level object.  Contains the level's actor list, Bsp information, and brush list.
//
class ENGINE_API ULevel : public ULevelBase
{
	DECLARE_CLASS(ULevel,ULevelBase,0)
	NO_DEFAULT_CONSTRUCTOR(ULevel)

	// Number of blocks of descriptive text to allocate with levels.
	enum {NUM_LEVEL_TEXT_BLOCKS=16};

	// Main variables, always valid.
	TArray<FReachSpec>		ReachSpecs;
	UModel*					Model;
	UTextBuffer*			TextBlocks[NUM_LEVEL_TEXT_BLOCKS];
	DOUBLE                  TimeSeconds;
	DOUBLE					GameTimeSeconds;
	DOUBLE					TotalGameTimeSeconds;
	TMap<FString,FString>	TravelInfo;

	// Only valid in memory.
	FCollisionHashBase* Hash;
	class FMovingBrushTrackerBase* BrushTracker;
	AActor* FirstDeleted;
	struct FActorLink* NewlySpawned;
	UBOOL InTick, Ticked;
	INT iFirstDynamicActor, NetTag;
	BYTE ZoneDist[64][64];

	// Portal quick search list.
	TMap<FName, AActor*> PortalViewMap;

	// Temporary stats.
	INT NetTickCycles, NetDiffCycles, ActorTickCycles, AudioTickCycles, FindPathCycles, MoveCycles, NumMoves, NumReps, NumPV, GetRelevantCycles, NumRPC, SeePlayer, Spawning, Unused, TickedActors;

	// Constructor.
	ULevel( UEngine* InEngine, UBOOL RootOutside );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();
	void PostLoad();

	// ULevel interface.
	virtual void Modify( UBOOL DoTransArrays=0 );
	virtual void SetActorCollision( UBOOL bCollision );
	virtual void __fastcall Tick( ELevelTick TickType, FLOAT DeltaSeconds );
	virtual void TickNetClient( FLOAT DeltaSeconds );
	virtual void TickNetServer( FLOAT DeltaSeconds );
	virtual INT ServerTickClient( UNetConnection* Conn, FLOAT DeltaSeconds );
	virtual void ReconcileActors();
	virtual void RememberActors();
	virtual UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar=*GLog );
	virtual void ShrinkLevel();
	virtual void CompactActors();
	virtual UBOOL Listen( FString& Error );
	virtual UBOOL IsServer();
	virtual UBOOL MoveActor( AActor *Actor, FVector Delta, FRotator NewRotation, FCheckResult &Hit, UBOOL Test=0, UBOOL IgnorePawns=0, UBOOL bIgnoreBases=0, UBOOL bNoFail=0 );
	virtual UBOOL FarMoveActor( AActor* Actor, FVector DestLocation, UBOOL Test=0, UBOOL bNoCheck=0 );
	virtual UBOOL DropToFloor( AActor* Actor );
	virtual UBOOL DestroyActor( AActor* Actor, UBOOL bNetForce=0 );
	virtual void CleanupDestroyed( UBOOL bForce );
	virtual AActor* SpawnActor( UClass* Class, FName InName=NAME_None, AActor* Owner=NULL, class APawn* Instigator=NULL, FVector Location=FVector(0,0,0), FRotator Rotation=FRotator(0,0,0), FName SpawnName=NAME_None, AActor* Template=NULL, UBOOL bNoCollisionFail=0, UBOOL bRemoteOwned=0 );
	virtual ABrush*	SpawnBrush();
	virtual void SpawnViewActor( UViewport* Viewport );
	virtual APlayerPawn* SpawnPlayActor( UPlayer* Viewport, ENetRole RemoteRole, const FURL& URL, FString& Error );
	virtual APlayerPawn* SpawnNewPlayerClass( UPlayer* Viewport, ENetRole RemoteRole, const TCHAR *newClass, FString& Error );
	virtual void SetActorZone( AActor* Actor, UBOOL bTest=0, UBOOL bForceRefresh=0 );
	virtual UBOOL FindSpot( FVector Extent, FVector& Location, UBOOL bCheckActors, UBOOL bAssumeFit );
	virtual void AdjustSpot( FVector &Adjusted, FVector TraceDest, FLOAT TraceLen, FCheckResult &Hit );
	virtual UBOOL CheckEncroachment( AActor* Actor, FVector TestLocation, FRotator TestRotation, UBOOL bTouchNotify );
	virtual UBOOL SinglePointCheck( FCheckResult& Hit, FVector Location, FVector Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors );
	virtual UBOOL SingleLineCheck( FCheckResult& Hit, AActor* SourceActor, const FVector& End, const FVector& Start, DWORD TraceFlags, FVector Extent=FVector(0,0,0), BYTE NodeFlags=0, UBOOL bMeshAccurate=0 );
	virtual FCheckResult* MultiPointCheck( FMemStack& Mem, FVector Location, FVector Extent, DWORD ExtraNodeFlags, ALevelInfo* Level, UBOOL bActors );
	virtual FCheckResult* MultiLineCheck( FMemStack& Mem, FVector End, FVector Start, FVector Size, UBOOL bCheckActors, ALevelInfo* LevelInfo, BYTE ExtraNodeFlags, UBOOL bMeshAccurate=0 );
	virtual void InitStats();
	virtual void GetStats( TCHAR* Result );
	virtual void DetailChange( UBOOL NewDetail );
	virtual INT TickDemoRecord( FLOAT DeltaSeconds );
	virtual INT TickDemoPlayback( FLOAT DeltaSeconds );
	virtual void UpdateTime( ALevelInfo* Info );
	virtual void WelcomePlayer( UNetConnection* Connection, TCHAR* Optional=TEXT("") );

	// FNetworkNotify interface.
	EAcceptConnection NotifyAcceptingConnection();
	void NotifyAcceptedConnection( class UNetConnection* Connection );
	UBOOL NotifyAcceptingChannel( class UChannel* Channel );
	ULevel* NotifyGetLevel() {return this;}
	void NotifyReceivedText( UNetConnection* Connection, const TCHAR* Text );
	void NotifyReceivedFile( UNetConnection* Connection, INT PackageIndex, const TCHAR* Error );
	UBOOL NotifySendingFile( UNetConnection* Connection, FGuid GUID );

	// Accessors.
	ABrush* Brush()
	{
		check(Actors.Num()>=2);
		check(Actors(1)!=NULL);
		check(Actors(1)->Brush!=NULL);
		return (ABrush*)Actors(1);
	}
	INT GetActorIndex( AActor* Actor )
	{
		for( INT i=0; i<Actors.Num(); i++ )
			if( Actors(i) == Actor )
				return i;
		appErrorf( TEXT("Actor not found: %s"), Actor->GetFullName() );
		return INDEX_NONE;
	}
	ALevelInfo* GetLevelInfo()
	{
		check(Actors(0));
		check(Actors(0)->IsA(ALevelInfo::StaticClass()));
		return (ALevelInfo*)Actors(0);
	}
	AZoneInfo* GetZoneActor( INT iZone )
	{
		return Model->Zones[iZone].ZoneActor ? Model->Zones[iZone].ZoneActor : GetLevelInfo();
	}
	void TraceVisible
	(
		FVector&		vTraceDirection,
		FCheckResult&	Hit,			// Item hit.
		AActor*			SourceActor,	// Source actor, this or its parents is never hit.
		const FVector&	Start,			// Start location.
		DWORD           TraceFlags,		// Trace flags.
		int				iDistance
	);
	void TraceVisibleObjects
	(
		UClass*			ParentClass,	
		FVector&		vTraceDirection,
		FCheckResult&	Hit,			// Item hit.
		AActor*			SourceActor,	// Source actor, this or its parents is never hit.
		const FVector&	Start,			// Start location.
		DWORD           TraceFlags,		// Trace flags.
		int				iDistance
	);
};

/*-----------------------------------------------------------------------------
	Iterators.
-----------------------------------------------------------------------------*/

//
// Iterate through all static brushes in a level.
//
class FStaticBrushIterator
{
public:
	// Public constructor.
	FStaticBrushIterator( ULevel *InLevel )
	:	Level   ( InLevel   )
	,	Index   ( -1        )
	{
		checkSlow(Level!=NULL);
		++*this;
	}
	void operator++()
	{
		do
		{
			if( ++Index >= Level->Actors.Num() )
			{
				Level = NULL;
				break;
			}
		} while
		(	!Level->Actors(Index)
		||	!Level->Actors(Index)->IsStaticBrush() );
	}
	ABrush* operator* ()
	{
		checkSlow(Level);
		checkSlow(Index<Level->Actors.Num());
		checkSlow(Level->Actors(Index));
		checkSlow(Level->Actors(Index)->IsStaticBrush());
		return (ABrush*)Level->Actors(Index);
	}
	ABrush* operator-> ()
	{
		checkSlow(Level);
		checkSlow(Index<Level->Actors.Num());
		checkSlow(Level->Actors(Index));
		checkSlow(Level->Actors(Index)->IsStaticBrush());
		return (ABrush*)Level->Actors(Index);
	}
	operator UBOOL()
	{
		return Level != NULL;
	}
protected:
	ULevel*		Level;
	INT		    Index;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
