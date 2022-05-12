/*=============================================================================
	UnCache.cpp: FMemCache implementation.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Notes:
	You can't apply more than 126 locks to an object. If you do, the lock count
	overflows and you'll get a "not locked" error when unlocking.

Revision history:
	* Rewritten by Tim Sweeney (speed, speed, speed!)
=============================================================================*/

#include "..\..\Engine\Src\EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Init & Exit.
-----------------------------------------------------------------------------*/

//
// Init the memory cache.
//
void FMemCache::Init
(
	INT		BytesToAllocate,	// Number of bytes for the cache.
	INT		MaxItems,			// Maximum cache items to track.
	void*	Start,				// Start of preallocated cache memory, NULL=allocate it.
	INT		SegmentSize			// Size of segment boundary, or 0=unsegmented.
)
{
	check( Initialized==0 );

	// Remember totals.
	MemTotal   = BytesToAllocate;
	ItemsTotal = MaxItems;
	MruId      = 0;
	MruItem    = NULL;

	// Allocate memory.
	CacheMemory		 = Start ? (BYTE*)Start : new(TEXT("CacheMemory"))BYTE[BytesToAllocate];
	ItemMemory       = new(TEXT("CacheItems"))FCacheItem[MaxItems];

	// Build linked list of items not associated with cache memory.
	FCacheItem** PrevLink = &UnusedItems;
	{for( INT i=0; i<MaxItems; i++ )
	{
		*PrevLink = &ItemMemory[i];
		PrevLink  = &ItemMemory[i].LinearNext;
	}}
	*PrevLink = NULL;

	// Create one or more segments of free space in the cache memory.
	FCacheItem* Prev    = NULL;
	INT         Segment = 0;
	if( SegmentSize==0 )
	{
		FCacheItem* ThisItem = UnusedItems;
		CreateNewFreeSpace
		(
			CacheMemory,
			CacheMemory + BytesToAllocate,
			NULL,
			NULL,
			Segment
		);
		Prev = ThisItem;
		Segment++;
	}
	else
	{
		for( Segment=0; Segment*SegmentSize<BytesToAllocate; Segment++ )
		{
			FCacheItem* ThisItem = UnusedItems;
			CreateNewFreeSpace
			(
				CacheMemory + Segment * SegmentSize,
				CacheMemory + Segment * SegmentSize + Min(SegmentSize,BytesToAllocate - Segment * SegmentSize),
				Prev,
				NULL,
				Segment
			);
			Prev = ThisItem;
		}
	}

	// Create last, empty item.
	LastItem = UnusedItems;
	CreateNewFreeSpace
	(
		CacheMemory + BytesToAllocate,
		CacheMemory + BytesToAllocate,
		Prev,
		NULL,
		Segment
	);

	// Init the hash table to empty since no items are used.
	{for( INT i=0; i<HASH_COUNT; i++ )
		HashItems[i] = NULL;}

	// Success.
	Initialized=1;
	CheckState();
}

//
// Shut down the memory cache.
//
void FMemCache::Exit( UBOOL FreeMemory )
{
	if( Initialized )
	{
		CheckState();

		// Release all memory.
		delete ItemMemory;
		if( FreeMemory )
			appFree( CacheMemory );

		// Success.
		Initialized = 0;
	}
}

/*-----------------------------------------------------------------------------
	Internal functions.
-----------------------------------------------------------------------------*/

//
// Merge a cache item and its immediate successor into one
// item, and remove the second. Returns the new merged item.
//
FMemCache::FCacheItem* FMemCache::MergeWithNext( FCacheItem* First )
{
	checkSlow( First );

	// Get second item.
	FCacheItem* Second = First->LinearNext;

	// Validate everything.
	checkSlow( Second );
	checkSlow( Second->LinearNext );
	checkSlow( Second != NULL );
	checkSlow( First->LinearNext == Second );	
	checkSlow( Second->LinearPrev == First );
	checkSlow( First->Segment == Second->Segment );

	// Absorb the second item into the first.
	First->LinearNext             = Second->LinearNext;
	First->LinearNext->LinearPrev = First;

	// Stick the second item at the head of the unused list.
	Second->LinearNext            = UnusedItems;
	UnusedItems                   = Second;

	// Success.
	return First;
}

