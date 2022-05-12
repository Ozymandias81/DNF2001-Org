/*=============================================================================
	UnTex.h: Unreal texture related classes.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Constants.
-----------------------------------------------------------------------------*/

enum {NUM_PAL_COLORS=256};	// Number of colors in a standard palette.

// Constants.
enum
{
	EHiColor565_R = 0xf800,
	EHiColor565_G = 0x07e0,
	EHiColor565_B = 0x001f,

	EHiColor555_R = 0x7c00,
	EHiColor555_G = 0x03e0,
	EHiColor555_B = 0x001f,

	ETrueColor_R  = 0x00ff0000,
	ETrueColor_G  = 0x0000ff00,
	ETrueColor_B  = 0x000000ff,
};

/*-----------------------------------------------------------------------------
	UPalette.
-----------------------------------------------------------------------------*/

//
// A truecolor value.
//
#define GET_COLOR_DWORD(color) (*(DWORD*)&(color))
class ENGINE_API FColor
{
public:
	union
	{
	// Variables.
#if __INTEL_BYTE_ORDER__
		struct { BYTE R,G,B,A; };
#else
		struct { BYTE A,B,G,R; };
#endif
		DWORD FlatColor;
	};

	// Constructors.
	FColor() {}
	FColor( BYTE InR, BYTE InG, BYTE InB )
	:	R(InR), G(InG), B(InB) {}
	FColor( BYTE InR, BYTE InG, BYTE InB, BYTE InA )
	:	R(InR), G(InG), B(InB), A(InA) {}
	FColor( const FPlane& P )
	:	R(Clamp(appFloor(P.X*256),0,255))
	,	G(Clamp(appFloor(P.Y*256),0,255))
	,	B(Clamp(appFloor(P.Z*256),0,255))
	,	A(Clamp(appFloor(P.W*256),0,255))
	{}

	// Serializer.
	friend FArchive& operator<< (FArchive &Ar, FColor &Color )
	{
		return Ar << Color.R << Color.G << Color.B << Color.A;
	}

	FColor &operator=(DWORD c)
	{
		GET_COLOR_DWORD(*this)=c;
		return *this;
	}

	// Operators.
	UBOOL operator==( const FColor &C ) const
	{
		//return D==C.D;
		return *(DWORD*)this == GET_COLOR_DWORD(C);
	}
	UBOOL operator!=( const FColor& C ) const
	{
		return GET_COLOR_DWORD(*this)!=GET_COLOR_DWORD(C);
	}
	INT Brightness() const
	{
		return (2*(INT)R + 3*(INT)G + 1*(INT)B)>>3;
	}
	FLOAT FBrightness() const
	{
		return (2.0*R + 3.0*G + 1.0*B)/(6.0*256.0);
	}
	DWORD TrueColor() const
	{
		DWORD D=*(DWORD*)this;
		return ((D&0xff)<<16) + (D&0xff00) + ((D&0xff0000)>>16);
	}
	_WORD HiColor565() const
	{
		DWORD D=GET_COLOR_DWORD(*this);
		return ((D&0xf8) << 8) + ((D&0xfC00) >> 5) + ((D&0xf80000) >> 19);
	}
	_WORD HiColor555() const
	{
		DWORD D=GET_COLOR_DWORD(*this);
		return ((D&0xf8) << 7) + ((D&0xf800) >> 6) + ((D&0xf80000) >> 19);
	}
	FVector Plane() const
	{
		return FPlane(R/255.f,G/255.f,B/255.f,A/255.0);
	}
	FColor Brighten( INT Amount )
	{
		return FColor( Plane() * (1.0 - Amount/24.0) );
	}
};
extern ENGINE_API FPlane FGetHSV( BYTE H, BYTE S, BYTE V );

//
// A palette object.  Holds NUM_PAL_COLORS unique FColor values, 
// forming a 256-color palette which can be referenced by textures.
//
class ENGINE_API UPalette : public UObject
{
	DECLARE_CLASS(UPalette,UObject,CLASS_SafeReplace)

	// Variables.
	TArray<FColor> Colors;

	// Constructors.
	UPalette();

	// UObject interface.
	void Serialize( FArchive& Ar );

