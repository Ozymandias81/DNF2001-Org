/*=============================================================================
	TcpNetDriver.cpp: Unreal TCP/IP driver.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* Created by Tim Sweeney.

Notes:
	* See \msdev\vc98\include\winsock.h and \msdev\vc98\include\winsock2.h 
	  for Winsock WSAE* errors returned by Windows Sockets.
=============================================================================*/

#include "..\..\Engine\Src\EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Declarations.
-----------------------------------------------------------------------------*/

// Classes.
class UTcpNetDriver;
class UTcpipConnection;

// Size of a UDP header.
#define IP_HEADER_SIZE     (20)
#define UDP_HEADER_SIZE    (IP_HEADER_SIZE+8)
#define SLIP_HEADER_SIZE   (UDP_HEADER_SIZE+4)
#define WINSOCK_MAX_PACKET (512)
#define NETWORK_MAX_PACKET (576)

// Variables.
UBOOL GInitialized;

/*-----------------------------------------------------------------------------
	UTcpipConnection.
-----------------------------------------------------------------------------*/

//
// Windows socket class.
//
class DLL_EXPORT_CLASS UTcpipConnection : public UNetConnection
{
	DECLARE_CLASS(UTcpipConnection,UNetConnection,CLASS_Config|CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(UTcpipConnection)

	// Variables.
	sockaddr_in		RemoteAddr;
	SOCKET			Socket;
	UBOOL			OpenedLocally;
	FResolveInfo*	ResolveInfo;

	// Constructors and destructors.
	UTcpipConnection( SOCKET InSocket, UNetDriver* InDriver, sockaddr_in InRemoteAddr, EConnectionState InState, UBOOL InOpenedLocally, const FURL& InURL )
	:	UNetConnection	( InDriver, InURL )
	,	Socket			( InSocket )
	,	RemoteAddr		( InRemoteAddr )
	,	OpenedLocally	( InOpenedLocally )
	{
		// Init the connection.
		State                 = InState;
		MaxPacket			  = WINSOCK_MAX_PACKET;
		PacketOverhead		  = SLIP_HEADER_SIZE;
		InitOut();

		// In connecting, figure out IP address.
		if( InOpenedLocally )
		{
			const TCHAR* s = *InURL.Host;
			for( INT i=0; i<4 && s!=NULL && *s>='0' && *s<='9'; i++ )
			{
				s = appStrchr(s,'.');
				if( s )
					s++;
			}
			if( i==4 && !s )
			{
				// Get numerical address directly.
				IpSetInt(RemoteAddr.sin_addr, inet_addr( appToAnsi(*InURL.Host)));
			}
			else
			{
				// Create thread to resolve the address.
				ResolveInfo = new FResolveInfo( *InURL.Host );
			}
		}
	}

	// UNetConnection interface.
	void LowLevelSend( void* Data, INT Count )
	{
		if( ResolveInfo )
		{
			// If destination address isn't resolved yet, send nowhere.
			if( !ResolveInfo->Resolved() )
			{
				// Host name still resolving.
				return;
			}
			else if( ResolveInfo->GetError() )
			{
				// Host name resolution just now failed.
				debugf( NAME_Log, TEXT("%s"), ResolveInfo->GetError() );
				Driver->ServerConnection->State = USOCK_Closed;
				delete ResolveInfo;
				ResolveInfo = NULL;
				return;
			}
			else
			{
				// Host name resolution just now succeeded.
				RemoteAddr.sin_addr = ResolveInfo->GetAddr();
				debugf( TEXT("Resolved %s (%s)"), ResolveInfo->GetHostName(), *IpString(ResolveInfo->GetAddr()) );
				delete ResolveInfo;
				ResolveInfo = NULL;
			}
		}

		// Send to remote.
		clock(Driver->SendCycles);
		sendto( Socket, (char *)Data, Count, 0, (sockaddr*)&RemoteAddr, sizeof(RemoteAddr) );
		unclock(Driver->SendCycles);
	}
	FString LowLevelGetRemoteAddress()
	{
		return IpString(RemoteAddr.sin_addr,ntohs(RemoteAddr.sin_port));
	}
	FString LowLevelDescribe()
	{
		return FString::Printf
		(
			TEXT("%s %s state: %s"),
			*URL.Host,
			*IpString(RemoteAddr.sin_addr,ntohs(RemoteAddr.sin_port)),
				State==USOCK_Pending	?	TEXT("Pending")
			:	State==USOCK_Open		?	TEXT("Open")
			:	State==USOCK_Closed		?	TEXT("Closed")
			:								TEXT("Invalid")
		);
	}
};
IMPLEMENT_CLASS(UTcpipConnection);

/*-----------------------------------------------------------------------------
	UTcpNetDriver.
-----------------------------------------------------------------------------*/

//
// Windows sockets network driver.
//
class DLL_EXPORT_CLASS UTcpNetDriver : public UNetDriver
{
	DECLARE_CLASS(UTcpNetDriver,UNetDriver,CLASS_Transient|CLASS_Config)

