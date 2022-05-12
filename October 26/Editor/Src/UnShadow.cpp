/*=============================================================================
	UnLight.cpp: Bsp light mesh illumination builder code
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"
#include "UnRender.h"

/*---------------------------------------------------------------------------------------
   Globals.
---------------------------------------------------------------------------------------*/

enum {UNITS_PER_METER = 32};  /* Number of world units per meter. */
#define UNALIGNED_LIGHT 1

// Snap V to down to Grid.
inline FLOAT SnapTo( FLOAT V, FLOAT Grid )
{
	return appFloor( V / Grid ) * Grid;
}

//
// Class used for storing all globally-accessible light generation parameters:
//
class FMeshIlluminator
{
public:
	// Variables.
	ULevel*			Level;
	INT				NumLights, PolysLit, ActivePolys, RaysTraced, Pairs, Oversample;
	typedef TArray<AActor*> FActorArray;
	TArray<FActorArray> Lights;

	// Functions.
	void ComputeLightVisibility( UViewport* Viewport, AActor* Actor );
	INT ComputeAllLightVisibility( UBOOL Selected );
	void LightBspSurf( AMover* Mover, INT iSurf, INT iPoly );
	void BuildSurfList( INT iNode );
	void SetupIndex( FLightMapIndex* Index, DWORD PolyFlags, FLOAT MinU, FLOAT MinV, FLOAT MaxU, FLOAT MaxV );
};

/*---------------------------------------------------------------------------------------
   Setup.
---------------------------------------------------------------------------------------*/

void FMeshIlluminator::SetupIndex( FLightMapIndex* Index, DWORD PolyFlags, FLOAT MinU, FLOAT MinV, FLOAT MaxU, FLOAT MaxV )
{
	Index->UScale = Index->VScale
	=	((PolyFlags&(PF_HighShadowDetail|PF_LowShadowDetail))==(PF_HighShadowDetail|PF_LowShadowDetail)) ? 128
	:	(PolyFlags&PF_HighShadowDetail) ? 16
	:	(PolyFlags&PF_LowShadowDetail) ? 64
	:	32;
	Index->DataOffset = 0;
	Index->Pan.Z = 0.0;
#if UNALIGNED_LIGHT
	for( ;; )
	{
		FLOAT UExtent   = MaxU - MinU;
		Index->UClamp	= Max( 2, 1 + appFloor((UExtent-0.25)/Index->UScale) );
		FLOAT UScale    = (UExtent+0.25)/(Index->UClamp-1);
		Index->Pan.X	= MinU - 0.125;
		if( Index->UClamp<=256 )
		{
			Index->UScale = UScale;
			break;
		}
		Index->UScale *= 2;
	}
	for( ;; )
	{
		FLOAT VExtent   = MaxV - MinV; 
		Index->VClamp	= Max( 2, 1 + appFloor((VExtent-0.25)/Index->VScale) );
		FLOAT VScale    = (VExtent+0.25)/(Index->VClamp-1);
		Index->Pan.Y	= MinV - 0.125;
		if( Index->VClamp<=256 )
		{
			Index->VScale = VScale;
			break;
		}
		Index->VScale *= 2;
	}
#else
	MinU -= 0.05; MaxU += 0.05;
	MinV -= 0.05; MaxV += 0.05;
	for( ;; )
	{
		FLOAT UExtent   = MaxU - MinU;
		FLOAT VExtent   = MaxV - MinV; 

		Index->UClamp	= 2 + appFloor((UExtent+0.25)/Index->UScale);
		Index->VClamp	= 2 + appFloor((VExtent+0.25)/Index->VScale);

		Index->Pan.X	= MinU - 0.125;
		Index->Pan.Y	= MinV - 0.125;
		Index->Pan.Z    = 0.0;

		if( Index->UClamp<=128 && Index->VClamp<=128 )
			break;
		Index->UScale *= 2;
		Index->VScale *= 2;
	}
#endif
}

/*---------------------------------------------------------------------------------------
   Light visibility computation.
---------------------------------------------------------------------------------------*/

