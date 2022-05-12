/*=============================================================================
	D3DDrv.cpp: Unreal Direct3D driver precompiled header generator.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by independent contractor who wishes to remanin anonymous.
		* Taken over by Tim Sweeney.
=============================================================================*/

// Windows includes.
#pragma warning(disable : 4291)
#pragma warning(disable : 4201)
#include <windows.h>
#include <objbase.h>

// Unreal includes.
#include "Engine.h"
#include "UnRender.h"

// Direct3D includes.
#define USE_DUKE4_DX8

#ifdef USE_DUKE4_DX8
	#define D3D_OVERLOADS 1
	#include "d3d8.h"
#else
	#include "d3d.h"
#endif
/*-----------------------------------------------------------------------------
	End.
-----------------------------------------------------------------------------*/