	// UPalette interface.
	BYTE BestMatch( FColor Color, INT First );
	UPalette* ReplaceWithExisting();
	void FixPalette();
};

/*-----------------------------------------------------------------------------
	UTexture and FTextureInfo.
-----------------------------------------------------------------------------*/

// Texture level-of-detail sets.
enum ELODSet
{
	LODSET_None  = 0,  // No level of detail mipmap tossing.
	LODSET_World = 1,  // World level-of-detail set.
	LODSET_Skin  = 2,  // Skin level-of-detail set.
	LODSET_MAX   = 8,  // Maximum.
};

enum {MAX_TEXTURE_LOD=4};

//
// Base mipmap.
//
struct ENGINE_API FMipmapBase
{
public:
	BYTE*			DataPtr;		// Pointer to data, valid only when locked.
	INT				USize,  VSize;	// Power of two tile dimensions.
	BYTE			UBits,  VBits;	// Power of two tile bits.
	FMipmapBase( BYTE InUBits, BYTE InVBits )
	:	DataPtr		(0)
	,	USize		(1<<InUBits)
	,	VSize		(1<<InVBits)
	,	UBits		(InUBits)
	,	VBits		(InVBits)
	{}
	FMipmapBase()
	{}
};

//
// Texture mipmap.
//
struct ENGINE_API FMipmap : public FMipmapBase
{
public:
	TLazyArray<BYTE> DataArray; // Data.
	FMipmap()
	{}
	FMipmap( BYTE InUBits, BYTE InVBits )
	:	FMipmapBase( InUBits, InVBits )
	,	DataArray( USize * VSize )
	{}
	FMipmap( BYTE InUBits, BYTE InVBits, INT InSize )
	:	FMipmapBase( InUBits, InVBits )
	,	DataArray( InSize )
	{}
	void Clear()
	{
		appMemzero( &DataArray(0), DataArray.Num() );
	}
	friend FArchive& operator<<( FArchive& Ar, FMipmap& M )
	{
		Ar << M.DataArray;
		Ar << M.USize << M.VSize << M.UBits << M.VBits;
		return Ar;
	}
};

//
// Texture clearing flags.
//
enum ETextureClear
{
	TCLEAR_Temporal	= 1,	// Clear temporal texture effects.
	TCLEAR_Bitmap   = 2,    // Clear the immediate bitmap.
};

//
// Texture formats.
//
enum ETextureFormat
{
	// Base types.
	TEXF_P8			= 0x00,
	TEXF_RGBA7		= 0x01,
	TEXF_RGB16		= 0x02,
	TEXF_DXT1       = 0x03,
	TEXF_RGB8       = 0x04,
	TEXF_RGBA8      = 0x05,
	TEXF_NODATA		= 0x06,
	TEXF_DXT3		= 0x07,
	TEXF_DXT5		= 0x08,
	TEXF_G8			= 0x09,
	TEXF_G16		= 0x0a,
	TEXF_MAX		= 0xff,
};

//
// A low-level bitmap.
//
class ENGINE_API UBitmap : public UObject
{
	DECLARE_ABSTRACT_CLASS(UBitmap,UObject,0)

	// General bitmap information.
	BYTE		Format;				// ETextureFormat.
	UPalette*	Palette;			// Palette if 8-bit palettized.
	BYTE		UBits, VBits;		// # of bits in USize, i.e. 8 for 256.
	INT			USize, VSize;		// Size, must be power of 2.
	INT			UClamp, VClamp;		// Clamped width, must be <= size.
	FColor		MipZero;			// Overall average color of texture.
	FColor		MaxColor;			// Maximum color for normalization.
	DOUBLE		LastUpdateTime;		// Last time texture was locked for rendering.

	// Static.
	static class UClient* __Client;

	// Constructor.
	UBitmap();

	// UBitmap interface.
	virtual void __fastcall Lock( FTextureInfo& TextureInfo, DOUBLE Time, INT LOD, URenderDevice* RenDev )=0;
	virtual void __fastcall Unlock( FTextureInfo& TextureInfo )=0;
	virtual FMipmapBase* GetMip( INT i )=0;
};

//
// A complex material texture.
//
class ENGINE_API UTexture : public UBitmap
{
	DECLARE_CLASS(UTexture,UBitmap,CLASS_SafeReplace)