//
// Compute visibility between each light in the world and each polygon.
// Returns number of lights to be applied.
//
INT FMeshIlluminator::ComputeAllLightVisibility( UBOOL Selected )
{
	// Open a temporary (non-visible) camera.
	UViewport* Viewport = GEditor->Client->NewViewport( FName(TEXT("ComputeAllLightVisibility")) /*NAME_None*/ );
	Level->SpawnViewActor( Viewport );
	Viewport->Input->Init( Viewport );
	Viewport->Actor->ShowFlags = SHOW_PlayerCtrl;
	Viewport->Actor->RendMap = REN_PlainTex;
	Viewport->OpenWindow( 0, 1, 128, 128, INDEX_NONE, INDEX_NONE );
	Viewport->Lock( FPlane(0,0,0,0),0,0,FPlane(0,0,0,0), FPlane(0,0,0,0), FPlane(0,0,0,0), 0 );

	// Compute all light visibility.
	INT n=0;	
	DOUBLE Time = appSeconds();
	for( INT i=0; i<Level->Actors.Num(); i++ )
	{
		if( (i&15)==0 )
			GWarn->StatusUpdatef( i, Level->Actors.Num(), TEXT("%s"), "Computing visibility" );
		AActor* Actor = Level->Actors(i);
		if( Actor )
			Actor->bLightChanged = 0;
		if( Actor && Actor->LightType!=LT_None && (Actor->bStatic||Actor->bNoDelete) )
		{
			Level->SetActorZone(Actor);
			int DoLight = (Actor->bSelected || !Selected);
			if( !DoLight && Actor->Region.Zone ) 
				DoLight = Actor->Region.Zone->bSelected;
			if( DoLight )
			{
				// Mark this actor as undeletable, so it can't be deleted at playtime, causing a
				// dangling light pointer.
				Actor->bNoDelete = 1;
				Viewport->Actor->Location = Actor->Location;
				n++;

				// Process all surfaces hit by this light.
				TArray<INT> iSurfs;
				GEditor->Render->GetVisibleSurfs( Viewport, iSurfs );

				for( INT i=0; i<iSurfs.Num(); i++ )
				{
					check(iSurfs(i)>=0);
					check(iSurfs(i)<Level->Model->Surfs.Num());
					check(Lights.Num()==Level->Model->Surfs.Num());
					FBspSurf& Poly = Level->Model->Surfs( iSurfs(i) );
					FPlane Plane( Level->Model->Points(Poly.pBase), Level->Model->Vectors(Poly.vNormal) );
					if
					(	Poly.iLightMap!=INDEX_NONE
					&&	(Actor->bSpecialLit ? (Poly.PolyFlags&PF_SpecialLit) : !(Poly.PolyFlags&PF_SpecialLit))
					&&	Abs(Plane.PlaneDot(Actor->Location))<=Actor->WorldLightRadius() )
					{
						Lights(iSurfs(i)).AddItem( Actor );
						NumLights++;
						Pairs++;
					}
				}
			}
		}
	}
	Time = appSeconds() - Time;
	debugf( NAME_Log, TEXT("Occluded %i views in %f sec (%f msec per light)"), n*6, Time, Time*1000.0/n/6 );

	// Delete viewport.
	Viewport->Unlock(0);
	delete Viewport;

	return n;
}

/*---------------------------------------------------------------------------------------
   Polygon lighting.
---------------------------------------------------------------------------------------*/