//
// Flush a cache item and return the one immediately
// following it in memory, or NULL if at end.
//
FMemCache::FCacheItem* FMemCache::FlushItem( FCacheItem* Item, UBOOL IgnoreLocked )
{
	checkSlow( Item != NULL );
	checkSlow( Item->Id != 0 );
	if( Item->Cost < COST_INFINITE ) 
	{
		// Flush this one item.
		Item->Id	= 0;
		Item->Cost	= 0;

		// If previous item is free space, merge with it.
		if( Item->LinearPrev && Item->LinearPrev->Id==0 && Item->Segment==Item->LinearPrev->Segment )
			Item = MergeWithNext( Item->LinearPrev );

		// If next item is free space, merge with it.
		if( Item->LinearNext && Item->LinearNext->Id==0 && Item->Segment==Item->LinearNext->Segment )
			Item = MergeWithNext( Item );
	}
	else if( !IgnoreLocked )
	{
		// Trying to flush a locked item.
		appErrorf( TEXT("Flushed locked cache object %08X.%08X"), (DWORD)(Item->Id>>32), (DWORD)Item->Id );
	}
	return Item->LinearNext;
}

//
// Make sure the state is valid.
//
void FMemCache::CheckState()
{

	// Make sure we're initialized.
	check( Initialized == 1 );

	// Make sure there's an initial item.
	check( CacheItems != NULL );

	// Init stats.
	INT ItemCount=0, UsedItemCount=0, WasFree=0, HashCount=0, MemoryCount=0, PrevSegment=-1;
	BYTE* ExpectedPointer = CacheMemory;

	// Traverse all cache items.
	for( FCacheItem* Item=CacheItems; Item!=LastItem; Item=Item->LinearNext )
	{
		// Make sure this memory is where we expect.
		check( Item->Data == ExpectedPointer );
		check( Item->LinearNext );
		check( Item->LinearNext->LinearPrev == Item );

		// Count memory.
		INT Size         = Item->LinearNext->Data - Item->Data;
		MemoryCount     += Size;
		ExpectedPointer += Size;

		// Count items.
		ItemCount++;

		// Make sure that free items aren't contiguous.
		if( Item->Id==0 && Item->Segment==PrevSegment )
			check( !WasFree );

		WasFree     = (Item->Id == 0);
		PrevSegment = Item->Segment;

		// Verify previous link.
		if( Item != CacheItems )
		{
			check( Item->LinearPrev );
			check( Item->LinearPrev->LinearNext == Item );
		}

		// If used, make sure this item is hashed exactly once.
		if( Item->Id )
		{
			UsedItemCount++;
			INT HashedCount=0;
			for( FCacheItem* HashItem=HashItems[GHash(Item->Id)]; HashItem; HashItem=HashItem->HashNext )
				HashedCount += (HashItem == Item);
			check(HashedCount!=0);
			check(HashedCount==1);
		}
	}
	check( ExpectedPointer == CacheMemory + MemTotal );
	
	{
		// Traverse all unused items.
		for( FCacheItem* Item=UnusedItems; Item; Item=Item->LinearNext )
			ItemCount++;
	}

	// Make sure all items are accounted for.
	check( ItemCount+1==ItemsTotal );

	// Make sure all hashed items are used, and there are no duplicate Id's.
	for( DWORD i=0; i<HASH_COUNT; i++ )
	{
		for( FCacheItem* Item=HashItems[i]; Item; Item=Item->HashNext )
		{
			// Count this hash item.
			HashCount++;

			// Make sure this Id belongs here.
			check( GHash(Item->Id) == i );

			// Make sure this Id is not duplicated.
			for( FCacheItem* Other=Item->HashNext; Other; Other=Other->HashNext )
				check( Other->Id != Item->Id );
		}
	}
	check( HashCount == UsedItemCount );

	// Success.
}