	// Subtextures.
	UTexture*	BumpMap;			// Bump map to illuminate this texture with.
	UTexture*	DetailTexture;		// Detail texture to apply.
	UTexture*	MacroTexture;		// Macrotexture to apply, not currently used.

	// Surface properties.
	FLOAT		Diffuse;			// Diffuse lighting coefficient (0.0-1.0).
	FLOAT		Specular;			// Specular lighting coefficient (0.0-1.0).
	FLOAT		Alpha;				// Reflectivity (0.0-0.1).
	FLOAT       Scale;              // Scaling relative to parent, 1.0=normal.
	FLOAT		Friction;			// Surface friction coefficient, 1.0=none, 0.95=some.
	FLOAT		MipMult;			// Mipmap multiplier.

	// Sounds.
	class UClass* Material;			// Material used by this texture.
	FName		  MaterialName;		// Name of material to use.

	//USound*		HitSound;			// Sound when the texture is hit with a projectile.
	UTexture*	  ChangeTextureOnHit;

	// Flags.
	DWORD		PolyFlags;			// Polygon flags to be applied to Bsp polys with texture (See PF_*).
	BITFIELD	bHighColorQuality:1 GCC_PACK(4); // High color quality hint.
	BITFIELD	bHighTextureQuality:1; // High color quality hint.
	BITFIELD	bRealtime:1;        // Texture changes in realtime.
	BITFIELD	bParametric:1;      // Texture data need not be stored.
	BITFIELD	bRealtimeChanged:1; // Changed since last render.
	BITFIELD    bHasComp:1;         // Compressed version included?

	BYTE        LODSet GCC_PACK(4); // Level of detail type.

	// Animation related.
	UTexture*	AnimNext;			// Next texture in looped animation sequence.
	UTexture*	AnimCur;			// Current animation frame.
	BYTE		PrimeCount;			// Priming total for algorithmic textures.
	BYTE		PrimeCurrent;		// Priming current for algorithmic textures.
	FLOAT		MinFrameRate;		// Minimum animation rate in fps.
	FLOAT		MaxFrameRate;		// Maximum animation rate in fps.
	FLOAT		Accumulator;		// Frame accumulator.

	// Table of mipmaps.
	TArray<FMipmap> Mips;			// Mipmaps in native format.
	TArray<FMipmap> CompMips;		// Mipmaps in requested format.
	BYTE            CompFormat;     // Decompressed texture format.
	DWORD			PolyFlagsEx;		// NJS: Extended poly flags

	// Constructor.
	UTexture();

	// UObject interface.
	void Serialize( FArchive& Ar );
	const TCHAR* Import( const TCHAR* Buffer, const TCHAR* BufferEnd, const TCHAR* FileType );
	void Export( FArchive& Ar, const TCHAR* FileType, INT Indent );
	void PostLoad();
	void Destroy();

	// UBitmap interface.
	DWORD GetColorsIndex()
	{
		return Palette->GetIndex();
	}
	FColor* GetColors()
	{
		return Palette ? &Palette->Colors(0) : NULL;
	}
	INT GetNumMips()
	{
		return Mips.Num();
	}
	FMipmapBase* GetMip( INT i )
	{
		return &Mips(i);
	}
	void __fastcall Lock( FTextureInfo& TextureInfo, DOUBLE Time, INT LOD, URenderDevice* RenDev );
	void __fastcall Unlock( FTextureInfo& TextureInfo );

	// UTexture interface.
	virtual void Clear( DWORD ClearFlags );
	virtual void Init( INT InUSize, INT InVSize );
	virtual void __fastcall Tick( FLOAT DeltaSeconds );
	virtual void ConstantTimeTick();
	virtual void MousePosition( DWORD Buttons, FLOAT X, FLOAT Y ) {}
	virtual void Click( DWORD Buttons, FLOAT X, FLOAT Y ) {}
	virtual void Update( DOUBLE Time );

	// UTexture functions.
	void BuildRemapIndex( UBOOL Masked );
	void CreateMips( UBOOL FullMips, UBOOL Downsample );
	void CreateColorRange();
	UBOOL Compress( ETextureFormat Format, UBOOL Mipmap );
	UBOOL Decompress( ETextureFormat Format );
	INT DefaultLOD();

