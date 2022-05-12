/*=============================================================================
	UnMesh.h: Unreal mesh objects.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
		* Beaten into submission by Chris Hargrove
=============================================================================*/

/*-----------------------------------------------------------------------------
	UUnrealMesh.
-----------------------------------------------------------------------------*/

// An actor notification event associated with an animation sequence.
class FMeshAnimNotify
{
public:	
	FLOAT	Time;			// Time to occur, 0.0-1.0.
	FName	Function;		// Name of the actor function to call.
	friend FArchive &operator<<( FArchive& Ar, FMeshAnimNotify& N )
		{return Ar << N.Time << N.Function;}
	FMeshAnimNotify()
		: Time(0.0), Function(NAME_None) {}
};

// Information about one animation sequence associated with a mesh,
// a group of contiguous frames.
class FMeshAnimSeq
{
public:
	FName					Name;		// Sequence's name.
	FName					Group;		// Group.
	INT						StartFrame;	// Starting frame number.
	INT						NumFrames;	// Number of frames in sequence.
	FLOAT					Rate;		// Playback rate in frames per second.
	TArray<FMeshAnimNotify> Notifys;	// Notifications.
	friend FArchive &operator<<( FArchive& Ar, FMeshAnimSeq& A )
		{return Ar << A.Name << A.Group << A.StartFrame << A.NumFrames << A.Notifys << A.Rate;}
	FMeshAnimSeq()
		: Name(NAME_None), Group(NAME_None), StartFrame(0), NumFrames(0), Rate(30.0), Notifys() {}
};

// Says which triangles a particular mesh vertex is associated with.
// Precomputed so that mesh triangles can be shaded with Gouraud-style
// shared, interpolated normal shading.
struct FMeshVertConnect
{
	INT	NumVertTriangles;
	INT	TriangleListOffset;
	friend FArchive &operator<<( FArchive& Ar, FMeshVertConnect& C )
		{return Ar << C.NumVertTriangles << C.TriangleListOffset;}
};

// Packed mesh vertex point for skinned meshes.
#define GET_MESHVERT_DWORD(mv) (*(DWORD*)&(mv))
struct FMeshVert
{
	// Variables.
#if __INTEL_BYTE_ORDER__
	INT X:11; INT Y:11; INT Z:10;
#else
	INT Z:10; INT Y:11; INT X:11;
#endif

	// Constructor.
	FMeshVert()
	{}
	FMeshVert( const FVector& In )
	: X((INT)In.X), Y((INT)In.Y), Z((INT)In.Z)
	{}

	// Functions.
	FVector Vector() const
	{
		return FVector( X, Y, Z );
	}

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FMeshVert& V )
	{
		return Ar << GET_MESHVERT_DWORD(V);
	}
};

// Texture coordinates associated with a vertex and one or more mesh triangles.
// All triangles sharing a vertex do not necessarily have the same texture
// coordinates at the vertex.
struct FMeshUV
{
	BYTE U;
	BYTE V;
	friend FArchive &operator<<( FArchive& Ar, FMeshUV& M )
		{return Ar << M.U << M.V;}
};

// One triangular polygon in a mesh, which references three vertices,
// and various drawing/texturing information.
struct FMeshTri
{
	_WORD		iVertex[3];		// Vertex indices.
	FMeshUV		Tex[3];			// Texture UV coordinates.
	DWORD		PolyFlags;		// Surface flags.
	INT			TextureIndex;	// Source texture index.
	friend FArchive &operator<<( FArchive& Ar, FMeshTri& T )
	{
		Ar << T.iVertex[0] << T.iVertex[1] << T.iVertex[2];
		Ar << T.Tex[0] << T.Tex[1] << T.Tex[2];
		Ar << T.PolyFlags << T.TextureIndex;
		return Ar;
	}
};

//
// A mesh, completely describing a 3D object (creature, weapon, etc) and
// its animation sequences.  Does not reference textures.
//
class ENGINE_API UUnrealMesh : public UMesh
{
	DECLARE_CLASS(UUnrealMesh,UMesh,0)

	// Objects.
	TLazyArray<FMeshVert>			Verts;
	TLazyArray<FMeshTri>			Tris;
	TArray<FMeshAnimSeq>			AnimSeqs;
	TLazyArray<FMeshVertConnect>	Connects;
	TArray<FBox>					BoundingBoxes;
	
	/* CDH: removed (never used)
	TArray<FSphere>					BoundingSpheres;//!!currently broken
	*/
	
	TLazyArray<INT>					VertLinks;
	TArray<UTexture*>				Textures;
	TArray<FLOAT>					TextureLOD;

	// Counts.
	INT						FrameVerts;
	INT						AnimFrames;

	// Render info.
	/* CDH: removed (never used)
	DWORD					AndFlags;
	DWORD					OrFlags;
	*/

	// Scaling.
	FVector					Scale;		// Mesh scaling.
	FVector 				Origin;		// Origin in original coordinate system.
	FRotator				RotOrigin;	// Amount to rotate when importing (mostly for yawing).

	// Editing info.
	/* CDH: removed (never used)
	INT						CurPoly;	// Index of selected polygon.
	INT						CurVertex;	// Index of selected vertex.
	*/

	// UObject
	UUnrealMesh();
	void Serialize(FArchive& Ar);