//
// Create a block of free space in the cache, and link it in.
// If either of Next or Prev are free space, simply merges
// and does not create a new item.
//
void FMemCache::CreateNewFreeSpace
(
	BYTE*		Start, 
	BYTE*		End, 
	FCacheItem*	Prev, 
	FCacheItem*	Next,
	INT			Segment
)
{

	// Make sure parameters are valid.
	checkSlow( Start >= CacheMemory );
	checkSlow( End <= (CacheMemory + MemTotal) );
	checkSlow( Start <= End );

	if( Prev && Prev->Id==0 && Prev->Segment==Segment )
	{
		// The previous item is free space, so automatically merge with it.
	}
	else if( Next && Next->Id==0 && Next->Segment==Segment )
	{
		// The next item is free space, so merge with it.
		Next->Data = Start;
	}
	else
	{
		// Make sure we can grab a new item.
		check( UnusedItems != NULL );

		// Grab a free space item from the list.
		FCacheItem* Item = UnusedItems;
		UnusedItems      = UnusedItems->LinearNext;

		// Create the free space item.
		Item->Data			= Start;
		Item->Segment		= Segment;
		Item->Time			= 0;
		Item->Id			= 0;
		Item->Cost			= 0;
		Item->LinearNext	= Next;
		Item->LinearPrev	= Prev;
		Item->HashNext		= NULL;

		// Link it in.
		if( Prev )
			Prev->LinearNext = Item;
		else
			CacheItems = Item;

		if( Next )
			Next->LinearPrev = Item;
	}
}

/*-----------------------------------------------------------------------------
	Flushing.
-----------------------------------------------------------------------------*/

//
// Flush the memory cache.
// If Mask=~0, flush exactly Id.
// Otherwise flush all Ids for which (TestId&Mask)==Id.
//
void FMemCache::Flush( QWORD Id, DWORD Mask, UBOOL IgnoreLocked )
{
	MruId     = 0;
	MruItem   = NULL;
	if( !Initialized )
		return;

	// Special case for flushing all items.
	if( Id == 0 )
		Mask = 0;
	if( Mask == ~0 )
	{
		// Quickly flush a single element.
		FCacheItem** PrevLink = &HashItems[GHash(Id)];
		while( *PrevLink != NULL )
		{
			FCacheItem *Item = *PrevLink;			
			if( Item->Id == Id )
			{
				// Remove item from hash.
				*PrevLink = Item->HashNext;

				// Flush the item.
				FlushItem( Item, IgnoreLocked );
				
				// Successfully flushed.
				break;
			}
			PrevLink = &Item->HashNext;
		}
	}
	else
	{
		// Slow wildcard flush of all in-memory cache items.
		FCacheItem* Item=CacheItems;
		while( Item )
		{
			if( Item->Id!=0 && (Item->Id & Mask)==(Id & Mask) )
			{
				// Remove item from hash table.
				if( Item->Cost < COST_INFINITE ) 
					Unhash( Item->Id );

				// Flush the item and get the next item in linear sequence.
				Item = FlushItem( Item, IgnoreLocked );
			}
			else Item = Item->LinearNext;
		}
		if( Mask==0 && !IgnoreLocked )
		{
			// Make sure we flushed the entire cache and there is only
			// one cache item remaining for each segment.
			check( CacheItems!=NULL );
			INT ExpectSegment=0;
			for( FCacheItem* TestItem=CacheItems; TestItem!=LastItem; TestItem=TestItem->LinearNext )
			{
				check( TestItem->Id==0 );
				check( TestItem->Segment==ExpectSegment++ );
			}
		}
	}
	ConditionalCheckState();
}

/*-----------------------------------------------------------------------------
	Creating.
-----------------------------------------------------------------------------*/

