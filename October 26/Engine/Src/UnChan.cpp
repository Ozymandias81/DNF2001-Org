/*=============================================================================
	UnChan.cpp: Unreal datachannel implementation.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

#define NET_DEBUG_MESSAGES 0
#define NET_DEBUG_REP      0

/*-----------------------------------------------------------------------------
	UChannel implementation.
-----------------------------------------------------------------------------*/

//
// Initialize the base channel.
//
UChannel::UChannel()
{}
void UChannel::Init( UNetConnection* InConnection, INT InChIndex, INT InOpenedLocally )
{
	Connection		= InConnection;
	ChIndex			= InChIndex;
	OpenedLocally	= InOpenedLocally;
	OpenPacketId	= INDEX_NONE;
	NegotiatedVer	= InConnection->NegotiatedVer;
}

//
// Route the UObject::Destroy.
//
INT UChannel::RouteDestroy()
{
	if( Connection && (Connection->GetFlags() & RF_Unreachable) )
	{
		ClearFlags( RF_Destroyed );
		if( Connection->ConditionalDestroy() )
			return 1;
		SetFlags( RF_Destroyed );
	}
	return 0;
}

//
// Set the closing flag.
//
void UChannel::SetClosingFlag()
{
	Closing = 1;
}

//
// Close the base channel.
//
void UChannel::Close()
{
	check(Connection->Channels[ChIndex]==this);
	if
	(	!Closing
	&&	(Connection->State==USOCK_Open || Connection->State==USOCK_Pending) )
	{		
		// Send a close notify, and wait for ack.
		FOutBunch CloseBunch( this, 1 );
		check(!CloseBunch.IsError());
		check(CloseBunch.bClose);
		CloseBunch.bReliable = 1;
		SendBunch( &CloseBunch, 0 );

#if NET_DEBUG_MESSAGES
		debugf( NAME_DevNetTraffic, TEXT( "*** Closing Channel %i" ), CloseBunch.ChIndex );
#endif
	}
}

//
// Base channel destructor.
//
void UChannel::Destroy()
{
	check(Connection);
	check(Connection->Channels[ChIndex]==this);

#if NET_DEBUG_MESSAGES
	debugf( NAME_DevNetTraffic, TEXT( "*** Destroying Channel %i" ), ChIndex );
#endif

	// Free any pending incoming and outgoing bunches.
	for( FOutBunch* Out=OutRec, *NextOut; Out; Out=NextOut )
		{NextOut = Out->Next; delete Out;}
	for( FInBunch* In=InRec, *NextIn; In; In=NextIn )
		{NextIn = In->Next; delete In;}

	// Remove from connection's channel table.
	verify(Connection->OpenChannels.RemoveItem(this)==1);
	Connection->Channels[ChIndex] = NULL;
	Connection                    = NULL;

	Super::Destroy();
}

//
// Handle an acknowledgement on this channel.
//
void UChannel::ReceivedAcks()
{
	check(Connection->Channels[ChIndex]==this);

	// Verify in sequence.
	for( FOutBunch* Out=OutRec; Out && Out->Next; Out=Out->Next )
		check(Out->Next->ChSequence>Out->ChSequence);

	// Release all acknowledged outgoing queued bunches.
	UBOOL DoClose = 0;
	while( OutRec && OutRec->ReceivedAck )
	{
		DoClose |= OutRec->bClose;
		FOutBunch* Release = OutRec;
		OutRec = OutRec->Next;
		delete Release;
		NumOutRec--;
	}

	// If a close has been acknowledged in sequence, we're done.
	if( DoClose || (OpenTemporary && OpenAcked) )
	{
		check(!OutRec);
		delete this;
	}
}

//
// Return the maximum amount of data that can be sent in this bunch without overflow.
//
INT UChannel::MaxSendBytes()
{
	INT ResultBits
	=	Connection->MaxPacket*8
	-	(Connection->Out.GetNumBits() ? 0 : MAX_PACKET_HEADER_BITS)
	-	Connection->Out.GetNumBits()
	-	MAX_PACKET_TRAILER_BITS
	-	MAX_BUNCH_HEADER_BITS;
	return Max( 0, ResultBits/8 );
}

//
// Handle time passing on this channel.
//
void UChannel::Tick()
{
	check(Connection->Channels[ChIndex]==this);
	if( ChIndex==0 && !OpenAcked )
	{
		// Resend any pending packets if we didn't get the appropriate acks.
		for( FOutBunch* Out=OutRec; Out; Out=Out->Next )
		{
			if( !Out->ReceivedAck )
			{
				FLOAT Wait = Connection->Driver->Time-Out->Time;
				checkSlow(Wait>=0.0);
				if( Wait>1.0 )
				{
#if NET_DEBUG_MESSAGES
					debugf( NAME_DevNetTraffic, TEXT("Channel %i ack timeout; resending %i..."), ChIndex, Out->ChSequence );
#endif
					check(Out->bReliable);
					Connection->SendRawBunch( *Out, 0 );
				}
			}
		}
	}
}

//
// Make sure the incoming buffer is in sequence and there are no duplicates.
//
void UChannel::AssertInSequenced()
{
	// Verify that buffer is in order with no duplicates.
	for( FInBunch* In=InRec; In && In->Next; In=In->Next )
		check(In->Next->ChSequence>In->ChSequence);
}