	// Variables.
	sockaddr_in	LocalAddr;
	SOCKET		Socket;

	// Constructor.
	UTcpNetDriver()
	{}

	// UNetDriver interface.
	UBOOL InitConnect( FNetworkNotify* InNotify, FURL& ConnectURL, FString& Error )
	{
		if( !Super::InitConnect( InNotify, ConnectURL, Error ) )
			return 0;
		if( !InitBase( 1, InNotify, ConnectURL, Error ) )
			return 0;

		// Connect to remote.
		sockaddr_in TempAddr;
		TempAddr.sin_family           = AF_INET;
		TempAddr.sin_port             = htons(ConnectURL.Port);
		IpSetBytes(TempAddr.sin_addr, 0, 0, 0, 0);

		// Create new connection.
		ServerConnection = new UTcpipConnection( Socket, this, TempAddr, USOCK_Pending, 1, ConnectURL );
		debugf( NAME_DevNet, TEXT("Game client on port %i, rate %i"), ntohs(LocalAddr.sin_port), ServerConnection->CurrentNetSpeed );

		// Create channel zero.
		GetServerConnection()->CreateChannel( CHTYPE_Control, 1, 0 );

		return 1;
	}
	UBOOL UTcpNetDriver::InitListen( FNetworkNotify* InNotify, FURL& LocalURL, FString& Error )
	{
		if( !Super::InitListen( InNotify, LocalURL, Error ) )
			return 0;
		if( !InitBase( 0, InNotify, LocalURL, Error ) )
			return 0;

		// Update result URL.
		LocalURL.Host = IpString(LocalAddr.sin_addr);
		LocalURL.Port = ntohs( LocalAddr.sin_port );
		debugf( NAME_DevNet, TEXT("TcpNetDriver on port %i"), LocalURL.Port );

		return 1;
	}
	void TickDispatch( FLOAT DeltaTime )
	{
		Super::TickDispatch( DeltaTime );

		// Process all incoming packets.
		BYTE Data[NETWORK_MAX_PACKET];
		sockaddr_in FromAddr;
		for( ; ; )
		{
			// Get data, if any.
			clock(RecvCycles);
			fd_set ReadSet;
			FD_ZERO( &ReadSet );
			FD_SET( Socket, &ReadSet );
			TIMEVAL Wait;
			Wait.tv_sec=0;
			Wait.tv_usec=0;
			INT Result=select(Socket+1,&ReadSet,NULL,NULL,&Wait);
			if( Result==0 || Result==SOCKET_ERROR )
				break;
			INT FromSize = sizeof(FromAddr);
			INT Size = recvfrom( Socket, (char*)Data, sizeof(Data), 0, (sockaddr*)&FromAddr, GCC_OPT_INT_CAST &FromSize );
			unclock(RecvCycles);

			// Handle result.
			if( Size==SOCKET_ERROR )
			{
				if( WSAGetLastError()!=WSAEWOULDBLOCK )
				{
					static UBOOL FirstError=1;
					if( FirstError )
						debugf( TEXT("UDP recvfrom error: %i"), WSAGetLastError() );
					FirstError = 0;
				}
				break;
			}

			// Figure out which socket the received data came from.
			UTcpipConnection* Connection = NULL;
			if( GetServerConnection() && IpMatches(GetServerConnection()->RemoteAddr,FromAddr) )
				Connection = GetServerConnection();
			for( INT i=0; i<ClientConnections.Num() && !Connection; i++ )
				if( IpMatches( ((UTcpipConnection*)ClientConnections(i))->RemoteAddr, FromAddr ) )
					Connection = (UTcpipConnection*)ClientConnections(i);

			// If we didn't find a client connection, maybe create a new one.
			if( !Connection && Notify->NotifyAcceptingConnection()==ACCEPTC_Accept )
			{
				Connection = new UTcpipConnection( Socket, this, FromAddr, USOCK_Open, 0, FURL() );
				Connection->URL.Host = IpString(FromAddr.sin_addr);
				Notify->NotifyAcceptedConnection( Connection );
				ClientConnections.AddItem( Connection );
			}

			// Send the packet to the connection for processing.
			if( Connection )
				Connection->ReceivedRawPacket( Data, Size );
		}
	}
	FString LowLevelGetNetworkNumber()
	{
		return IpString(LocalAddr.sin_addr);
	}
	void LowLevelDestroy()
	{
		// Close the socket.
		if( Socket )
		{
			if( closesocket(Socket) )
				debugf( NAME_Exit, TEXT("WinSock closesocket error (%i)"), WSAGetLastError() );
			Socket=NULL;
			debugf( NAME_Exit, TEXT("WinSock shut down") );
		}
	}

