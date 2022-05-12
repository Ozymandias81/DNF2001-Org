/*=============================================================================
	SoftDrvPrivate.h: Rendering package private header.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*------------------------------------------------------------------------------------
	Dependencies.
------------------------------------------------------------------------------------*/

#include "..\..\Render\Src\RenderPrivate.h"

/*------------------------------------------------------------------------------------
	Software rendering private definitions.
------------------------------------------------------------------------------------*/

// Maximum supported sizes. 
#define MaximumYScreenSize  1200   
#define MaximumXScreenSize  2048

// Coarse fast square root approximation
#define FASTAPPROX_MAN_BITS 8   /* Number of bits of approximate square root mantissa, <=23 */
extern FLOAT FastSqrtTbl[2<<FASTAPPROX_MAN_BITS];

void SetupFastSqrt();

//
// An 8-byte MMX structure.
//

union FMMX
{
	struct
	{
#if __INTEL_BYTE_ORDER__
		SWORD R, G, B, A;
#else
		SWORD A, B, G, R;
#endif
	};

	struct
	{
#if __INTEL_BYTE_ORDER__
		BYTE R1, G1, B1, A1, R2, G2, B2, A2;
#else
		BYTE A2, B2, G2, R2, A1, B1, G1, R1;
#endif
	};

	struct
	{
#if __INTEL_BYTE_ORDER__
		BYTE SB1, SG1, SR1, SA1, SR2, SG2, SB2, SA2;
#else
		BYTE SA2, SR2, SG2, SB2, SA1, SB1, SG1, SR1;
#endif
	};

	struct
	{
#if __INTEL_BYTE_ORDER__
		INT IL, IH;
#else
		INT IH, IL;
#endif
	};
	struct
	{
#if __INTEL_BYTE_ORDER__
		DWORD DL, DH;
#else
		DWORD DH, DL;
#endif
	};
	struct
	{
#if __INTEL_BYTE_ORDER__
		FLOAT FL, FH;
#else
		DWORD FH, FL;
#endif
	};
	DOUBLE D;
	QWORD  Q;
	SQWORD SQ;
};


// ColorMode device constants.
enum {
		MMX15,
		MMX16,
		MMXReserved,
		MMX32,
		Pentium15,
		Pentium16,
		PentiumReserved,
		Pentium32
};

// RenderMode constants.
enum {
		DRAWNORMAL,
		DRAWTRANSLUCENT,
		DRAWMODULATED,
		DRAWSTIPPLED,
		DRAWMASKED,
		DRAWRESERVED1,
		DRAWRESERVED2
};


//
// A 3D rendering device.
//
class DLL_EXPORT USoftwareRenderDevice : public URenderDevice
{
	DECLARE_CLASS(USoftwareRenderDevice,URenderDevice,CLASS_Config)

	// Temp statistics.
	INT SurfPalBuilds;

	// Preference variables.
	BITFIELD		FastTranslucency;
	BITFIELD		HighResTextureSmooth;
	BITFIELD		LowResTextureSmooth;
	BITFIELD		RAM32Mode;
	BITFIELD		TextureSmooth;
	FLOAT			DetailBias;

	// Variables.
	DWORD			RenderMode;
	DWORD			ColorMode;
	INT				FrameLocksCounter;
	INT				GByteStride;
	FVector			FlashScale, FlashFog;
	FLOAT			PerspError;
	FLOAT			GMasterScale;
	FVector			GFloatScale, GFloatFog, GMaxColor, GFloatRange;
	TArray<BYTE>	HitStack;
	BYTE*			HitData;
	INT*			HitSize;
	INT				HitCount;

	// Constructors.	
	void StaticConstructor();

	// URenderDevice interface.
	UBOOL __fastcall Init( UViewport* InViewport, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen );
	UBOOL __fastcall SetRes( INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen );
	void __fastcall Exit();
	void __fastcall Flush(	UBOOL );
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );
	//void Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* InHitData, INT* InHitSize );
	void __fastcall Lock( FColor FogColor, float FogDensity, INT FogDistance, FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData=NULL, INT* HitSize=NULL );

	void __fastcall Unlock( UBOOL Blit );
	//void DrawComplexSurface( FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet,TArray<FTransSample> lightingInfo );
	void __fastcall DrawComplexSurface( FSceneNode* Frame, FSurfaceInfo& Surface, FSurfaceFacet& Facet);
	void __fastcall DrawGouraudPolygon( FSceneNode* Frame, FTextureInfo& Texture, FTransTexture** Pts, INT NumPts, DWORD PolyFlags, FSpanBuffer* SpanBuffer );
	void __fastcall DrawTile( FSceneNode* Frame, 
					FTextureInfo& Info, 
					FLOAT X, FLOAT Y, 
					FLOAT XL, FLOAT YL, 
					FLOAT U, FLOAT V, 
					FLOAT UL, FLOAT VL, 
					class FSpanBuffer* Span, 
					FLOAT Z, 
					FPlane Color, FPlane Fog, 
					DWORD PolyFlags, 
					DWORD PolyFlagsEx,
					//UBOOL bilinear, 
					FLOAT alpha,
					FLOAT rot,
					FLOAT rotationOffsetX,
					FLOAT rotationOffsetY 
	);
	void __fastcall GetStats( TCHAR* Result );
	void __fastcall Draw2DLine( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FVector P1, FVector P2 );
	void __fastcall Draw2DPoint( FSceneNode* Frame, FPlane Color, DWORD LineFlags, FLOAT X1, FLOAT Y1, FLOAT X2, FLOAT Y2, FLOAT Z );
	void __fastcall PushHit( const BYTE* Data, INT Count );
	void __fastcall PopHit( INT Count, UBOOL bForce );
	void __fastcall ReadPixels( FColor* Pixels, UBOOL BackBuffer = false);
	void __fastcall ClearZ( FSceneNode* Frame ){};