//
// Apply all lights to one poly, generating its lighting mesh and updating
// the tables:
//
void FMeshIlluminator::LightBspSurf( AMover* Mover, INT iSurf, INT iPoly )
{
	FBspSurf& Surf = Level->Model->Surfs(iSurf);
	check(Surf.iLightMap!=INDEX_NONE);
	UModel* Model = Mover ? Mover->Brush : Level->Model;
	FLightMapIndex* Index = &Model->LightMap( Surf.iLightMap );

	// Get numbers.
	FVector	Base      =  Level->Model->Points (Surf.pBase);
	FVector	Normal    =  Level->Model->Vectors(Surf.vNormal);
	FVector	TextureU  =  Level->Model->Vectors(Surf.vTextureU);
	FVector	TextureV  =  Level->Model->Vectors(Surf.vTextureV);

	// Find extent of static world surface from all of the Bsp polygons that use the surface.
	FLOAT MinU = +10000000.0;
	FLOAT MinV = +10000000.0;
	FLOAT MaxU = -10000000.0;
	FLOAT MaxV = -10000000.0;
	if( Mover )
	{
		// Compute extent.
		FPoly& Poly = Mover->Brush->Polys->Element( iPoly );
		for( INT k=0; k<Poly.NumVertices; k++ )
		{
			// Find extent in untransformed brush space.
			FVector Vertex	= Poly.Vertex[k] - Poly.Base;
			FLOAT	U		= Vertex | Poly.TextureU;
			FLOAT	V		= Vertex | Poly.TextureV;
			MinU=Min(U,MinU); MaxU=Max(U,MaxU);
			MinV=Min(V,MinV); MaxV=Max(V,MaxV);
		}
		SetupIndex( Index, Poly.PolyFlags | (Mover->bDynamicLightMover ? PF_LowShadowDetail : 0), MinU, MinV, MaxU, MaxV );
		if( Mover->bDynamicLightMover )
			return;
	}
	else
	{
		for( INT i=0; i<Level->Model->Nodes.Num(); i++ )
		{
			FBspNode& Node = Level->Model->Nodes(i);
			if( Node.iSurf==iSurf && Node.NumVertices>0 )
			{
				FVert *VertPool = &Level->Model->Verts(Node.iVertPool);
				for( BYTE B=0; B < Node.NumVertices; B++ )
				{
					FVector Vertex	= Level->Model->Points(VertPool[B].pVertex) - Base;
					FLOAT	U		= Vertex | TextureU;
					FLOAT	V		= Vertex | TextureV;
					MinU            = Min(U,MinU);
					MaxU            = Max(U,MaxU);
					MinV            = Min(V,MinV);
					MaxV            = Max(V,MaxV);
				}
			}
		}
		SetupIndex( Index, Surf.PolyFlags, MinU, MinV, MaxU, MaxV );
	}

	// Calculate coordinates.
	FVector		NewBase		 = Level->Model->Points(Surf.pBase) + Normal * 4.0;
	INT			ByteSize	 = ((Index->UClamp+7)>>3) * Index->VClamp;
	FCoords		TexCoords    = FCoords( FVector(0,0,0), TextureU, TextureV, Normal ).Inverse().Transpose();

	// Raytrace each lightsource.
	if( Lights(iSurf).Num() )
	{
		// Setup index.
		Index->DataOffset   = Model->LightBits.Num();
		Index->iLightActors = Model->Lights.Num();

		// Perform raytracing.
		TArray<BYTE> Data;
		Data.Add( ByteSize );
		for( INT i=0; i<Lights(iSurf).Num(); i++ )
		{
			// Prepare.
			AActor* Actor       = Lights(iSurf)(i);
			UBOOL   DidHit      = 0;
			FLOAT	SqRadius	= Square(Actor->WorldLightRadius());
			FLOAT   UScale      = Index->UScale;
			FLOAT   VScale      = Index->VScale;
			FVector	Vertex0		= NewBase + TexCoords.XAxis*Index->Pan.X + TexCoords.YAxis*Index->Pan.Y;
#if UNALIGNED_LIGHT
			if( Surf.PolyFlags & PF_BrightCorners )
			{
				UScale  -= 0.5 / (Index->UClamp-1);
				VScale  -= 0.5 / (Index->VClamp-1);
				Vertex0 += TexCoords.XAxis*0.25 + TexCoords.YAxis*0.25;
			}
#endif
			FVector VertexDU	= TexCoords.XAxis * UScale;
			FVector VertexDV	= TexCoords.YAxis * VScale;

			// Perform raytracing.
			INT Count=0;
			FCheckResult Hit(0.0);
			BYTE NodeFlags = NF_NotVisBlocking;
			if( Surf.PolyFlags & PF_BrightCorners )
				NodeFlags |= NF_BrightCorners;
			for( INT VCounter=0; VCounter<Index->VClamp; VCounter++ )
			{
				FVector Vertex=Vertex0;
				for( INT UCounter=0; UCounter<Index->UClamp; UCounter+=8 )
				{
					BYTE B = 0;
					BYTE M = 1;
					UBOOL Prev = 0;
					for( INT SubU=UCounter; SubU<Index->UClamp && M; SubU++,M=M<<1 )
					{
						if( FDistSquared(Actor->Location,Vertex) < SqRadius )
							if( (Prev=Level->Model->LineCheck( Hit, NULL, Actor->Location, Vertex, FVector(0,0,0), NodeFlags ))!=NULL )
							//if( (Prev=Level->SingleLineCheck( Hit, Actor, Actor->Location, Vertex, TRACE_Movers|TRACE_Level, FVector(0,0,0), NodeFlags))!=0 )
								{B |= M; DidHit=1;}
						RaysTraced++;
						Vertex += VertexDU;
					}
					if( Prev )
						for( M; M; M*=2 )
							B |= M;
					Data(Count++) = B;
				}
				Vertex0 += VertexDV;
			}

			// If any hit, add data.
			if( DidHit )
			{
				Model->Lights.AddItem( Actor );
				for( INT i=0; i<ByteSize; i++ )
					Model->LightBits.AddItem( Data(i) );
			}
		}
		Model->Lights.AddItem( NULL );
	}
}

