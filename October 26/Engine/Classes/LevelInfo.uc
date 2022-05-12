//=============================================================================
// LevelInfo contains information about the current level. There should 
// be one per level and it should be actor 0. UnrealEd creates each level's 
// LevelInfo automatically so you should never have to place one
// manually.
//
// The ZoneInfo properties in the LevelInfo are used to define
// the properties of all zones which don't themselves have ZoneInfo.
//=============================================================================
class LevelInfo extends ZoneInfo
	native;

// Textures.
#exec Texture Import File=Textures\DefaultTexture.pcx

//-----------------------------------------------------------------------------
// Level time.

// Time passage.
var() float TimeDilation;          // Normally 1 - scales real time passage.

// Current time.
var           float	TimeSeconds;			// Time in seconds since level began play.
var			  float TimeDeltaSeconds;  		// Time spent in last frame.

// GameTimeSeconds and TotalGameTimeSeconds are automatically carried from level to level
var			  float GameTimeSeconds;		// Seconds that have elapsed minus pausing (reset every level change)
var			  float TotalGameTimeSeconds;	// Seconds that have elapsed minus pausing (since very beginning of play)

var transient int   Year;          // Year.
var transient int   Month;         // Month.
var transient int   Day;           // Day of month.
var transient int   DayOfWeek;     // Day of week.
var transient int   Hour;          // Hour.
var transient int   Minute;        // Minute.
var transient int   Second;        // Second.
var transient int   Millisecond;   // Millisecond.

//-----------------------------------------------------------------------------
// Text info about level.

var() localized string Title;
var()           string Author;		    // Who built it.
var() localized string IdealPlayerCount;// Ideal number of players for this level. I.E.: 6-8
var() int	RecommendedEnemies;			// number of enemy bots recommended (used by rated games)
var() int	RecommendedTeammates;		// number of friendly bots recommended (used by rated games)
var() unbound localized string LocationName;
var() localized string LevelEnterText;  // Message to tell players when they enter.
var()           string LocalizedPkg;    // Package to look in for localizations.
var             string Pauser;          // If paused, name of person pausing the game.
var levelsummary Summary;
var           string VisibleGroups;		    // List of the group names which were checked when the level was last saved

//-----------------------------------------------------------------------------
// Flags affecting the level.

var() bool           bLonePlayer;     // No multiplayer coordination, i.e. for entranceways.
var bool             bBegunPlay;      // Whether gameplay has begun.
var bool             bPlayersOnly;    // Only update players.
var bool             bHighDetailMode; // Client high-detail mode.
var bool			 bDropDetail;	  // frame rate is below DesiredFrameRate, so drop high detail actors
var bool			 bAggressiveLOD;  // frame rate is well below DesiredFrameRate, so make LOD more aggressive
var bool             bStartup;        // Starting gameplay.
var() bool			 bHumansOnly;	  // Only allow "human" player pawns in this level
var bool			 bNoCheating;	  
var bool			 bAllowFOV;
var() globalconfig bool  bPawnFacialNoise; // Whether or not to apply facial bone noise to pawns

//-----------------------------------------------------------------------------
// Audio properties.
var(Audio) String		 Mp3;				// Default MP3 for level.
var(Audio) String		 Mp3Prefix;			// Context sensitive prefix, if empty, then automatically determined from the map.

var(Audio) const  music  Song;          // Default song for level.
var(Audio) const  byte   SongSection;   // Default song order for level.
var(Audio) const  byte   CdTrack;       // Default CD track for level.
var(Audio) float         PlayerDoppler; // Player doppler shift, 0=none, 1=full.

//-----------------------------------------------------------------------------
// Miscellaneous information.

var() float Brightness;
var() texture Screenshot;
var() texture CloudcastTexture;
var   texture DefaultTexture;
var   int HubStackLevel;

var   transient enum ELevelAction
{
	LEVACT_None,
	LEVACT_Loading,
	LEVACT_LoadingNoLogo,
	LEVACT_Saving,
	LEVACT_Connecting,
	LEVACT_Precaching
} LevelAction;
var   transient int LevelLoadState;

// NumSaves and NumLoads are automatically carried from level to level
// NumLoads is really maintained in a separate file, and updated when the load occurs (see dnSaveLoad_IncreaseLoads)
var int				NumSaves;
var int				NumLoads;

//-----------------------------------------------------------------------------
// Renderer Management.
//var() bool bNeverPrecache;	// NJS: Obsolete, remove.

//-----------------------------------------------------------------------------
// Networking.

var enum ENetMode
{
	NM_Standalone,        // Standalone game.
	NM_DedicatedServer,   // Dedicated server, no local client.
	NM_ListenServer,      // Listen server.
	NM_Client             // Client only, no local server.
} NetMode;
var string ComputerName;  // Machine's name according to the OS.
var string EngineVersion; // Engine version.
var string MinNetVersion; // Min engine version that is net compatible.

//-----------------------------------------------------------------------------
// Gameplay rules

var() class<gameinfo> DefaultGameType;
var GameInfo Game;
var GameReplicationInfo GRI;

//-----------------------------------------------------------------------------
// Navigation point and Pawn lists (chained using nextNavigationPoint and nextPawn).

var const NavigationPoint NavigationPointList;
var const Pawn PawnList;

//-----------------------------------------------------------------------------
// Server related.

var string NextURL;
var bool bNextItems;
var float NextSwitchCountdown;

//-----------------------------------------------------------------------------
// Particle System:
var SoftParticleSystem ParticleSystems;
var TriggerPortal	   TriggerPortals;

//-----------------------------------------------------------------------------
// Actor Performance Management

var int AIProfile[8]; // TEMP statistics
var float AvgAITime;	//moving average of Actor time

//-----------------------------------------------------------------------------
// Physics control
var const BoneRope    RopeList;

//-----------------------------------------------------------------------------
// Spawn notification list
var SpawnNotify SpawnNotify;

//-----------------------------------------------------------------------------
// Direction.
var() int North;

//-----------------------------------------------------------------------------
// Objectives.
var() struct native ObjectiveInfo
{
	var() string Text;
	var() bool	 Complete;
} ObjectiveInfos[6];

//-----------------------------------------------------------------------------
// Functions.

//
// Return the URL of this level on the local machine.
//
native simulated function string GetLocalURL();

//
// Return the URL of this level, which may possibly
// exist on a remote machine.
//
native simulated function string GetAddressURL();

//
// Jump the server to a new level.
//
event ServerTravel( string URL, bool bItems )
{
	if( NextURL=="" )
	{
		bNextItems          = bItems;
		NextURL             = URL;
		if( Game!=None )
		{
			Game.ProcessServerTravel( URL, bItems );
		} else
			NextSwitchCountdown = 0;
	}
}

function PostBeginPlay()
{
	super.PostBeginPlay();
}

event FinishedLoading()
{
	if(Mp3Prefix=="") 
    {
        Mp3Prefix = GetURLMap();
    }

	MusicPlay(Level.Mp3, true);
}

//-----------------------------------------------------------------------------
// Network replication.

replication
{
	reliable if( Role==ROLE_Authority )
		Pauser, TimeDilation, bNoCheating, bAllowFOV, GRI;
}

defaultproperties
{
     TimeDilation=+00001.000000
	 Brightness=1
     Title="Untitled"
     bHiddenEd=True
	 CdTrack=255
	 DefaultTexture=DefaultTexture
	 HubStackLevel=0
	 bHighDetailMode=True
	 PlayerDoppler=0
	 DefaultVisibilityRadius=1500.0
	 VisibleGroups="None"
	 bPawnFacialNoise=true
}