//
// Process a properly-sequenced bunch.
//
UBOOL UChannel::ReceivedSequencedBunch( FInBunch& Bunch )
{
	// Note this bunch's retirement.
	if( Bunch.bReliable )
		Connection->InReliable[ChIndex] = Bunch.ChSequence;

	// Handle a regular bunch.
	if( !Closing )
		ReceivedBunch( Bunch );

	// We have fully received the bunch, so process it.
	if( Bunch.bClose )
	{
		// Handle a close-notify.
		if( InRec )
			appErrorfSlow( TEXT("Close Anomaly %i / %i"), Bunch.ChSequence, InRec->ChSequence );
#if NET_DEBUG_MESSAGES
		debugf( NAME_DevNetTraffic, TEXT("      Channel %i got close-notify"), ChIndex );
#endif
		delete this;
		return 1;
	}
	return 0;
}

//
// Process a raw, possibly out-of-sequence bunch: either queue it or dispatch it.
// The bunch is sure not to be discarded.
//
void UChannel::ReceivedRawBunch( FInBunch& Bunch )
{
	check(Connection->Channels[ChIndex]==this);
	if
	(	Bunch.bReliable
	&&	Bunch.ChSequence!=Connection->InReliable[ChIndex]+1 )
	{
		// If this bunch has a dependency on a previous unreceived bunch, buffer it.
		checkSlow(!Bunch.bOpen);

		// Verify that UConnection::ReceivedPacket has passed us a valid bunch.
		check(Bunch.ChSequence>Connection->InReliable[ChIndex]);

		// Find the place for this item, sorted in sequence.
#if NET_DEBUG_MESSAGES
		debugf( NAME_DevNetTraffic, TEXT("      Queuing bunch with unreceived dependency") );
#endif
		for( FInBunch** InPtr=&InRec; *InPtr; InPtr=&(*InPtr)->Next )
		{
			if( Bunch.ChSequence==(*InPtr)->ChSequence )
			{
				// Already queued.
				return;
			}
			else if( Bunch.ChSequence<(*InPtr)->ChSequence )
			{
				// Stick before this one.
				break;
			}
		}
		FInBunch* New = new(TEXT("FInBunch"))FInBunch(Bunch);
		New->Next     = *InPtr;
		*InPtr        = New;
		NumInRec++;
		check(NumInRec<=RELIABLE_BUFFER);
		AssertInSequenced();
	}
	else
	{
		// Receive it in sequence.
		UBOOL Deleted = ReceivedSequencedBunch( Bunch );
		if( Deleted )
			return;

		// Dispatch any waiting bunches.
		while( InRec )
		{
			if( InRec->ChSequence!=Connection->InReliable[ChIndex]+1 )
				break;
#if NET_DEBUG_MESSAGES
			debugf( NAME_DevNetTraffic, TEXT("      Unleashing queued bunch") );
#endif
			FInBunch* Release = InRec;
			InRec = InRec->Next;
			NumInRec--;
			UBOOL Deleted = ReceivedSequencedBunch( *Release );
			delete Release;
			if( Deleted )
				return;
			AssertInSequenced();
		}
	}
}

//
// Send a bunch if it's not overflowed, and queue it if it's reliable.
//
INT UChannel::SendBunch( FOutBunch* Bunch, UBOOL Merge )
{
	check(!Closing);
	check(Connection->Channels[ChIndex]==this);
	check(!Bunch->IsError());

	// Set bunch flags.
	if( OpenPacketId==INDEX_NONE && OpenedLocally )
	{
		Bunch->bOpen = 1;
		OpenTemporary = !Bunch->bReliable;
	}

	// If channel was opened temporarily, we are never allowed to send reliable packets on it.
	if( OpenTemporary )
		check(!Bunch->bReliable);

	// Contemplate merging.
	FOutBunch* OutBunch = NULL;
	if
	(	Merge
	&&	Connection->LastOut.ChIndex==Bunch->ChIndex
	&&	Connection->AllowMerge
	&&	Connection->LastEnd.GetNumBits()
	&&	Connection->LastEnd.GetNumBits()==Connection->Out.GetNumBits()
	&&	Connection->Out.GetNumBytes()+Bunch->GetNumBytes()+(MAX_BUNCH_HEADER_BITS+MAX_PACKET_TRAILER_BITS+7)/8<=Connection->MaxPacket )
	{
		// Merge.
		check(!Connection->LastOut.IsError());
		Connection->LastOut.SerializeBits( Bunch->GetData(), Bunch->GetNumBits() );
		Connection->LastOut.bReliable |= Bunch->bReliable;
		Connection->LastOut.bOpen     |= Bunch->bOpen;
		Connection->LastOut.bClose    |= Bunch->bClose;
		OutBunch                       = Connection->LastOutBunch;
		Bunch                          = &Connection->LastOut;
		check(!Bunch->IsError());
		Connection->LastStart.Pop( Connection->Out );
		Connection->OutBunAcc--;
	}
	else Merge=0;

	// Find outgoing bunch index.
	if( Bunch->bReliable )
	{
		// Find spot, which was guaranteed available by FOutBunch constructor.
		if( OutBunch==NULL )
		{
			check(NumOutRec<RELIABLE_BUFFER-1+Bunch->bClose);
			Bunch->Next	= NULL;
			Bunch->ChSequence = ++Connection->OutReliable[ChIndex];
			NumOutRec++;
			OutBunch = new(TEXT("FOutBunch"))FOutBunch(*Bunch);
			for( FOutBunch** OutLink=&OutRec; *OutLink; OutLink=&(*OutLink)->Next );
			*OutLink = OutBunch;
		}
		else
		{
			Bunch->Next = OutBunch->Next;
			*OutBunch = *Bunch;
		}
		Connection->LastOutBunch = OutBunch;
	}
	else
	{
		OutBunch = Bunch;
		Connection->LastOutBunch = NULL;//warning: Complex code, don't mess with this!
	}

#if NET_DEBUG_MESSAGES
	debugf( NAME_DevNetTraffic, TEXT("      Send Bunch Index:%i Sequence:%i"), OutBunch->ChIndex, OutBunch->ChSequence );
#endif

	// Send the raw bunch.
	OutBunch->ReceivedAck = 0;
	INT PacketId = Connection->SendRawBunch( *OutBunch, 1 );
	if( OpenPacketId==INDEX_NONE && OpenedLocally )
		OpenPacketId = PacketId;
	if( OutBunch->bClose )
		SetClosingFlag();

	// Update channel sequence count.
	Connection->LastOut = *OutBunch;
	Connection->LastEnd	= FBitWriterMark(Connection->Out);


	return PacketId;
}