/*---------------------------------------------------------------------------------------
   Index building
---------------------------------------------------------------------------------------*/

//
// Recursively go through the Bsp nodes and build a list of active Bsp surfs,
// allocating their light map indices.
//
void FMeshIlluminator::BuildSurfList( INT iNode )
{
	FBspNode& Node = Level->Model->Nodes(iNode);
	FBspSurf& Surf = Level->Model->Surfs(Node.iSurf);
	if( Node.NumVertices )
	{
		if( !(Surf.PolyFlags & PF_NoShadows) )
		{
			if( Surf.iLightMap==INDEX_NONE )
			{
				Surf.iLightMap = Level->Model->LightMap.Add();
				PolysLit++;
			}
		}
	}
	if( Node.iFront != INDEX_NONE ) BuildSurfList(Node.iFront);
	if( Node.iBack  != INDEX_NONE ) BuildSurfList(Node.iBack);
	if( Node.iPlane != INDEX_NONE ) BuildSurfList(Node.iPlane);
}

/*---------------------------------------------------------------------------------------
   High-level lighting routine
---------------------------------------------------------------------------------------*/

void UEditorEngine::shadowIlluminateBsp( ULevel* Level, INT Selected )
{
	FMeshIlluminator Illum;

	// Init.
	GCache.Flush();
	Trans->Reset( TEXT("Rebuilding lighting") );
	GWarn->BeginSlowTask( TEXT("Raytracing"), 1, 0 );

	Illum.Level = Level;
	if( Illum.Level->Model->Nodes.Num() != 0 )
	{
		// Empty lighting.
		Level->Model->LightMap.Empty();
		Level->Model->LightBits.Empty();

		// Init stats.
		Illum.PolysLit			= 0;
		Illum.RaysTraced		= 0;
		Illum.ActivePolys		= 0;
		Illum.Pairs				= 0;

		// Clear all surface light mesh indices.
		for( INT i=0; i<Level->Model->Surfs.Num(); i++ )
			Level->Model->Surfs(i).iLightMap = INDEX_NONE;

		// Tell all actors that we're about to raytrace the world.
		// This enables movable brushes to set their positions for raytracing.
		for( i=0; i<Level->Actors.Num(); i++ )
		{
			if( (i&15)==0 )
				GWarn->StatusUpdatef( i, Illum.Level->Actors.Num(), TEXT("%s"), TEXT("Allocating meshes") );
			AMover* Actor = Cast<AMover>( Level->Actors(i) );
			if( Actor )
			{
				Actor->PreRaytrace();
				Actor->SetWorldRaytraceKey();
				UModel* Model = Actor->Brush;
				if( Model && Model->Polys )
				{
					Model->LightMap.Empty();
					for( INT j=0; j<Model->Polys->Element.Num(); j++ )
					{
						FPoly& Poly = Model->Polys->Element(j);
						if( !(Poly.PolyFlags & PF_NoShadows) )
						{
							// Add light map index.
							Poly.iBrushPoly = Model->LightMap.Add();
							FLightMapIndex* Index = &Model->LightMap(Poly.iBrushPoly);
							Index->iLightActors = INDEX_NONE;
						}
						else Poly.iBrushPoly = INDEX_NONE;
					}
				}
			}
		}

		// Compute light visibility and update index with it.
		Level->BrushTracker = GNewBrushTracker( Level );
		Illum.BuildSurfList( 0 );
		for( i=0; i<Level->Model->Surfs.Num(); i++ )
			new(Illum.Lights)TArray<AActor*>;
		Illum.NumLights = Illum.ComputeAllLightVisibility( Selected );

		// Lock the level with collision.
		//Level->SetActorCollision( 1 );

		// Make light list.
		for( i=0; i<Level->Model->LightMap.Num(); i++ )
			Level->Model->LightMap(i).iLightActors=INDEX_NONE;

		// Count raytraceable surfs.
		INT n=0, c=0;
		for( i=0; i<Level->Model->Surfs.Num(); i++ )
			n += (Level->Model->Surfs(i).iLightMap != INDEX_NONE);

		// Raytrace each world surface.
		for( i=0; i<Level->Model->Surfs.Num(); i++ )
		{
			if
			(	Level->Model->Surfs(i).iLightMap!=INDEX_NONE
			&&	!Level->BrushTracker->SurfIsDynamic(i) )
			{
				GWarn->StatusUpdatef( c++, n, TEXT("%s"), TEXT("Raytracing") );
				Illum.LightBspSurf( NULL, i, 0 );
			}
		}

		// Raytrace the movers.
		for( i=0; i<Level->Actors.Num(); i++ )
		{
			AMover* Actor = Cast<AMover>( Level->Actors(i) );
			if( Actor && Actor->Brush && Actor->Brush->Polys )
			{
				Actor->SetBrushRaytraceKey();
				Level->BrushTracker->Flush( Actor );
				for( INT j=0; j<Actor->Brush->Polys->Element.Num(); j++ )
				{
					FPoly& Poly = Actor->Brush->Polys->Element(j);
					if( Poly.iLink!=INDEX_NONE && Poly.iBrushPoly!=INDEX_NONE )
						Illum.LightBspSurf( Actor, Poly.iLink, j );
				}
				Actor->SetWorldRaytraceKey();
			}
		}
		delete Level->BrushTracker;
		Level->BrushTracker = NULL;

		// Tell all actors that we're done raytracing the world.
		for( i=0; i<Level->Actors.Num(); i++ )
		{
			AActor* Actor = Illum.Level->Actors(i);
			if( Actor )
				Actor->bDynamicLight = 0;
			if( Cast<AMover>(Actor) )
				Cast<AMover>(Actor)->PostRaytrace();
		}
		debugf( NAME_Log, TEXT("%i Lights, %i Polys, %i Pairs, %i Rays"), Illum.NumLights, Illum.PolysLit, Illum.Pairs, Illum.RaysTraced );

		// Finish up.
		//Level->SetActorCollision( 0 );
	}

	GWarn->EndSlowTask();
	GCache.Flush();

}