	// UPrimitive
	FBox GetRenderBoundingBox(const AActor* Owner, UBOOL Exact);
	UBOOL LineCheck
	(
		FCheckResult&	Result,
		AActor*			Owner,
		FVector			End,
		FVector			Start,
		FVector			Size,
		DWORD           ExtraNodeFlags,
		UBOOL			bMeshAccurate=0
	);

	// UMesh
	UClass* GetInstanceClass();

	// UUnrealMesh
	UUnrealMesh(INT NumPolys, INT NumVerts, INT NumFrames);
	virtual void SetScale(FVector InScale);
};

/*-----------------------------------------------------------------------------
	UUnrealLodMesh.
-----------------------------------------------------------------------------*/

// Compact structure to send processing info to mesh digestion at import time.
// CDH: renamed from ULODProcessInfo to FMeshLodProcessInfo to avoid confusion, as it is not a UObject
struct FMeshLodProcessInfo
{
	UBOOL LevelOfDetail;
	UBOOL NoUVData;
	UBOOL OldAnimFormat;
	INT   SampleFrame;
	INT   MinVerts;
	INT   Style;
};

// LOD-style Textured vertex struct. references one vertex, and 
// contains texture U,V information. 4 bytes.
// One triangular polygon in a mesh, which references three vertices,
// and various drawing/texturing information. 
struct FMeshWedge
{
	_WORD		iVertex;		// Vertex index.
	FMeshUV		TexUV;			// Texture UV coordinates. ( 2 bytes)
	friend FArchive &operator<<( FArchive& Ar, FMeshWedge& T )
	{
		Ar << T.iVertex << T.TexUV;
		return Ar;
	}

	FMeshWedge& operator=( const FMeshWedge& Other )
	{
		// Superfast copy by considering it as a DWORD.
		*(DWORD*)this = *(DWORD*)&Other;
		return *this;
	}	
};


// LOD-style triangular polygon in a mesh, which references three textured vertices.  8 bytes.
struct FMeshFace
{
	_WORD		iWedge[3];		// Textured Vertex indices.
	_WORD		MaterialIndex;	// Source Material (= texture plus unique flags) index.

	friend FArchive &operator<<( FArchive& Ar, FMeshFace& F )
	{
		Ar << F.iWedge[0] << F.iWedge[1] << F.iWedge[2];
		Ar << F.MaterialIndex;
		return Ar;
	}

	FMeshFace& operator=( const FMeshFace& Other )
	{
		this->iWedge[0] = Other.iWedge[0];
		this->iWedge[1] = Other.iWedge[1];
		this->iWedge[2] = Other.iWedge[2];
		this->MaterialIndex = Other.MaterialIndex;
		return *this;
	}	
};

// LOD-style mesh material.
struct FMeshMaterial
{
	DWORD		PolyFlags;		// Surface flags.
	INT			TextureIndex;	// Source texture index.
	friend FArchive &operator<<( FArchive& Ar, FMeshMaterial& M )
	{
		return Ar << M.PolyFlags << M.TextureIndex;
	}
};

//
// A LodMesh, completely describing a 3D object (creature, weapon, etc) and
// its animation sequences.  Does not reference textures.
//
class ENGINE_API UUnrealLodMesh : public UUnrealMesh
{
	DECLARE_CLASS(UUnrealLodMesh,UUnrealMesh,0)

	// LOD-specific objects.
	// Make lazy arrays where useful.
	TArray<_WORD>			CollapsePointThus;  // Lod-collapse single-linked list for points.
	TArray<_WORD>           FaceLevel;          // Minimum lod-level indicator for each face.
	TArray<FMeshFace>       Faces;              // Faces 
	TArray<_WORD>			CollapseWedgeThus;  // Lod-collapse single-linked list for the wedges.
	TArray<FMeshWedge>		Wedges;             // 'Hoppe-style' textured vertices.
	TArray<FMeshMaterial>   Materials;          // Materials
	TArray<FMeshFace>       SpecialFaces;       // Invisible special-coordinate faces.

	// Misc Internal.
	INT    ModelVerts;     // Number of 'visible' vertices.
	INT	   SpecialVerts;   // Number of 'invisible' (special attachment) vertices.

	// Max of x/y/z mesh scale for LOD gauging (works on top of drawscale).
	FLOAT  MeshScaleMax;

	// Script-settable LOD controlling parameters.
	FLOAT  LODStrength;    // Scales the (not necessarily linear) falloff of vertices with distance.
	INT    LODMinVerts;    // Minimum number of vertices with which to draw a model.
	FLOAT  LODMorph;       // >0.0 = allow morphing ; 0.0-1.0 = range of vertices to morph.
	FLOAT  LODZDisplace;   // Z displacement for LOD distance-dependency tweaking.
	FLOAT  LODHysteresis;  // Controls LOD-level change delay and morphing.

	// Remapping of animation vertices.
	TArray<_WORD> RemapAnimVerts;
	INT    OldFrameVerts;  // Possibly different old per-frame vertex count.
	
	// UObject
	UUnrealLodMesh() {}
	void Serialize(FArchive& Ar);

	// UMesh
	UClass* GetInstanceClass();

	// UUnrealMesh
	void SetScale(FVector InScale);
	UUnrealLodMesh(INT NumPolys, INT NumVerts, INT NumFrames);
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