//
// Describe the channel.
//
FString UChannel::Describe()
{
	return FString(TEXT("State=")) + (Closing ? TEXT("closing") : TEXT("open") );
}

//
// Return whether this channel is ready for sending.
//
INT UChannel::IsNetReady( UBOOL Saturate )
{
	// If saturation allowed, ignore queued byte count.
	if( NumOutRec>=RELIABLE_BUFFER-1 )
		return 0;
	return Connection->IsNetReady( Saturate );
}

//
// Returns whether the specified channel type exists.
//
UBOOL UChannel::IsKnownChannelType( INT Type )
{
	return Type>=0 && Type<CHTYPE_MAX && ChannelClasses[Type];
}

//
// Negative acknowledgement processing.
//
void UChannel::ReceivedNak( INT NakPacketId )
{
	for( FOutBunch* Out=OutRec; Out; Out=Out->Next )
	{
		// Retransmit reliable bunches in the lost packet.
		if( Out->PacketId==NakPacketId && !Out->ReceivedAck )
		{
			check(Out->bReliable);
#if NET_DEBUG_MESSAGES
			debugf( NAME_DevNetTraffic, TEXT("      Channel %i nak; resending %i..."), Out->ChIndex, Out->ChSequence );
#endif
			Connection->SendRawBunch( *Out, 0 );
		}
	}
}

// UChannel statics.
UClass* UChannel::ChannelClasses[CHTYPE_MAX]={0,0,0,0,0,0,0,0};
IMPLEMENT_CLASS(UChannel)

/*-----------------------------------------------------------------------------
	UControlChannel implementation.
-----------------------------------------------------------------------------*/

//
// Initialize the text channel.
//
UControlChannel::UControlChannel()
{}
void UControlChannel::Init( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally )
{
	Super::Init( InConnection, InChannelIndex, InOpenedLocally );
}

//
// UControlChannel destructor. 
//
void UControlChannel::Destroy()
{
	check(Connection);
	if( RouteDestroy() )
		return;
	check(Connection->Channels[ChIndex]==this);

	Super::Destroy();
}

//
// Handle an incoming bunch.
//
void UControlChannel::ReceivedBunch( FInBunch& Bunch )
{
	check(!Closing);
	for( ; ; )
	{
		FString Text;
		Bunch << Text;
		if( Bunch.IsError() )
			break;
		Connection->Driver->Notify->NotifyReceivedText( Connection, *Text );
	}
}

//
// Text channel FArchive interface.
//
void UControlChannel::Serialize( const TCHAR* Data, EName MsgType )
{
	// Delivery is not guaranteed because NewBunch may fail.
	FOutBunch Bunch( this, 0 );
	Bunch.bReliable = 1;
	FString Text=Data;
	Bunch << Text;
	if( !Bunch.IsError() )
	{
		SendBunch( &Bunch, 1 );
	}
	else
	{
		debugf( NAME_DevNet, TEXT("Control channel bunch overflowed") );
		//!!should signal error somehow
	}
}

//
// Describe the text channel.
//
FString UControlChannel::Describe()
{
	return FString(TEXT("Text ")) + UChannel::Describe();
}

IMPLEMENT_CLASS(UControlChannel);

/*-----------------------------------------------------------------------------
	UActorChannel.
-----------------------------------------------------------------------------*/

//
// Initialize this actor channel.
//
UActorChannel::UActorChannel()
{}
void UActorChannel::Init( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally )
{
	Super::Init( InConnection, InChannelIndex, InOpenedLocally );
	Level			= Connection->Driver->Notify->NotifyGetLevel();
	RelevantTime	= Connection->Driver->Time;
	LastUpdateTime	= Connection->Driver->Time - Connection->Driver->SpawnPrioritySeconds;
}

//
// Set the closing flag.
//
void UActorChannel::SetClosingFlag()
{
	if( Actor )
		Connection->ActorChannels.Remove( Actor );
	UChannel::SetClosingFlag();
}

//
// Close it.
//
void UActorChannel::Close()
{
	UChannel::Close();
	Actor = NULL;
}

//
// Time passes...
//
void UActorChannel::Tick()
{
	UChannel::Tick();
}