/*---------------------------------------------------------------------------------------
   Light link topic handler
---------------------------------------------------------------------------------------*/

AUTOREGISTER_TOPIC(TEXT("Light"),LightTopicHandler);
void LightTopicHandler::Get( ULevel* Level, const TCHAR* Item, FOutputDevice& Ar )
{
	int Meshes=0,MeshPts=0,Size=0,MaxSize=0,CacheSize=0,Meters=0,LightCount=0,SelCount=0;

	for( int i=0; i<Level->Actors.Num(); i++ )
	{
		AActor *Actor = Level->Actors(i);
		if( Actor && Actor->LightBrightness )
		{
			LightCount++;
			if( Actor->bSelected )
				SelCount++;
		}
	}

	if( !Level || !Level->Model->LightMap.Num() )
	{
		Meshes = 0;
	}
	else
	{
		for( INT i=0; i<Level->Model->LightMap.Num(); i++ )
		{
			Size       = (int)Level->Model->LightMap(i).UClamp * (int)Level->Model->LightMap(i).VClamp;
			MeshPts   += Size;
	  		CacheSize += Size * Level->Model->LightMap(i).UScale * Level->Model->LightMap(i).UScale;
			if( Size > MaxSize ) MaxSize = Size;
		}
		Meters = CacheSize / (UNITS_PER_METER * UNITS_PER_METER);
	}
    if      (appStricmp(Item,TEXT("Meshes"))==0) 	Ar.Logf(TEXT("%i"),Meshes);
    else if (appStricmp(Item,TEXT("MeshPts"))==0) 	Ar.Logf(TEXT("%i"),MeshPts);
    else if (appStricmp(Item,TEXT("MaxSize"))==0) 	Ar.Logf(TEXT("%i"),MaxSize);
    else if (appStricmp(Item,TEXT("Meters"))==0) 	Ar.Logf(TEXT("%i"),Meters);
    else if (appStricmp(Item,TEXT("Count"))==0) 	Ar.Logf(TEXT("%i (%i)"),LightCount,SelCount);
    else if (appStricmp(Item,TEXT("AvgSize"))==0) 	Ar.Logf(TEXT("%i"),MeshPts/Max(1,Meshes));
    else if (appStricmp(Item,TEXT("CacheSize"))==0)	Ar.Logf(TEXT("%iK"),CacheSize/1000);

}
void LightTopicHandler::Set( ULevel* Level, const TCHAR* Item, const TCHAR* Data )
{
};

/*---------------------------------------------------------------------------------------
   The End
---------------------------------------------------------------------------------------*/