	// UTexture accessors.
	UTexture* Get( DOUBLE Time )
	{
		Update( Time );
		return AnimCur ? AnimCur : this;
	}

	// NJS: 2d pixel collision checking:
	UBOOL CollisionCheck( int x, int y, UTexture *Other, int oX, int oY );
	BYTE  GetPixel( int x, int y );

	DECLARE_FUNCTION(execCollisionCheck);
	DECLARE_FUNCTION(execGetPixel);
};

//
// Information about a locked texture. Used for ease of rendering.
//
enum {MAX_MIPS=12};
struct ENGINE_API FTextureInfo
{
	friend class UBitmap;
	friend class UTexture;

	// Variables.
	UTexture*		Texture;				// Optional texture.
	QWORD			CacheID;				// Unique cache ID.
	QWORD			PaletteCacheID;			// Unique cache ID of palette.
	FVector			Pan;					// Panning value relative to texture planes.
	FColor*			MaxColor;				// Maximum color in texture and all its mipmaps.
	ETextureFormat	Format;					// Texture format.
	FLOAT			UScale;					// U Scaling.
	FLOAT			VScale;					// V Scaling.
	INT				USize;					// Base U size.
	INT				VSize;					// Base V size.
	INT				UClamp;					// U clamping value, or 0 if none.
	INT				VClamp;					// V clamping value, or 0 if none.
	INT				NumMips;				// Number of mipmaps.
	INT				LOD;					// Level of detail, 0=highest.
	FColor*			Palette;				// Palette colors.
	BITFIELD		bHighColorQuality:1;	// High color quality hint.
	BITFIELD		bHighTextureQuality:1;	// High color quality hint.
	BITFIELD		bRealtime:1;			// Texture changes in realtime.
	BITFIELD		bParametric:1;			// Texture data need not be stored.
	BITFIELD		bRealtimeChanged:1;		// Changed since last render.
	FMipmapBase*	Mips[MAX_MIPS];			// Array of NumMips of mipmaps.

	// Functions.
	void Load();
	void Unload();
	void CacheMaxColor();
};

/*-----------------------------------------------------------------------------
	UFont.
-----------------------------------------------------------------------------*/

// Font constants.
enum {NUM_FONT_PAGES=256};
enum {NUM_FONT_CHARS=256};

//
// Information about one font glyph which resides in a texture.
//
struct ENGINE_API FFontCharacter
{
	// Variables.
	INT StartU, StartV;
	INT USize, VSize;

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FFontCharacter& Ch )
	{
		return Ar << Ch.StartU << Ch.StartV << Ch.USize << Ch.VSize;
	}
};

//
// A font page.
//
struct ENGINE_API FFontPage
{
	// Variables.
	UTexture* Texture;
	TArray<FFontCharacter> Characters;

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FFontPage& Ch )
	{
		return Ar << Ch.Texture << Ch.Characters;
	}
};

//
// A font object, containing information about a set of glyphs.
// The glyph bitmaps are stored in the contained textures, while
// the font database only contains the coordinates of the individual
// glyph.
//
class ENGINE_API UFont : public UObject
{
	DECLARE_CLASS(UFont,UObject,0)

	// Variables.
	INT CharactersPerPage;
	TArray<FFontPage> Pages;

	// Constructors.
	UFont();

	// UObject interface.
	void Serialize( FArchive& Ar );
};

class ENGINE_API UFontTrueType : public UFont
{
	DECLARE_CLASS(UFontTrueType,UFont,CLASS_Transient)

	// Variables.
	TMap<TCHAR, INT> VHeights;

	// Constructors.
	UFontTrueType();

	// UObject interface.
	void Serialize( FArchive& Ar );
};

/*----------------------------------------------------------------------------
	NJS: Abstract ProceduralTexture base class.
	Helps to have it in engine so exporters can be modified, and a little
	structurally cleaner in general anyhow.
----------------------------------------------------------------------------*/

class ENGINE_API UProceduralTexture : public UTexture
{
	DECLARE_ABSTRACT_CLASS(UProceduralTexture,UTexture,0)
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