private:
	// USoftwareRenderDevice interface.
	void InitDrawSurf();

	void MMXFlashTriangle( FSceneNode* Frame, FTextureInfo& Texture, DWORD PolyFlags, FSpanBuffer* SpanBuffer );
	void PentiumFlashTriangle( FSceneNode* Frame, FTextureInfo& Texture, DWORD PolyFlags, FSpanBuffer* SpanBuffer );

	void MMXDrawRectangle( FSceneNode* Node, FTextureInfo& Texture,FTransTexture* Pts, DWORD PolyFlags, FSpanBuffer* SpanBuffer );
	void PentiumDrawRectangle( FSceneNode* Node, FTextureInfo& Texture,FTransTexture* Pts, DWORD PolyFlags, FSpanBuffer* SpanBuffer );
	void InitMMXFlashes( FLOAT Brightness, INT ColorBytes, DWORD Caps);
	void InitColorTables( FLOAT Brightness, INT ColorBytes, DWORD Caps);
	void InitDepthFogTable();

	void InnerGouraudMMX32(DWORD PolyFlags, INT MinY, INT MaxY, FSceneNode* Frame,FMipmapBase* Mip,FSpanBuffer* SpanBuffer);
    void InnerGouraudMMX16(DWORD PolyFlags, INT MinY, INT MaxY, FSceneNode* Frame,FMipmapBase* Mip,FSpanBuffer* SpanBuffer);
	void InnerGouraudMMX15(DWORD PolyFlags, INT MinY, INT MaxY, FSceneNode* Frame,FMipmapBase* Mip,FSpanBuffer* SpanBuffer);

	void BlitTile32(FSpanBuffer* Span);
	void BlitMask32(FSpanBuffer* Span);
	void BlitAlpha32(FSpanBuffer* Span);

	void BlitTile1516(FSpanBuffer* Span);
	void BlitMask1516(FSpanBuffer* Span);
	void BlitAlpha15(FSpanBuffer* Span);
	void BlitAlpha16(FSpanBuffer* Span);

	void ClearScreenFast16(_WORD* Dest, DWORD Color);
	void ClearScreenFast32(DWORD* Dest, DWORD Color);

	void RenderSurfSpansMMX32( FRainbowPtr& DestPtr, FSurfaceInfo& Surface,INT TaskStartY, INT TaskEndY);
	void RenderSurfSpansMMX15( FRainbowPtr& DestPtr, FSurfaceInfo& Surface,INT TaskStartY, INT TaskEndY);
	void RenderSurfSpansMMX16( FRainbowPtr& DestPtr, FSurfaceInfo& Surface,INT TaskStartY, INT TaskEndY);
};




#if ASM

//
// Fast floating point power routines.
// Made coarse but fast for per-span subdivision requirement determination....
// About 12 cycles on the Pentium.
//

inline FLOAT FastSqrt(FLOAT F)
{
	__asm
	{
		mov  eax,[F]                        // get float as int.
		shr  eax,(23 - FASTAPPROX_MAN_BITS) - 2 // shift away unused low mantissa.
		mov  ebx,[F]						// get float as int.
		and  eax, ((1 << (FASTAPPROX_MAN_BITS+1) )-1) << 2 // 2 to avoid "[eax*4]".
		and  ebx, 0x7F000000				// 7 bit exp., wipe low bit+sign.
		shr  ebx, 1							// exponent/2.
		mov  eax,DWORD PTR FastSqrtTbl [eax]	// index hi bit is exp. low bit.
		add  eax,ebx						// recombine with exponent.
		mov  [F],eax						// store.
	}
	return F;								// compiles to fld [F].
}

#else

inline FLOAT FastSqrt(FLOAT F) {return appSqrt(F);}

#endif


/*------------------------------------------------------------------------------------
	The End.
------------------------------------------------------------------------------------*/
