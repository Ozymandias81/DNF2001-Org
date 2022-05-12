/*=============================================================================
	UnRandom.cpp: Random number generation code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Description:
	Maintains a set of random numbers from 0.0-1.0 updated per tick.  One 
	set of random numbers is discontinuous and is regenerated every tick,
	and the other set is continuous and is smoothly interpolated every tick.

Notes:
*	Subsystem is designed for single use.

Revision history:
    10-21-96, Tim: Rewritten from the ground up.
=============================================================================*/

#include "..\..\Engine\Src\EnginePrivate.h"

/*------------------------------------------------------------------------------------
	FGlobalRandoms definition
------------------------------------------------------------------------------------*/

/*
class FGlobalRandoms : public FGlobalRandomsBase
{
private:

	// Functions from FGlobalRandomsBase.
	void Init()	{};
	void Exit()	{};
	void __fastcall Tick(FLOAT TimeSeconds);

	// Variables.
	static FLOAT RandomDeltas[N_RANDS]; // Deltas used to update Randoms.
	static DWORD LastTicks;
};
*/
//
// FGlobalRandomsBase statics.
//
FLOAT FGlobalRandoms::RandomBases[N_RANDS];
FLOAT FGlobalRandoms::Randoms[N_RANDS];

//
// FGlobalRandoms statics.
//
FLOAT FGlobalRandoms::RandomDeltas[N_RANDS];
DWORD FGlobalRandoms::LastTicks=0;

/*------------------------------------------------------------------------------------
	FGlobalRandoms Implementation
------------------------------------------------------------------------------------*/

//
// Update random number tables.
//
void __fastcall FGlobalRandoms::Tick( FLOAT TimeSeconds )
{
	DWORD ServerTicks = DWORD (TimeSeconds * 35.f);

	// optimize: This code would benefit greatly from a fast random number generator
	// that generates floating point randoms from 0.0 to 1.0. This could be done with
	// an integer random generator and ieee-specific code like:
	//
	// float temp=1.0, *result;
	// int randombits = (some completely random bit pattern);
	// *(int*)&result = (*(int*)&temp & 0xff800000) | (randombits & 0x007fffff);

	// Regenerate all random bases for temporally discontinuous random numbers.
	for( int i=0; i<N_RANDS; i++ )
		RandomBases[i] = appFrand();
	
	// Update range of random numbers.
	if( ((DWORD)((ServerTicks - LastTicks) * RAND_CYCLE) < N_RANDS) && LastTicks )
	{
		// No wraparound.
		int r = (ServerTicks * RAND_CYCLE) & RAND_MASK;
		int m = (LastTicks   * RAND_CYCLE) & RAND_MASK;
		for( int n=m; n!=r; n = (n+1) & RAND_MASK )
			RandomDeltas[n] = (RandomBases[n] - Randoms[n]) / (N_RANDS/RAND_CYCLE);
		
		// Update all other randoms.
		FLOAT f = (ServerTicks - LastTicks) & RAND_MASK;
		do
		{
			Randoms[r] += RandomDeltas[r] * f;
			r = (r+1) & RAND_MASK;
		}
		while( r != m );
	}
	else
	{
		// Wraparound or first time.
		for( int i=0; i<N_RANDS; i++)
		{
			Randoms     [i] = appFrand();
			RandomDeltas[i] = (RandomBases[i] - Randoms[i]) / (N_RANDS/RAND_CYCLE);
		}
	}

	for( i=0; i<N_RANDS; i++ )
		Randoms[i]=Clamp(Randoms[i],0.f,1.f);

	// Remember tick count.
	LastTicks = ServerTicks;
}

/*------------------------------------------------------------------------------------
	Random subsystem instantiation
------------------------------------------------------------------------------------*/

FGlobalRandoms GRandoms;

/*------------------------------------------------------------------------------------
	The End
------------------------------------------------------------------------------------*/
