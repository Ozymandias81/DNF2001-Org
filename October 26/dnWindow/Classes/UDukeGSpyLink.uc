class UDukeGSpyLink extends UBrowserBufferedTcpLink;

// Misc
var UDukeGSpyFact			OwnerFactory;
var IpAddr					MasterServerIpAddr;
var bool					bOpened;

// Params
var string					MasterServerAddress;	// Address of the master server
var int						MasterServerTCPPort;	// Optional port that the master server is listening on
var int 					Region;					// Region of the game server
var int						MasterServerTimeout;
var string					GameName;

// Error messages
var localized string		ResolveFailedError;
var localized string		TimeOutError;
var localized string		CouldNotConnectError;

// for WaitFor
const FoundSecureRequest = 1;
const FoundSecret        = 2;
const NextIP             = 3;
const NextAddress        = 4;

function BeginPlay()
{
	Disable( 'Tick' );
	Super.BeginPlay();
}

function Start()
{
	ResetBuffer();
	
	MasterServerIpAddr.Port = MasterServerTCPPort;

	if( MasterServerAddress=="" )
	{	
		MasterServerAddress = "dukenet.3drealms.com";
		//MasterServerAddress = "master"$Region$".gamespy.com";
	}

	Resolve( MasterServerAddress );
}

function DoBufferQueueIO()
{
	Super.DoBufferQueueIO();
}

function Resolved( IpAddr Addr )
{
	// Set the address
	MasterServerIpAddr.Addr = Addr.Addr;

	// Handle failure.
	if ( MasterServerIpAddr.Addr == 0 )
	{
		Log( "UBrowserGSpyLink: Invalid master server address, aborting." );
		return;
	}

	// Display success message.
	Log( "UBrowserGSpyLink: Master Server is "$MasterServerAddress$":"$MasterServerIpAddr.Port );

	// Bind the local port.
	if ( BindPort() == 0 )
	{
		Log( "UBrowserGSpyLink: Error binding local port, aborting." );
		return;
	}

	Open( MasterServerIpAddr );
	SetTimer( MasterServerTimeout, false );
}

event Timer( optional int TimerNum )
{
	if ( !bOpened )
	{
		Log( "UDukeGSpyLink: Couldn't connect to master server." );
		OwnerFactory.QueryFinished( false, CouldNotConnectError$MasterServerAddress );
		GotoState( 'Done' );	
	}	
}

event Closed()
{
}

// Host resolution failue.
function ResolveFailed()
{
	Log( "UDukeGSpyLink: Failed to resolve master server address, aborting." );
	OwnerFactory.QueryFinished( false, ResolveFailedError$MasterServerAddress );
	GotoState( 'Done' );
}

event Opened()
{
	bOpened = true;
	Enable( 'Tick' );

	WaitFor("\\basic\\\\secure\\", 5, FoundSecureRequest);
}


function Tick( float DeltaTime )
{
	DoBufferQueueIO();
}


function HandleServer( string Text )
{
	local string	Address;
	local string	Port;

	Address = ParseDelimited( Text, ":", 1 );
	Port	= ParseDelimited( ParseDelimited( Text, ":", 2 ), "\\", 1 );

	OwnerFactory.FoundServer( Address, int( Port ), "", GameName );
}


function GotMatch( int MatchData )
{

	switch ( MatchData )
	{
	case FoundSecureRequest:
		Enable( 'Tick' );
		WaitForCount( 6, 5, FoundSecret );
		break;
	case FoundSecret:
		Enable( 'Tick' );
		SendBufferedData( "\\gamename\\"$GameName$"\\location\\"$Region$"\\validate\\"$Validate(WaitResult, GameName)$"\\final\\" );
		GotoState( 'FoundSecretState' );
		break;
	case NextIP:
		Enable( 'Tick' );
		if ( WaitResult == "final\\" )
		{
			OwnerFactory.QueryFinished( true );
			GotoState( 'Done' );
		}
		else
		{
			WaitFor( "\\", 10, NextAddress );
		}
		break;
	case NextAddress:
		Enable( 'Tick' );
		HandleServer( WaitResult );
		WaitFor( "\\", 5, NextIP );
		break;
	default:
		break;
	}
}

function GotMatchTimeout( int MatchData )
{
	Log( "Timed out in master server protocol.  Waiting for "$WaitingFor$" in state "$MatchData );
	OwnerFactory.QueryFinished( false, TimeOutError );
	GotoState( 'Done' );
}

// States
state FoundSecretState 
{
	function Tick( float Delta )
	{
		Global.Tick( Delta );

		// Hack for 0 servers in server list
		if ( !IsConnected() && WaitResult == "\\final\\" )
		{
			OwnerFactory.QueryFinished( true );
			GotoState( 'Done' );
		}
	}

Begin:
	Enable( 'Tick' );
	SendBufferedData( "\\list\\\\gamename\\"$GameName$"\\final\\" );
	WaitFor( "ip\\", 30, NextIP );
}

state Done
{
Begin:
	Disable( 'Tick' );
}

defaultproperties
{
	ResolveFailedError="The master server could not be resolved: "
	CouldNotConnectError="Connecting to the master server timed out: "
	TimeOutError="Timeout talking to the master server"
}