//
// Create an element in the cache.
//
// This is O(num_items_in_cache), sacrificing some speed in the
// name of better cache efficiency. However there aren't any really
// good algorithms for priority queues where most priorities change
// every iteration this that I'm aware of.
//
BYTE* FMemCache::Create
(
	QWORD			Id, 
	FCacheItem*&	Item, 
	INT				CreateSize, 
	INT				Alignment,
	INT				SafetyPad
)
{
	clock(CreateCycles);
	check(Initialized);
	
	if(CreateSize<=0) appErrorf(TEXT("CreateSize<=0 (%i)"),CreateSize);
	check(CreateSize>0);
	check(Id != 0);
	NumCreates++;

	// Best cost and starting element found thus far.
	SQWORD	    BestCost  = COST_INFINITE;
	FCacheItem* BestFirst = NULL;
	FCacheItem* BestLast  = NULL;

	// Iterate through items. Find shortest contiguous sets of items
	// which contain enough space for this entry. Evaluate the sum cost
	// for each set, remembering the best cost.
	SQWORD Cost=0;
	FCacheItem* First=CacheItems;
	{for( FCacheItem* Last=CacheItems; Last!=LastItem; Last=Last->LinearNext )
	{
		// Add the cost and size of new Last element to our accumulator.
		Cost += Last->Cost;

		// While the interval from First to Last (inclusive) contains
		// enough space for the item we're creating, consider it as a
		// candidate, and go to the next First.
		while( First && (Last->LinearNext->Data - Align(First->Data,Alignment) >= (CreateSize+SafetyPad)) )
		{
			// Is this the best solution so far?
			if( Cost<BestCost && First->Segment==Last->Segment )
			{
				BestCost  = Cost;
				BestFirst = First;
				BestLast  = Last;
			}

			// Subtract the cost and size from the element we're passing:
			Cost -= First->Cost;
			checkSlow(Cost>=0);

			// Go to next First.
			First = First->LinearNext;
		}
	}}

	// See if we found a suitable place to put the item.
	if( BestFirst == NULL )
	{
		// Critical error: the item can't fit in the cache.
		INT ItemsLocked=0, Bytes=0, BytesLocked=0;
		for( FCacheItem* Last=CacheItems; Last!=LastItem; Last=Last->LinearNext )
		{
			INT Size = (Last->LinearNext->Data - Last->Data);
			Bytes += Size;
			if( Last->Cost >= COST_INFINITE )
			{
				ItemsLocked++;
				BytesLocked += Size;
			}
		}
		Exec( TEXT("DUMPCACHE") );
		appErrorf( TEXT("Create %08x.%08X failed: Size=%i Pad=%i Align=%i NumLocked=%i BytesLocked=%i/%i"), (DWORD)(Id>>32), (DWORD)Id, CreateSize, SafetyPad, Alignment, ItemsLocked, BytesLocked, Bytes );
	}

	// Merge all items from Start to End into one bigger item,
	// while unhashing them all.
	while( BestLast != BestFirst )
	{
		if( BestLast->Id != 0 ) Unhash( BestLast->Id );
		BestLast = MergeWithNext( BestLast->LinearPrev );
	}
	if( BestFirst->Id != 0 ) Unhash( BestFirst->Id );

	// Now we have a big free memory block from BestFirst->Data to 
	// BestFirst->Data + BestFirst->Size.
	BYTE* Result = Align( BestFirst->Data, Alignment );
	checkSlow( Result + CreateSize <= BestFirst->LinearNext->Data );
	checkSlow( ((INT)Result & (Alignment-1)) == 0 );

	// Claim BestFirst for the block we're creating, and lock it.
	BestFirst->Time = (FCacheItem::TCacheTime)Time;
	BestFirst->Id   = Id;
	BestFirst->Cost = CreateSize + COST_INFINITE;

	// Hash it.
	FCacheItem** HashPtr	= &HashItems[GHash(Id)];
	BestFirst->HashNext		= *HashPtr;
	*HashPtr				= BestFirst;

	// Create free space past the end of the newly allocated block.
	if( UnusedItems && (Result + CreateSize < BestFirst->LinearNext->Data ) )
	{
		CreateNewFreeSpace
		(
			Result + CreateSize, 
			BestFirst->LinearNext->Data,
			BestFirst,
			BestFirst->LinearNext,
			BestFirst->Segment
		);
	}

	// Create free space before the beginning of the newly allocated one.
	if( UnusedItems && (Result - BestFirst->Data) >= IGNORE_SIZE )
	{
		// Not currently used, so we trap this as an error.
		appErrorf(TEXT("Bizarre cache alignment"));
		CreateNewFreeSpace
		(
			BestFirst->Data, 
			Result,
			BestFirst->LinearPrev,
			BestFirst,
			BestFirst->Segment
		);
		BestFirst->Data = Result;
	}

	// Set the resulting Item.
	MruItem = Item = BestFirst;
	MruId          = Id;

	ConditionalCheckState();
	unclock(CreateCycles);

	return Result;
}