//
// Actor channel destructor.
//
void UActorChannel::Destroy()
{
	check(Connection);

    if( RouteDestroy() )
		return;
	
    check(Connection->Channels[ChIndex]==this);

	// Remove from hash and stuff.
	SetClosingFlag();

	// Destroy Recent properties.
	if( Recent.Num() )
	{
		check(ActorClass);
		UObject::ExitProperties( &Recent(0), ActorClass );
	}

	// If we're the client, destroy this actor.
	if( Connection->Driver->ServerConnection )
	{
		check(!Actor || Actor->IsValid());
		check(Level);
		check(Level->IsValid());
		check(Connection);
		check(Connection->IsValid());
		check(Connection->Driver);
		check(Connection->Driver->IsValid());
		if( Actor && !Actor->bNetTemporary )
			Actor->GetLevel()->DestroyActor( Actor, 1 );
	}
	else if( Actor && !OpenAcked )
	{
		// Resend temporary actors if nak'd.
		Connection->SentTemporaries.RemoveItem( Actor );
	}	

	Super::Destroy();
}

//
// Negative acknowledgements.
//
void UActorChannel::ReceivedNak( INT NakPacketId )
{
	UChannel::ReceivedNak(NakPacketId);
	if( ActorClass )
		for( INT i=Retirement.Num()-1; i>=0; i-- )
			if( Retirement(i).OutPacketId==NakPacketId && !Retirement(i).Reliable )
				Dirty.AddUniqueItem(i);
}

//
// Allocate replication tables for the actor channel.
//
void UActorChannel::SetChannelActor( AActor* InActor )
{
	check(!Closing);
	check(Actor==NULL);

	// Set stuff.
	Actor                      = InActor;
	ActorClass                 = Actor->GetClass();
	FClassNetCache* ClassCache = Connection->PackageMap->GetClassNetCache( ActorClass );

	// Add to map.
	Connection->ActorChannels.Set( Actor, this );

	// Allocate replication condition evaluation cache.
	RepEval.AddZeroed( ClassCache->GetRepConditionCount() );

	// Init recent properties.
	if( !InActor->bNetTemporary )
	{
		// Allocate recent property list.
		INT Size = ActorClass->Defaults[CPD_Normal].Num();
		Recent.Add( Size );
		UObject::InitProperties( &Recent(0), Size, ActorClass, NULL, 0 );

		// Init config properties, to force replicate them.
		for( UProperty* It=ActorClass->ConfigLink; It; It=It->ConfigLinkNext )
		{
			if( It->PropertyFlags & CPF_NeedCtorLink )
				It->DestroyValue( &Recent(It->Offset) );
			UBoolProperty* BoolProperty = Cast<UBoolProperty>(It);
			if( !BoolProperty )
				appMemzero( &Recent(It->Offset), It->GetSize() );
			else
				*(DWORD*)&Recent(It->Offset) &= ~BoolProperty->BitMask;
		}
	}

	// Allocate retirement list.
	Retirement.Empty( ActorClass->ClassReps.Num() );
	while( Retirement.Num()<ActorClass->ClassReps.Num() )
		new(Retirement)FPropertyRetirement;
}

//
// Handle receiving a bunch of data on this actor channel.
//