	// UTcpNetDriver interface.
	UBOOL InitBase( UBOOL Connect, FNetworkNotify* InNotify, FURL& URL, FString& Error )
	{
		// Init WSA.
		if( !InitSockets( Error ) )
			return 0;

		// Create UDP socket and enable broadcasting.
		Socket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
		if( Socket == INVALID_SOCKET )
		{
			Socket = 0;
			Error = FString::Printf( TEXT("WinSock: socket failed (%i)"), SocketError() );
			return 0;
		}
		UBOOL TrueBuffer=1;
		if( setsockopt( Socket, SOL_SOCKET, SO_BROADCAST, (char*)&TrueBuffer, sizeof(TrueBuffer) ) )
		{
			Error = FString::Printf( TEXT("%s: setsockopt SO_BROADCAST failed (%i)"), SOCKET_API, SocketError() );
			return 0;
		}
		UBOOL Yes=1;
		if( setsockopt( Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&Yes, sizeof(Yes) ) )
			debugf(TEXT("setsockopt with SO_REUSEADDR failed"));

		// Increase socket queue size, because we are polling rather than threading
		// and thus we rely on Windows Sockets to buffer a lot of data on the server.
		INT RecvSize = Connect ? 0x8000 : 0x20000, SizeSize=sizeof(RecvSize);
		INT SendSize = Connect ? 0x8000 : 0x20000;
		setsockopt( Socket, SOL_SOCKET, SO_RCVBUF, (char*)&RecvSize, SizeSize );
		getsockopt( Socket, SOL_SOCKET, SO_RCVBUF, (char*)&RecvSize, GCC_OPT_INT_CAST &SizeSize );
		setsockopt( Socket, SOL_SOCKET, SO_SNDBUF, (char*)&SendSize, SizeSize );
		getsockopt( Socket, SOL_SOCKET, SO_SNDBUF, (char*)&SendSize, GCC_OPT_INT_CAST &SizeSize );
		debugf( NAME_Init, TEXT("%s: Socket queue %i / %i"), SOCKET_API, RecvSize, SendSize );

		// Bind socket to our port.
		LocalAddr.sin_family    = AF_INET;
		LocalAddr.sin_addr		= getlocalbindaddr( *GLog );
		LocalAddr.sin_port      = 0;
		UBOOL HardcodedPort     = 0;
		if( !Connect )
		{
			// Init as a server.
			HardcodedPort = Parse( appCmdLine(), TEXT("PORT="), URL.Port );
			LocalAddr.sin_port = htons(URL.Port);
		}
		INT AttemptPort = ntohs(LocalAddr.sin_port);
		INT boundport   = bindnextport( Socket, &LocalAddr, HardcodedPort ? 1 : 20, 1 );
		if( boundport==0 )
		{
			Error = FString::Printf( TEXT("%s: binding to port %i failed (%i)"), SOCKET_API, AttemptPort, SocketError() );
			return 0;
		}
		DWORD NoBlock=1;
		if( ioctlsocket( Socket, FIONBIO, &NoBlock ) )
		{
			Error = FString::Printf( TEXT("%s: ioctlsocket failed (%i)"), SOCKET_API, SocketError() );
			return 0;
		}

		// Success.
		return 1;
	}
	UTcpipConnection* GetServerConnection() {return (UTcpipConnection*)ServerConnection;}
};
IMPLEMENT_CLASS(UTcpNetDriver);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