/*-----------------------------------------------------------------------------
	Tick.
-----------------------------------------------------------------------------*/

//
// Handle time passing.
//
void FMemCache::Tick()
{
	clock(TickCycles);
	ConditionalCheckState();
	MruId     = 0;
	MruItem   = NULL;

	// Init memory stats.
	MemFresh = MemStale = 0;
	ItemsFresh = ItemsStale = ItemGaps = 0;

	// Check each item.
#if DO_GUARD_SLOW
	for( FCacheItem* Item=CacheItems; Item!=LastItem; Item=Item->LinearNext )
	{
		if( Item->Id == 0 )
		{
			ItemGaps++;
		}
		else if( Item->Cost >= COST_INFINITE )
		{
			appErrorf( TEXT("Cache item %08X still locked in call to Tick"), Item->Id );
		}
		else if( Time - Item->Time > 1 )
		{
			// Exponentially decrease Cost for stale items.
			Item->Cost -= Item->Cost >> 5;
			MemStale   += Item->LinearNext->Data - Item->Data;
			ItemsStale++;
		}
		else if( Time - Item->Time == 1 )
		{
			// Cut Cost by 1/4th as soon as it first becomes stale.
			Item->Cost = Item->Cost >> 2;
		}
		else
		{
			MemFresh += Item->LinearNext->Data - Item->Data;
			ItemsFresh++;
		}
	}
#else
	for( FCacheItem* Item=CacheItems; Item!=LastItem; Item=Item->LinearNext )
		if( Item->Id!=0 && Time>Item->Time )
			Item->Cost -= (Item->Cost >> 5);
#endif

	// Update the cache's time.
	Time++;
	unclock(TickCycles);
	//debugf(TEXT("Create=%06.3f msec Check=%06.3f"),(CreateCycles*GSecondsPerCycle*1000.0),(TickCycles*GSecondsPerCycle*1000.0));
	//TickCycles=CreateCycles=0;
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

UBOOL FMemCache::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if( ParseCommand(&Cmd,TEXT("DUMPCACHE")) )
	{
		for( FCacheItem* Item=CacheItems; Item!=LastItem; Item=Item->LinearNext )
		{
			const TCHAR* Descr=NULL;
			if( Item->Cost >= COST_INFINITE )
				Descr=TEXT("Locked");
			else if( Item->Id == 0 )
				Descr=TEXT("Empty");
			else if( Time - Item->Time >= 1)
				Descr=TEXT("Stale");
			else
				Descr=TEXT("Fresh");
			Ar.Logf( TEXT("%02X [%i]: %s"), (BYTE)Item->Id, Item->LinearNext->Data - Item->Data, Descr );
		}
		return 1;
	}
	else return 0;
}

/*-----------------------------------------------------------------------------
	Status.
-----------------------------------------------------------------------------*/

//
// Write useful human readable performance stats into a string.
//
void FMemCache::Status( TCHAR *StatusText )
{
	// Display stats.
	appSprintf
	(
		StatusText, 
		TEXT("Gets=%04i (%04.1f) Crts=%03i (%04.1f) Fresh=%03iK Stale=%03iK Items=%03i Tick=%04.1f"),
		NumGets,
		GetCycles * GSecondsPerCycle*1000,
		NumCreates,
		CreateCycles * GSecondsPerCycle*1000,
		MemFresh/1024,
		MemStale/1024,
		ItemsFresh+ItemsStale+ItemGaps,
		TickCycles * GSecondsPerCycle*1000
	);

	// Reinitialize time-variant stats.
	NumGets = GetCycles = NumCreates = CreateCycles = TickCycles = 0;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