void UActorChannel::ReceivedBunch( FInBunch& Bunch )
{
	check(!Closing);

    // Initialize client if first time through.
	FClassNetCache* ClassCache = NULL;
	UBOOL NewlySpawned = 0;
	if( Actor==NULL )
	{
		if( !Bunch.bOpen )
		{
			appErrorf(TEXT("New actor channel received non-open packet: %i/%i/%i/%i"),Bunch.bOpen,Bunch.bClose,Bunch.bReliable,Bunch.ChIndex);
			//debugf( NAME_DevNetTraffic, TEXT("New actor channel received non-open packet: %i/%i/%i/%i"),Bunch.bOpen,Bunch.bClose,Bunch.bReliable,Bunch.ChIndex);
		}

		// Read class.
		UObject* Object;
		Bunch << Object;
		AActor* InActor = Cast<AActor>( Object );
		if( InActor==NULL )
		{
			// Transient actor.
			UClass* ActorClass = Cast<UClass>( Object );
			check(ActorClass);
			check(ActorClass->IsChildOf(AActor::StaticClass()));
			FVector Location;
			Bunch << Location;
			InActor = Level->SpawnActor( ActorClass, NAME_None, NULL, NULL, Location, FRotator(0,0,0), NAME_None, NULL, 1, 1 );
			NewlySpawned = 1;
            check( InActor );
		}
#if NET_DEBUG_MESSAGES
		debugf( NAME_DevNetTraffic, TEXT("      Spawn %s:"), InActor->GetFullName() );
#endif
		SetChannelActor( InActor );
	}
#if NET_DEBUG_MESSAGES
	else debugf( NAME_DevNetTraffic, TEXT("      Actor %s:"), Actor->GetFullName() );
#endif
	ClassCache = Connection->PackageMap->GetClassNetCache(ActorClass);

	// Owned by connection's player?
	Actor->bNetOwner = 0;
	APlayerPawn* Top = Cast<APlayerPawn>( Actor->GetTopOwner() );
	UPlayer* Player = Top ? Top->Player : NULL;

	// Set quickie replication variables.
	if( Connection->Driver->ServerConnection )
	{
		// We are the client.
		if( Player && Player->IsA( UViewport::StaticClass() ) )
			Actor->bNetOwner = 1;
	}
	else
	{
		// We are the server.
		if( Player==Connection )
			Actor->bNetOwner = 1;
	}

	// Handle the data stream.
	INT             RepIndex   = Bunch.ReadInt( ClassCache->GetMaxIndex() );
	FFieldNetCache* FieldCache = Bunch.IsError() ? NULL : ClassCache->GetFromIndex( RepIndex );
	while( FieldCache )
	{
		// Save current properties.
		//debugf(TEXT("Rep %s: %i"),FieldCache->Field->GetFullName(),RepIndex);
		Actor->PreNetReceive();

		// Receive properties from the net.
		UProperty* It;
		while( FieldCache && (It=Cast<UProperty>(FieldCache->Field))!=NULL )
		{
			// Receive array index.
			BYTE Element=0;
			if( It->ArrayDim != 1 )
				Bunch << Element;

			// Pointer to destiation.
			BYTE* DestActor  = (BYTE*)Actor;
			BYTE* DestRecent = Recent.Num() ? &Recent(0) : NULL;

			// Server, see if UnrealScript replication condition is met.
			if( !Connection->Driver->ServerConnection )
			{
				Exchange(Actor->Role,Actor->RemoteRole);
				DWORD Val=0;
				FFrame( Actor, It->GetOwnerClass(), It->RepOffset, NULL ).Step( Actor, &Val );
				Exchange(Actor->Role,Actor->RemoteRole);

				// Skip if no replication is desired.
				if( !Val || !Actor->bNetOwner )
				{
#if NET_DEBUG_MESSAGES
					debugf( NAME_DevNet, TEXT("Received unwanted property value %s in %s"), It->GetName(), Actor->GetFullName() );
#endif
					DestActor  = NULL;
					DestRecent = NULL;
				}
			}

			// Check property ordering.
			FPropertyRetirement& Retire = Retirement( It->RepIndex + Element );
			if( Bunch.PacketId>=Retire.InPacketId ) //!! problem with reliable pkts containing dynamic references, being retransmitted, and overriding newer versions. Want "OriginalPacketId" for retransmissions?
			{
				// Receive this new property.
				Retire.InPacketId = Bunch.PacketId;
			}
			else
			{
				// Skip this property, because it's out-of-date.
#if NET_DEBUG_MESSAGES
				debugf( NAME_DevNet, TEXT("Received out-of-date %s"), It->GetName() );
#endif
				DestActor  = NULL;
				DestRecent = NULL;
			}

			// Receive property.
			FMemMark Mark(GMem);
			INT   Offset = It->Offset + Element*It->ElementSize;
			BYTE* Data   = DestActor ? (DestActor + Offset) : NewZeroed<BYTE>(GMem,It->ElementSize);
			It->NetSerializeItem( Bunch, Connection->PackageMap, Data );
			if( DestRecent )
				It->CopySingleValue( DestRecent + Offset, Data );
			Mark.Pop();

			// Successfully received it.
#if NET_DEBUG_MESSAGES
			debugf( NAME_DevNetTraffic, TEXT("         %s"), It->GetName() );
#endif

			// Get next.
			RepIndex   = Bunch.ReadInt( ClassCache->GetMaxIndex() );
			FieldCache = Bunch.IsError() ? NULL : ClassCache->GetFromIndex( RepIndex );
		}

		// Process important changed properties.
		Actor->PostNetReceive();
        Actor->eventPostNetReceive();

		// Handle function calls.
		if( FieldCache && Cast<UFunction>(FieldCache->Field) )
		{
			FName Message = FieldCache->Field->GetFName();
			UFunction* Function = Actor->FindFunction( Message );
			check(Function);

			// See if UnrealScript replication condition is met.
			UBOOL Ignore=0;
			for( UFunction* Test=Function; Test->GetSuperFunction(); Test=Test->GetSuperFunction() );
			if( !Connection->Driver->ServerConnection )
			{
				Exchange(Actor->Role,Actor->RemoteRole);
				DWORD Val=0;
				FFrame( Actor, Test->GetOwnerClass(), Test->RepOffset, NULL ).Step( Actor, &Val );
				Exchange(Actor->Role,Actor->RemoteRole);
				if( !Val || !Actor->bNetOwner )
				{
					debugf( NAME_DevNet, TEXT("Received unwanted function %s in %s"), *Message, Actor->GetFullName() );
					Ignore = 1;
				}
			}
#if NET_DEBUG_MESSAGES
			debugf( NAME_DevNetTraffic, TEXT("      Received RPC: %s"), *Message );
#endif

			// Get the parameters.
			FMemMark Mark(GMem);
			BYTE* Parms = new(GMem,MEM_Zeroed,Function->ParmsSize)BYTE;
			for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
				if( Connection->PackageMap->ObjectToIndex(*It)!=INDEX_NONE )
					if( It->IsA(UBoolProperty::StaticClass()) || Bunch.ReadBit() )
						It->NetSerializeItem(Bunch,Connection->PackageMap,Parms+It->Offset);

			// Call the function.
			if( !Ignore )
			{
				// The bClientDemoNetFunc flag gets cleared in ProcessDemoRecFunction
				Actor->bClientDemoNetFunc = 1;
				Actor->ProcessEvent( Function, Parms );
			}

			// Destroy the parameters.
			//warning: highly dependent on UObject::ProcessEvent freeing of parms!
			{for( UProperty* Destruct=Function->ConstructorLink; Destruct; Destruct=Destruct->ConstructorLinkNext )
				if( Destruct->Offset < Function->ParmsSize )
					Destruct->DestroyValue( Parms + Destruct->Offset );}
			Mark.Pop();

			// Next.
			RepIndex   = Bunch.ReadInt( ClassCache->GetMaxIndex() );
			FieldCache = Bunch.IsError() ? NULL : ClassCache->GetFromIndex( RepIndex );
		}
		else if( FieldCache )
		{
			appErrorfSlow( TEXT("Invalid replicated field %i in %s"), RepIndex, Actor->GetFullName() );
			return;
		}
	}

	// Notify the actor.
	if ( NewlySpawned )
		Actor->eventPostNetInitial();

	// If this is the player's channel and the connection was pending, mark it open.
	if
	(	Connection->Driver
	&&	Connection == Connection->Driver->ServerConnection
	&&	Connection->State==USOCK_Pending
	&&	Actor->bNetOwner
	&&	Actor->IsA(APlayerPawn::StaticClass()) )
	{
		Connection->HandleClientPlayer( CastChecked<APlayerPawn>(Actor) );
	}
}

//
// Replicate this channel's actor differences.
//
void UActorChannel::ReplicateActor()
{
	check(Actor);
	check(!Closing);
	//debugf(TEXT("Replicate %s:%s"),ActorClass->GetName(),Actor->GetFullName );

	// Create an outgoing bunch, and skip this actor if the channel is saturated.
	FOutBunch Bunch( this, 0 );
	if( Bunch.IsError() )
		return;

	// Send initial stuff.
	if( OpenPacketId!=INDEX_NONE )
	{
		Actor->bNetInitial = 0;
		if( !SpawnAcked && OpenAcked )
		{
			// After receiving ack to the spawn, force refresh of all subsequent unreliable packets, which could
			// have been lost due to ordering problems. Note: We could avoid this by doing it in FActorChannel::ReceivedAck,
			// and avoid dirtying properties whose acks were received *after* the spawn-ack (tricky ordering issues though).
			SpawnAcked = 1;
			for( INT i=Retirement.Num()-1; i>=0; i-- )
				if( Retirement(i).OutPacketId!=INDEX_NONE && !Retirement(i).Reliable )
					Dirty.AddUniqueItem(i);
		}
	}
	else
	{
		Actor->bNetInitial = 1;
		Bunch.bClose    =  Actor->bNetTemporary;
		Bunch.bReliable = !Actor->bNetTemporary;
	}

	// Get class network info cache.
	FClassNetCache* ClassCache = Connection->PackageMap->GetClassNetCache(Actor->GetClass());
	check(ClassCache);

	// Owned by connection's player?
	Actor->bNetOwner = 0;
	for( AActor* Top=Actor; Top->Owner; Top=Top->Owner );
	UPlayer* Player = Top->IsA(APlayerPawn::StaticClass()) ? ((APlayerPawn*)Top)->Player : NULL;
	UBOOL bRecordingDemo = Connection->Driver->IsA( UDemoRecDriver::StaticClass() );
	UBOOL bClientDemo = bRecordingDemo && Actor->Level->NetMode == NM_Client;

	// Set quickie replication variables.
	if( bRecordingDemo )
		Actor->bNetOwner = bClientDemo ? (Player && Cast<UViewport>(Player)!=NULL) : 1;
	else	
		Actor->bNetOwner = Connection->Driver->ServerConnection ? Cast<UViewport>(Player)!=NULL : Player==Connection;

	// If initial, send init data.
	if( Actor->bNetInitial && OpenedLocally )
	{
		if( Actor->bStatic || Actor->bNoDelete )
		{
			// Persitent actor.
			Bunch << Actor;
		}
		else
		{
			// Transient actor.
			Bunch << ActorClass << Actor->Location;
			if( Recent.Num() )
				((AActor*)&Recent(0))->Location = Actor->Location;
        }
	}

	// Save out the actor's RemoteRole, and downgrade it if necessary.
	BYTE ActualRemoteRole=Actor->RemoteRole;
	if( Actor->RemoteRole==ROLE_AutonomousProxy && ((Actor->Instigator && !Actor->Instigator->bNetOwner && !Actor->bNetOwner) || bRecordingDemo) )
		Actor->RemoteRole=ROLE_SimulatedProxy;
	Actor->bSimulatedPawn = Actor->bIsPawn && Actor->RemoteRole==ROLE_SimulatedProxy;

	// Get memory for retirement list.
	FMemMark Mark(GMem);
	appMemzero( &RepEval(0), RepEval.Num() );
	INT* Reps = New<INT>( GMem, Retirement.Num() ), *LastRep;
	UBOOL		FilledUp = 0;

	// Figure out which properties to replicate.
	BYTE*   CompareBin = Recent.Num() ? &Recent(0) : &ActorClass->Defaults[CPD_Normal](0);
	INT     iCount     = ClassCache->RepProperties.Num();
	LastRep            = Actor->GetOptimizedRepList( CompareBin, &Retirement(0), Reps, Connection->PackageMap );
	if( Actor->ShouldDoScriptReplication() )
	{
		for( INT iField=0; iField<iCount; iField++  )
		{
			FFieldNetCache* FieldCache = ClassCache->RepProperties(iField);
			UProperty* It = CastChecked<UProperty>(FieldCache->Field);
			BYTE& Eval = RepEval(FieldCache->ConditionIndex);
			if( Eval!=2 )
			{
				UObjectProperty* Op = Cast<UObjectProperty>(It);
				for( INT Index=0; Index<It->ArrayDim; Index++ )
				{
					// Evaluate need to send the property.
#if NET_DEBUG_REP
					debugf( TEXT("   Evaluate Rep (%s) %s"), Actor->GetFullName(),It->GetName() );
#endif
					INT Offset = It->Offset + Index*It->ElementSize;
					BYTE* Src = (BYTE*)Actor + Offset;
					if( Op && !Connection->PackageMap->CanSerializeObject(*(UObject**)Src) )
					{
						Src = NULL;
					}
					if( !It->Identical(CompareBin+Offset,Src) )
					{
#if NET_DEBUG_REP
						debugf( TEXT("   !is not identical. Recent? %i"), Recent.Num() );
#endif
						if( !(Eval & 2) )
						{
							DWORD Val=0;
							FFrame( Actor, It->RepOwner->GetOwnerClass(), It->RepOwner->RepOffset, NULL ).Step( Actor, &Val );
							Eval = Val | 2;
						}
						if( Eval & 1 )
							*LastRep++ = It->RepIndex+Index;
					}
				}
			}
		}
	}
	check(!Bunch.IsError());

	// Add dirty properties to list.
	for( INT i=Dirty.Num()-1; i>=0; i-- )
	{
		INT D=Dirty(i);
		for( INT* R=Reps; R<LastRep; R++ )
			if( *R==D )
				break;
		if( R==LastRep )
			*LastRep++=D;
	}
	// Replicate those properties.
	for( INT* iPtr=Reps; iPtr<LastRep; iPtr++ )
	{
		// Get info.
		FRepRecord* Rep    = &ActorClass->ClassReps(*iPtr);
		UProperty*	It     = Rep->Property;
		INT         Index  = Rep->Index;
		INT         Offset = It->Offset + Index*It->ElementSize;

		// Figure out field to replicate.
		FFieldNetCache* FieldCache
		=	It->GetFName()==NAME_Role
		?	ClassCache->GetFromField(Connection->Driver->RemoteRoleProperty)
		:	It->GetFName()==NAME_RemoteRole
		?	ClassCache->GetFromField(Connection->Driver->RoleProperty)
		:	ClassCache->GetFromField(It);
		check(FieldCache);

		// Send property name and optional array index.
        FBitWriterMark Mark( Bunch );
		Bunch.WriteInt( FieldCache->FieldNetIndex, ClassCache->GetMaxIndex() );
		if( It->ArrayDim != 1 )
		{
			BYTE Element = Index;
			Bunch << Element;
		}

		// Send property.
		UBOOL Mapped = It->NetSerializeItem( Bunch, Connection->PackageMap, (BYTE*)Actor + Offset );
#if NET_DEBUG_MESSAGES
		debugf( NAME_DevNetTraffic, TEXT("   Send (%s) %s %i Ch:%i Seq:%i"), Actor->GetFullName(),It->GetName(),Mapped, Bunch.ChIndex, Bunch.ChSequence );
#endif
		if( !Bunch.IsError() )
		{
			// Update recent value.
			if( Recent.Num() )
			{
				if( Mapped )
					It->CopySingleValue( &Recent(Offset), (BYTE*)Actor + Offset );
				else
					appMemzero( &Recent(Offset), It->ElementSize );
			}
			Actor->GetLevel()->NumReps++;
		}
		else
		{
			// Stop the changes because we overflowed.
			Mark.Pop( Bunch );
			LastRep  = iPtr;
			FilledUp = 1;
			break;
		}
	}

	// If not empty, send and mark as updated.
	if( Bunch.GetNumBits() )
	{
		INT PacketId = SendBunch( &Bunch, 1 );
		for( INT* Rep=Reps; Rep<LastRep; Rep++ )
		{
			Dirty.RemoveItem(*Rep);
			FPropertyRetirement& Retire = Retirement(*Rep);
			Retire.OutPacketId = PacketId;
			Retire.Reliable    = Bunch.bReliable;
		}
		if( Actor->bNetTemporary )
		{
			Connection->SentTemporaries.AddItem( Actor );
		}
	}

	// If we evaluated everything, mark LastUpdateTime, even if nothing changed.
	if( !FilledUp )
		LastUpdateTime = Connection->Driver->Time;

	// Reset temporary net info.
	Actor->bNetOwner  = 0;
	Actor->RemoteRole = ActualRemoteRole;

	Mark.Pop();
}

//
// Describe the actor channel.
//
FString UActorChannel::Describe()
{
	if( Closing || !Actor )
		return FString(TEXT("Actor=None ")) + UChannel::Describe();
	else
		return FString::Printf(TEXT("Actor=%s (Role=%i RemoteRole=%i) "), Actor->GetFullName(), Actor->Role, Actor->RemoteRole) + UChannel::Describe();
}

IMPLEMENT_CLASS(UActorChannel);

/*-----------------------------------------------------------------------------
	UFileChannel implementation.
-----------------------------------------------------------------------------*/

UFileChannel::UFileChannel()
{}
void UFileChannel::Init( UNetConnection* InConnection, INT InChannelIndex, INT InOpenedLocally )
{
	Super::Init( InConnection, InChannelIndex, InOpenedLocally );
	FileAr			= NULL;
	Transfered		= 0;
	PackageIndex	= INDEX_NONE;
}
void UFileChannel::ReceivedBunch( FInBunch& Bunch )
{
	check(!Closing);
	if( OpenedLocally )
	{
		// Receiving a file sent from the other side.
		FPackageInfo& Info = Connection->PackageMap->List( PackageIndex );
		checkSlow(Bunch.GetNumBytes());

		// Receiving spooled file data.
		if( Transfered==0 )
		{
			// Open temporary file initially.
			debugf( NAME_DevNet, TEXT("Receiving package '%s'"), Info.Parent->GetName() );
			GFileManager->MakeDirectory( *GSys->CachePath, 0 );
			appCreateTempFilename( *GSys->CachePath, Filename );
			FileAr = GFileManager->CreateFileWriter( Filename );
		}

		// Receive.
		if( !FileAr )
		{
			// Opening file failed.
			appSprintf( Error, LocalizeError(TEXT("NetOpen")) );
			Close();
		}
		else
		{
			FileAr->Serialize( Bunch.GetData(), Bunch.GetNumBytes() );
			if( FileAr->IsError() )
			{
				// Write failed.
				appSprintf( Error, LocalizeError("NetWrite"), Filename );
				Close();
			}
			else
			{
				// Successful.
				Transfered += Bunch.GetNumBytes();
				TCHAR Msg1[256], Msg2[256];
				appSprintf( Msg1, LocalizeProgress("ReceiveFile"), PrettyName );
				appSprintf( Msg2, LocalizeProgress("ReceiveSize"), Info.FileSize/1024, 100.f*Transfered/Info.FileSize );
				Connection->Driver->Notify->NotifyProgress( Msg1, Msg2, 4.0 );
			}
		}
	}
	else
	{
		// Request to send a file.
		FGuid Guid;
		Bunch << Guid;
		if( !Bunch.IsError() )
		{
			for( INT i=0; i<Connection->PackageMap->List.Num(); i++ )
			{
				FPackageInfo& Info = Connection->PackageMap->List(i);
				if( Info.Guid==Guid && Info.URL!=TEXT("") )
				{
					appStrncpy( Filename, *Info.URL, ARRAY_COUNT(Filename) );
					if( Connection->Driver->Notify->NotifySendingFile( Connection, Guid ) )
					{
						check(Info.Linker);
						FileAr = GFileManager->CreateFileReader( *Info.URL );
						if( FileAr )
						{
							// Accepted! Now initiate file sending.
							debugf( NAME_DevNet, LocalizeProgress("NetSend"), Filename );
							PackageIndex = i;
							return;
						}
					}
				}
			}
		}

		// Illegal request; refuse it by closing the channel.
		debugf( NAME_DevNet, LocalizeError("NetInvalid") );
		FOutBunch Bunch( this, 1 );
		SendBunch( &Bunch, 0 );
	}
}
void UFileChannel::Tick()
{
	UChannel::Tick();
	Connection->TimeSensitive = 1;
	INT Size;
	while( FileAr && !OpenedLocally && IsNetReady(1) && (Size=MaxSendBytes())!=0 )
	{
		// Sending.
		INT Remaining = Connection->PackageMap->List(PackageIndex).FileSize-Transfered;
		FOutBunch Bunch( this, Size>=Remaining );
		Size = Min( Size, Remaining );
		BYTE* Buffer = (BYTE*)appAlloca( Size );
		FileAr->Serialize( Buffer, Size );
		if( FileAr->IsError() )
		{
			//!!
		}
		Transfered += Size;
		Bunch.Serialize( Buffer, Size );
		Bunch.bReliable = 1;
		check(!Bunch.IsError());
		SendBunch( &Bunch, 0 );
		Connection->FlushNet();
		if( Bunch.bClose )
		{
			// Finished.
			delete FileAr;
			FileAr = NULL;
		}
	}
}
void UFileChannel::Destroy()
{
	check(Connection);
	if( RouteDestroy() )
		return;
	check(Connection->Channels[ChIndex]==this);

	// Close the file.
	if( FileAr )
	{
		delete FileAr;
		FileAr = NULL;
	}

	// Notify that the receive succeeded or failed.
	if( OpenedLocally )
	{
		check(Connection->PackageMap->List.IsValidIndex(PackageIndex));
		FPackageInfo& Info = Connection->PackageMap->List( PackageIndex );
		TCHAR Dest[256];
		appSprintf( Dest, TEXT("%s") PATH_SEPARATOR TEXT("%s.uxx"), *GSys->CachePath, Info.Guid.String() );
		if( !*Error && Transfered==0 )
			appSprintf( Error, LocalizeError("NetRefused"), Info.Parent->GetName() );
		if( !*Error && GFileManager->FileSize(Filename)!=Info.FileSize )
			appSprintf( Error, LocalizeError("NetSize") );
		if( !*Error && !GFileManager->Move( Dest, Filename ) )
			appSprintf( Error, LocalizeError("NetMove") );
		if( *Error )
		{
			// Failure.
			Connection->Driver->Notify->NotifyReceivedFile( Connection, PackageIndex, Error );
			if( FileAr )
				GFileManager->Delete( Filename );
		}
		else
		{
			// Success.
			TCHAR Msg[256];
			appSprintf( Msg, TEXT("Received '%s'"), PrettyName );
			Connection->Driver->Notify->NotifyProgress( TEXT("Success"), Msg, 4.0 );
			Connection->Driver->Notify->NotifyReceivedFile( Connection, PackageIndex, Error );
		}
	}
	else if( FileAr )
	{
		//warning: If !OpenedLocally, PackageIndex may be INDEX_NONE if requested invalid file.
		delete FileAr;
		FileAr = NULL;
	}
	Super::Destroy();
}
FString UFileChannel::Describe()
{
	FPackageInfo& Info = Connection->PackageMap->List( PackageIndex );
	return FString::Printf
	(
		TEXT("File='%s', %s=%i/%i "),
		Filename,
		OpenedLocally ? TEXT("Received") : TEXT("Sent"),
		Transfered,
		Info.FileSize
	) + UChannel::Describe();
}
IMPLEMENT_CLASS(UFileChannel)

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
