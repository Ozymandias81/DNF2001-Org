/*=============================================================================
	UnRoute.cpp: Unreal AI routing code.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

Revision history:
	* ...
=============================================================================*/

#include "EnginePrivate.h"

/* clearPaths()
clear all temporary path variables used in routing
*/
inline void APawn::clearPath(ANavigationPoint *node)
{
	//debugf("Clear %s",node->GetName());
	node->visitedWeight = 10000000;
	node->nextOrdered = NULL;
	node->prevOrdered = NULL;
	node->bEndPoint = 0;
	if ( node->bSpecialCost )
		node->cost = node->eventSpecialCost(this);
	else
		node->cost = node->ExtraCost;
}

void APawn::clearPaths()
{
	guard(APawn::clearPaths);
	ANavigationPoint *Nav = GetLevel()->GetLevelInfo()->NavigationPointList;
	while (Nav)
	{
		clearPath(Nav);
		Nav = Nav->nextNavigationPoint;
	}
	unguard;
}

void FSortedPathList::FindVisiblePaths(APawn *Searcher, FVector Dest, FSortedPathList *DestPoints, INT bClearPaths,
							 INT &startanchor, INT &endanchor)
{
	guard(FSortedPathList::FindVisiblePaths);

	//unclock(GetLevel()->FindPathCycles);
	//debugf("Pre-Vis time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//debugf("%d actors in level", GetLevel()->Num);
	//if (startanchor) debugf("Start anchor");
	//if (endanchor) debugf("End anchor");
	//clock(GetLevel()->FindPathCycles);

	// find paths visible from this pawn
	//FIXME - PVS check here
	
	if ( Searcher->MoveTarget && Searcher->MoveTarget->IsA(ANavigationPoint::StaticClass()) 
		&& (Abs(Searcher->MoveTarget->Location.Z - Searcher->Location.Z) < (Searcher->CollisionHeight+Searcher->MoveTarget->CollisionHeight) ) )
	{
		FVector TargDir = Searcher->MoveTarget->Location - Searcher->Location;
		TargDir.Z = 0;
		FLOAT MaxDist = ::Max(1024.f, Searcher->CollisionRadius * Searcher->CollisionRadius);
		if ( Searcher->bIsPlayer && Searcher->MoveTarget->IsA(AInventorySpot::StaticClass()) )
			MaxDist *= 2; //FIXME - not just for players
		if ( (TargDir | TargDir) < MaxDist )
		{
			startanchor = 1;
			Path[0] = Searcher->MoveTarget;
			Dist[0] = 0;
			numPoints = 1;
			//debugf("Have a start anchor");
		}
	}

	FCheckResult Hit(1.0);
	ANavigationPoint *Nav = Searcher->GetLevel()->GetLevelInfo()->NavigationPointList;
	int dist;
	while (Nav)
	{
		if (bClearPaths)
			Searcher->clearPath(Nav);
		if ( !startanchor )
		{
			dist = (int)(Searcher->Location - Nav->Location).SizeSquared();
			if ( dist < 640000 )
				addPath(Nav, dist);
		}
		if ( !endanchor )
		{
			dist = (int)(Dest - Nav->Location).SizeSquared();
			if ( dist < 640000 )
				DestPoints->addPath(Nav, dist);
		}
		Nav = Nav->nextNavigationPoint;
	}

	//unclock(GetLevel()->FindPathCycles);
	//debugf("Vis time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//if (startanchor) debugf("Start anchor");
	//if (endanchor) debugf("End anchor");
	//clock(GetLevel()->FindPathCycles);

	unguard;
}

int FSortedPathList::findEndPoint(APawn *Searcher, INT &startanchor) 
{
	guard(FSortedPathList::findEndPoint);

	//find nearest path reachable by this pawn
	FVector	ViewPoint = Searcher->Location;
	ViewPoint.Z += Searcher->BaseEyeHeight; //look from eyes
	ULevel *MyLevel = Searcher->GetLevel();

	while (numPoints > 0)
	{
		if ( MyLevel->Model->FastLineCheck(Path[0]->Location, ViewPoint)
			&& Searcher->pointReachable(Path[0]->Location, 1) )
		{
			Dist[0] = (INT) appSqrt(Dist[0]);
			if ( (Dist[0] < ::Max(64, (int)Searcher->CollisionRadius))
				&& (Abs(Path[0]->Location.Z - Searcher->Location.Z) < (Searcher->CollisionHeight + Path[0]->CollisionHeight)) )
				startanchor = 1;
			else
			{
				((ANavigationPoint *)Path[0])->bEndPoint = 1;
				((ANavigationPoint *)Path[0])->bestPathWeight = Dist[0];
			}
			return 1;
		}
		else 
			removePath(0);
	}

	return 0;
	unguard;
}

int FSortedPathList::checkAnchorPath(APawn *Searcher, FVector Dest) 
{
	guard(FSortedPathList::checkAnchorPath);

	ULevel *MyLevel = Searcher->GetLevel();
	FVector RealLocation = Searcher->Location;

	if ( (Dest - Path[0]->Location).SizeSquared() < 640000.f ) 
	{	
		if( MyLevel->Model->FastLineCheck(Path[0]->Location, Dest)
			&& MyLevel->FarMoveActor(Searcher, Path[0]->Location, 1) )
		{
			if ( Searcher->pointReachable(Dest) ) //NOTE- reachable needs to do its own trace here always - to guarantee no error
				return 1;
			MyLevel->FarMoveActor(Searcher, RealLocation, 1, 1);
		}
	}
	numPoints = 1;
	return 0;
	unguard;
}

void FSortedPathList::expandAnchor(APawn *Searcher) 
{
	guard(FSortedPathList::expandAnchor);

	ULevel *MyLevel = Searcher->GetLevel();
	ANavigationPoint *anchor = (ANavigationPoint *)Path[0];
	anchor->cost = 1000000; //paths shouldn't go through anchor
	INT j = 0;
	FReachSpec *spec;
	FCheckResult Hit;
	INT moveFlags = Searcher->calcMoveFlags(); 
	INT iRadius = (int)(Searcher->CollisionRadius);
	INT iHeight = (int)(Searcher->CollisionHeight);
	while (j<16)
	{
		if (anchor->Paths[j] == -1)
			j = 16;
		else
		{
			spec = &MyLevel->ReachSpecs(anchor->Paths[j]);
			//debugf("Expand to %s",spec->End->GetName()); 
			if ( spec->supports(iRadius, iHeight, moveFlags) )
			{
				MyLevel->SingleLineCheck(Hit, Searcher, spec->End->Location, spec->Start->Location, TRACE_VisBlocking);
				if ( !Hit.Actor || !Hit.Actor->IsA(AMover::StaticClass()) 
					|| (Searcher->bCanOpenDoors && (Searcher->bIsPlayer || !((AMover *)Hit.Actor)->bPlayerOnly)) )
				{
					//debugf("Expansion to %s successful",spec->End->GetName()); 
					((ANavigationPoint *)spec->End)->bEndPoint = 1;
					((ANavigationPoint *)spec->End)->bestPathWeight = spec->distance;
				}
			}
			j++;
		}
	}
	unguard;
}

int APawn::CanMoveTo(AActor *Anchor, AActor *Dest)
{
	guard(APawn::CanMoveTo);

	ULevel *MyLevel = GetLevel();
	ANavigationPoint *Start = (ANavigationPoint *)Anchor;
	INT j = 0;
	FReachSpec *spec;
	FCheckResult Hit;

	while (j<16)
	{
		if (Start->Paths[j] == -1)
			j = 16;
		else
		{
			spec = &MyLevel->ReachSpecs(Start->Paths[j]);
			if ( (spec->End == Dest) 
				&& spec->supports((int)CollisionRadius,
									(int)CollisionHeight,
									calcMoveFlags()) )
			{
				MyLevel->SingleLineCheck(Hit, this, Dest->Location, Start->Location, TRACE_VisBlocking);
				if ( !Hit.Actor || !Hit.Actor->IsA(AMover::StaticClass()) 
					|| (bCanOpenDoors && (bIsPlayer || !((AMover *)Hit.Actor)->bPlayerOnly)) )
					return 1;
			}
			j++;
		}
	}

	j = 0;
	while (j<16)
	{
		if (Start->PrunedPaths[j] == -1)
			j = 16;
		else
		{
			spec = &MyLevel->ReachSpecs(Start->PrunedPaths[j]);
			if ( (spec->End == Dest)
				&& spec->supports((int)CollisionRadius,
									(int)CollisionHeight,
									calcMoveFlags()) )
			{
				MyLevel->SingleLineCheck(Hit, this, Dest->Location, Start->Location, TRACE_VisBlocking);
				if ( !Hit.Actor || !Hit.Actor->IsA(AMover::StaticClass()) 
					|| (bCanOpenDoors && (bIsPlayer || !((AMover *)Hit.Actor)->bPlayerOnly)) )
					return 1;
			}
			j++;
		}
	}
	
	return 0;
	unguard;
}

void FSortedPathList::findAltEndPoint(APawn *Searcher, AActor *&bestPath) 
{
	guard(FSortedPathList::findAltEndPoint);

	//check if other paths (beyond Path[0]) might be better destinations
	int bestDist = ((ANavigationPoint *)Path[0])->visitedWeight + Dist[0]; 
	FSortedPathList AltEndPoints;
	AltEndPoints.numPoints = 0;
	for (int j=1; j<numPoints; j++)
	{
		int newDist = (INT) appSqrt(Dist[j]);
		newDist += ((ANavigationPoint *)Path[j])->visitedWeight;
		if ( (newDist < bestDist) && (Abs(Path[j]->Location.Z - Searcher->Location.Z) < 120)
			&& ((((Path[j]->Location - Searcher->Location) | (bestPath->Location - Searcher->Location)) < 0)
			|| (newDist < ::Max((int)(0.85 * bestDist), bestDist - 150))) )
			AltEndPoints.addPath(Path[j], newDist);
	}
	FVector	ViewPoint = Searcher->Location;
	ViewPoint.Z += Searcher->BaseEyeHeight; //look from eyes
	ULevel *MyLevel = Searcher->GetLevel();

	for ( j=0; j<AltEndPoints.numPoints; j++ )
	{
		if ( MyLevel->Model->FastLineCheck(AltEndPoints.Path[j]->Location, ViewPoint)
			&& Searcher->pointReachable(AltEndPoints.Path[j]->Location, 1) )
		{
			bestPath = AltEndPoints.Path[j];
			return;
		}
	}

	unguard;
}

int APawn::findPathToward(AActor *goal, INT bSinglePath, AActor *&bestPath, INT bClearPaths)
{
	guard(APawn::findPathToward);

	bestPath = NULL;
	if (!goal)
		return 0;

	if ( !GetLevel()->GetLevelInfo()->NavigationPointList || !GetLevel()->ReachSpecs.Num() )
	{
		//FName statename = GetState();
		//debugf("No Paths Defined for findpathtoward for %s in state %s",GetName(), *statename);
		return 0;
	}

	//unclock(GetLevel()->FindPathCycles);
	//debugf(" %s FindPathToward %s", GetName(), goal->GetName());
	//debugf("Start time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//clock(GetLevel()->FindPathCycles);

	if ( (Physics != PHYS_Flying) && goal->IsA(APawn::StaticClass()) && (goal->Physics == PHYS_Falling) )
	{
		FVector Dest = goal->Location;
		((APawn *)goal)->jumpLanding(goal->Velocity, Dest);
		//debugf("Goal pawn is falling");
		return findPathTo(Dest, bSinglePath, bestPath, bClearPaths);
	}

	FVector RealLocation = Location;
	FSortedPathList EndPoints;
	FSortedPathList DestPoints;
	EndPoints.numPoints = 0;
	DestPoints.numPoints = 0; 
	INT startanchor = 0;
	INT endanchor = 0;

	if ( goal->IsA(ANavigationPoint::StaticClass()) )
	{
		//debugf("found an end anchor");
		endanchor = 1;
		DestPoints.Path[0] = goal;
		DestPoints.Dist[0] = 0;
		DestPoints.numPoints = 1;
	}

	EndPoints.FindVisiblePaths(this, goal->Location, &DestPoints, bClearPaths, startanchor, endanchor);
	if ( (EndPoints.numPoints == 0) || (DestPoints.numPoints == 0) )
		return 0;
	//debugf("Visible endpoints = %d", EndPoints.numPoints);
	//debugf("visible destpoints = %d", DestPoints.numPoints);

	if ( !startanchor && !EndPoints.findEndPoint(this, startanchor) )
	{
		GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
		//debugf("failed to find an endpoint");
		return 0;
	}
	if ( startanchor )
	{
		//debugf("Start anchor is %s", EndPoints.Path[0]->GetName());
		if ( (goal->IsA(ANavigationPoint::StaticClass()) && CanMoveTo(EndPoints.Path[0], goal)) || EndPoints.checkAnchorPath(this, goal->Location) )
		{
			if ( goal->IsA(ANavigationPoint::StaticClass()) )
				bestPath = goal;
			else
				bestPath = EndPoints.Path[0];
			GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
			//debugf("anchor works");
			return 1;
		}
		else
			EndPoints.expandAnchor(this);
	}

	//now explore paths from destination
	if ( !endanchor ) //find an end anchor
	{
		FVector	ViewPoint = Location;
		ViewPoint.Z += BaseEyeHeight; //look from eyes
		INT j = 0;
		while (j<DestPoints.numPoints)
		{
			if ( GetLevel()->Model->FastLineCheck(goal->Location, DestPoints.Path[j]->Location)
				&& GetLevel()->FarMoveActor(this, DestPoints.Path[j]->Location, 1) )
			{
				if ( actorReachable(goal, 1) )
				{
					endanchor = 1;
					DestPoints.Path[0] = DestPoints.Path[j];
					DestPoints.Dist[0] = (INT) appSqrt(DestPoints.Dist[j]);
					j = DestPoints.numPoints;
				}
			}
			j++;
		}
	}

	if ( !endanchor && bHunting )
		endanchor = 1;

	//unclock(GetLevel()->FindPathCycles);
	//debugf("Setup time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//clock(GetLevel()->FindPathCycles);
	//if ( !endanchor )
	//	debugf("No end anchor found");
	if ( endanchor )
	{
		AActor *newPath = NULL;
		int moveFlags = calcMoveFlags();
		((ANavigationPoint *)DestPoints.Path[0])->visitedWeight = DestPoints.Dist[0];
		if (breadthPathFrom(DestPoints.Path[0], newPath, bSinglePath, moveFlags))
		{
			bestPath = newPath;
			GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
			if ( !startanchor && !bSinglePath )
				EndPoints.findAltEndPoint(this, bestPath);
			SetRouteCache((ANavigationPoint *)bestPath);
			return 1;
		}
	}

	GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
	return 0;

	unguard;
}

/* findPathTowardBestInventory()
	Used by bots.
	Determine the best inventory item goal (weighted by distance and its botdesireability)
	returns the bestPath
*/
FLOAT APawn::findPathTowardBestInventory(AActor *&bestPath, INT bClearPaths, FLOAT MinWeight, INT bPredictRespawns)
{
	guard(APawn::findPathTowardBestInventory);

	bestPath = NULL;
	if ( !GetLevel()->GetLevelInfo()->NavigationPointList || !GetLevel()->ReachSpecs.Num() )
	{
		//FName statename = GetState();
		//debugf("No Paths Defined for findpathtowardbestinventory for %s in state %s",GetName(), *statename);
		return 0;
	}

	//unclock(GetLevel()->FindPathCycles);
	//debugf(" %s FindPathTowardBestInventory", GetName());
	//debugf("Start time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//clock(GetLevel()->FindPathCycles);

	FVector RealLocation = Location;
	FSortedPathList EndPoints;
	FSortedPathList DestPoints;
	EndPoints.numPoints = 0;
	DestPoints.numPoints = 0; 
	INT startanchor = 0;
	INT endanchor = 1;

	EndPoints.FindVisiblePaths(this, FVector(0,0,0), &DestPoints, bClearPaths, startanchor, endanchor);
	if ( EndPoints.numPoints == 0 )
		return 0;
	//debugf(NAME_DevPath,"visible endpoints = %d", EndPoints.numPoints);
	//debugf("visible destpoints = %d", DestPoints.numPoints);

	if ( !startanchor && !EndPoints.findEndPoint(this, startanchor) )
	{
		GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
		return 0;
	}
	if ( !startanchor ) //then get on the network
	{
		bestPath = EndPoints.Path[0];
		GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
		return 0.00005;
	}

	EndPoints.expandAnchor(this);

	//unclock(GetLevel()->FindPathCycles);
	//debugf(NAME_DevPath,"Setup time");
	//clock(GetLevel()->FindPathCycles);

	AActor *newPath = NULL;
	int moveFlags = calcMoveFlags();
	((ANavigationPoint *)EndPoints.Path[0])->visitedWeight = ::Max(10, EndPoints.Dist[0]);
	FLOAT bestInventoryWeight = breadthPathToInventory(EndPoints.Path[0], newPath, moveFlags, MinWeight, bPredictRespawns);
	//debugf(NAME_DevPath,"BestInv is %f compared to weight %f", bestInventoryWeight, MinWeight);
	if ( bestInventoryWeight > MinWeight)
	{
		bestPath = newPath;
		SetRouteCache((ANavigationPoint *)bestPath);
		GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
		return bestInventoryWeight;
	}

	GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
	return 0;

	unguard;
}
int APawn::findPathTo(FVector Dest, INT bSinglePath, AActor *&bestPath, INT bClearPaths)
{
	guard(APawn::findPathTo);

	//unclock(GetLevel()->FindPathCycles);
	//debugf("Start time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//clock(GetLevel()->FindPathCycles);
	bestPath = NULL;
	if ( !GetLevel()->GetLevelInfo()->NavigationPointList || !GetLevel()->ReachSpecs.Num() )
	{
		//FName statename = GetState();
		//debugf("No Paths Defined for findpathto for %s in state %s",GetName(), *statename);
		return 0;
	}

	FVector RealLocation = Location;
	FSortedPathList EndPoints;
	FSortedPathList DestPoints;
	EndPoints.numPoints = 0;
	DestPoints.numPoints = 0; //FIXME - this should be done by FSortedPathList when constructed
	INT startanchor = 0;
	INT endanchor = 0;

	EndPoints.FindVisiblePaths(this, Dest, &DestPoints, bClearPaths, startanchor, endanchor);
	//debugf("Visible endpoints = %d", EndPoints.numPoints);
	//debugf("visible destpoints = %d", DestPoints.numPoints);
	if ((EndPoints.numPoints == 0) || (DestPoints.numPoints == 0))
		return 0;

	if ( !startanchor && !EndPoints.findEndPoint(this, startanchor) )
	{
		GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
		return 0;
	}
	if ( startanchor )
	{
		if ( EndPoints.checkAnchorPath(this, Dest) )
		{
			bestPath = EndPoints.Path[0];
			GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
			return 1;
		}
		else
			EndPoints.expandAnchor(this);
	}

	//now explore paths from destination
	if ( !endanchor ) //find an end anchor
	{
		FVector	ViewPoint = Location;
		ViewPoint.Z += BaseEyeHeight; //look from eyes
		INT j = 0;
		while (j<DestPoints.numPoints)
		{
			if ( GetLevel()->Model->FastLineCheck(Dest, DestPoints.Path[j]->Location)
				&& GetLevel()->FarMoveActor(this, DestPoints.Path[j]->Location, 1)
				&& pointReachable(Dest, 1))
				{
					endanchor = 1;
					DestPoints.Path[0] = DestPoints.Path[j];
					DestPoints.Dist[0] = (INT) appSqrt(DestPoints.Dist[j]);
					j = DestPoints.numPoints;
				}
			j++;
		}
	}

	//unclock(GetLevel()->FindPathCycles);
	//debugf("Setup time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
	//debugf("Dest point is %s",DestPoints.Path[0]->GetName());
	//clock(GetLevel()->FindPathCycles);

	if (endanchor)
	{
		AActor *newPath = NULL;
		int moveFlags = calcMoveFlags(); 
		((ANavigationPoint *)DestPoints.Path[0])->visitedWeight = DestPoints.Dist[0];
		if (breadthPathFrom(DestPoints.Path[0], newPath, bSinglePath, moveFlags))
		{
			//unclock(GetLevel()->FindPathCycles);
			//debugf("BFS time was %f", GetLevel()->FindPathCycles * GSys->MSecPerCycle);
			//clock(GetLevel()->FindPathCycles);
			bestPath = newPath;
			GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
			if ( !startanchor && !bSinglePath )
				EndPoints.findAltEndPoint(this, bestPath);
			SetRouteCache((ANavigationPoint *)bestPath);
			return 1;
		}
	}

	GetLevel()->FarMoveActor(this, RealLocation, 1, 1);
	return 0;
	unguard;
}

/* FindRandomDest()
returns a random pathnode reachable from creature's current location
FIXME- clean up like FindPathTo()
*/
int APawn::findRandomDest(AActor *&bestPath)
{
	guard(APawn::findRandomDest);
	int result = 0;
	ULevel *MyLevel = GetLevel();
	if ( MyLevel->GetLevelInfo()->NavigationPointList || !GetLevel()->ReachSpecs.Num() )
	{
		//debugf("No Paths Defined for findrandomdest for %s in state %s",GetName(), GetMainStack()->Describe() );
		return 0;
	}
	FSortedPathList EndPoints;
	EndPoints.numPoints = 0;

	// find paths visible from this pawn
	FVector	ViewPoint = Location;
	ViewPoint.Z += BaseEyeHeight; //look from eyes

	AActor *Actor = GetLevel()->GetLevelInfo()->NavigationPointList;
	while (Actor)
	{
		if ( (Location - Actor->Location).SizeSquared() < 250000 )
		{
			if ( MyLevel->Model->FastLineCheck(Actor->Location, ViewPoint) )
			{ 	
				FVector dir = Actor->Location - Location;
				FLOAT dist = dir.Size();
				EndPoints.addPath(Actor, dist); 
				if ( EndPoints.numPoints == 4 )
					break;
			}
		}
		Actor = ((ANavigationPoint *)Actor)->nextNavigationPoint;
	}

	//mark reachable path nodes
	int numReached = 0;
	int moveFlags = calcMoveFlags(); 

	for(INT i=0; i<EndPoints.numPoints; i++)
	{
		if (!((ANavigationPoint *)EndPoints.Path[i])->bEndPoint)
			if ( actorReachable(EndPoints.Path[i], 1) )
				numReached = numReached + TraverseFrom(EndPoints.Path[i], moveFlags);
	}
	if ( numReached == 0 )
		return 0;

	//pick a random path node from among those reachable
	Actor = MyLevel->GetLevelInfo()->NavigationPointList;
	bestPath = NULL;
	while (Actor)
	{
		if (((ANavigationPoint *)Actor)->bEndPoint)
		{
			result = 1;
			bestPath = Actor;
			if (appFrand() * (FLOAT)numReached <= 1.0)
				break; //quit with current path
			numReached -= 1;
		}
		Actor = ((ANavigationPoint *)Actor)->nextNavigationPoint;
	}
	return result;
	unguard;
}

/* TraverseFrom()
traverse the graph, marking all reached paths.  Return number of new paths marked
*/

int APawn::TraverseFrom(AActor *start, int moveFlags)
{
	guard(APawn::TraverseFrom);
	int numMarked = 1; //mark the one we're at
	ANavigationPoint *node = (ANavigationPoint *)start;

	int iRadius = (int)CollisionRadius;
	int iHeight = (int)CollisionHeight;
	node->bEndPoint = 1;
	FReachSpec *spec;
	ULevel *MyLevel = GetLevel();
	int i = 0;
	while (i<16)
	{
		if (node->Paths[i] == -1)
			i = 16;
		else
		{
			spec = &MyLevel->ReachSpecs(node->Paths[i]);
			ANavigationPoint *nextNode = spec->End->IsA(ANavigationPoint::StaticClass()) ? (ANavigationPoint *)spec->End : NULL;

			if (nextNode && !nextNode->bEndPoint && (!nextNode->bPlayerOnly || bIsPlayer) )
				if (spec->supports(iRadius, iHeight, moveFlags))
					numMarked = numMarked + TraverseFrom(nextNode, moveFlags);
			i++;
		}
	}
	return numMarked;
	unguard;
}

/* breadthPathFrom()
Breadth First Search through navigation network
startnode is the starting path
end when we find a pathnode marked as an end point (bEndpoint == 1)
*/
int APawn::breadthPathFrom(AActor *start, AActor *&bestPath, int bSinglePath, int moveFlags)
{
	guard(APawn::breadthPathFrom);
	//FIXME - perhaps track and bound number of edges to search or max depth?
	ANavigationPoint* currentnode = (ANavigationPoint *)start;
	ANavigationPoint* nextnode;
	ANavigationPoint* BinTree = currentnode;

	int iRadius = (int)CollisionRadius;
	int iHeight = (int)CollisionHeight;
	int p = 0;
	int n = 0;
	int realSplit = 1;
	FReachSpec *spec;
	while ( currentnode )
	{
		if ( currentnode->bEndPoint )
		{
			//debugf("best path is %s", currentnode->GetName());
			((ANavigationPoint *)start)->previousPath = NULL;
			bestPath = currentnode;
			return 1;
		}
		if ( (!currentnode->bPlayerOnly || bIsPlayer) || (currentnode == start) )
		{
			int i = 0;
			while ( i<16 )
			{
				if (currentnode->upstreamPaths[i] == -1)
					i = 16;
				else
				{
					spec = &GetLevel()->ReachSpecs(currentnode->upstreamPaths[i]);
					//debugf("check path from %s to %s with %d, %d",spec->Start->GetName(), spec->End->GetName(), spec->CollisionRadius, spec->CollisionHeight);
					if (spec->supports(iRadius, iHeight, moveFlags))
					{
						ANavigationPoint* startnode = (ANavigationPoint* )spec->Start;
						int nextweight = spec->distance + startnode->cost;
						int newVisit = nextweight + currentnode->visitedWeight + startnode->bEndPoint * startnode->bestPathWeight; 
						//debugf("Path from %s to %s costs %d total %d versus %d", spec->Start->GetName(), spec->End->GetName(), nextweight, newVisit, startnode->visitedWeight);
						if ( startnode->visitedWeight > newVisit )
						{
							if ( startnode->prevOrdered ) //remove from old position
							{
								startnode->prevOrdered->nextOrdered = startnode->nextOrdered;
								if (startnode->nextOrdered)
									startnode->nextOrdered->prevOrdered = startnode->prevOrdered;
								if ( BinTree == startnode )
								{
									if ( startnode->prevOrdered->visitedWeight > newVisit )
										BinTree = startnode->prevOrdered;
								}
								else if ( (startnode->visitedWeight > BinTree->visitedWeight)
									&& (newVisit < BinTree->visitedWeight) )
									realSplit--;	
							}
							else if ( newVisit > BinTree->visitedWeight )
								realSplit++;
							else
								realSplit--;
							//debugf("find spot for %s with BinTree = %s",startnode->GetName(), BinTree->GetName());
							startnode->previousPath = currentnode;
							startnode->visitedWeight = newVisit;
							if (  BinTree->visitedWeight < startnode->visitedWeight )
								nextnode = BinTree;
							else
								nextnode = currentnode;
							//debugf(" start at %s with %d to place %s with %d",nextnode->GetName(),nextnode->visitedWeight, startnode->GetName(), startnode->visitedWeight);
							int numList = 0; //TEMP FIXME
							while ( nextnode->nextOrdered && (nextnode->nextOrdered->visitedWeight < startnode->visitedWeight) )
							{
								numList++;
								if ( numList > 500 )
								{
									debugf( TEXT("Breadth path list overflow from %s"), start->GetName() );
									return 0;
								}
								nextnode = nextnode->nextOrdered;
							}
							if (nextnode->nextOrdered != startnode)
							{
								if (nextnode->nextOrdered)
									nextnode->nextOrdered->prevOrdered = startnode;
								startnode->nextOrdered = nextnode->nextOrdered;
								nextnode->nextOrdered = startnode;
								startnode->prevOrdered = nextnode;
							}
							//if (startnode->nextOrdered)
							//	debugf(" place %s after %s before %s ", startnode->GetName(), nextnode->GetName(), startnode->nextOrdered->GetName());
							//else
							//	debugf(" place %s after %s at end ", startnode->GetName(), nextnode->GetName());
						}
					}
					i++;
				}
			}
			realSplit++;
			int move = (int)(0.5 * realSplit);
			while ( p < move )
			{
				p++;
				if (BinTree->nextOrdered)
					BinTree = BinTree->nextOrdered;
			}
		}
		n++;
		if ( bSinglePath && ( n > 4) )
			return 0;
		if ( n > 1000 )
		{
			debugf(NAME_DevPath, TEXT("1000 navigation nodes searched from %s!"), start->GetName() );
			return 0;
		}
		//debugf("Done with %s",currentnode->GetName());
		currentnode = currentnode->nextOrdered;
		/*
		nextnode = currentnode;
		while (nextnode)
		{
			debugf("Next %s",nextnode->GetName());
			nextnode = nextnode->nextOrdered;
		}
		*/
	}
	//debugf("No path found");
	return 0;
	
	unguard;
}

inline void FSortedPathList::addPath(AActor * node, INT dist)
{
	guard(FSortedPathList::addPath);
	int n=0; 
	if ( numPoints > 8 )
	{
		if ( dist > Dist[numPoints/2] )
		{
			n = numPoints/2;
			if ( (numPoints > 16) && (dist > Dist[n + numPoints/4]) )
				n += numPoints/4;
		}
		else if ( (numPoints > 16) && (dist > Dist[numPoints/4]) )
			n = numPoints/4;
	}

	while ((n < numPoints) && (dist > Dist[n]))
		n++;

	if (n < MAXSORTED)
	{
		AActor *nextPath = Path[n];
		FLOAT nextDist = Dist[n];
		Path[n] = node;
		Dist[n] = dist;
		if (numPoints < MAXSORTED)
			numPoints++;
		n++;
		while (n < numPoints) 
		{
			AActor *afterPath = Path[n];
			FLOAT afterDist = Dist[n];
			Path[n] = nextPath;
			Dist[n] = (INT) nextDist;
			nextPath = afterPath;
			nextDist = afterDist;
			n++;
		}
	}
	unguard;
}

inline void FSortedPathList::removePath(int p)
{
	guard(FSortedPathList::removePath);

	for ( int n=p;n<numPoints-1;n++ )
	{
		Path[n] = Path[n+1];
		Dist[n] = Dist[n+1];
	}
	numPoints--;
	unguard;
}

/* breadthPathToInventory()
Breadth First Search through navigation network
starting from path bot is on.
When encounter inventoryspot, query its item's botdesireability
keep track of best weight and the nextpath associated with it
*/
FLOAT APawn::breadthPathToInventory(AActor *start, AActor *&bestPath, int moveFlags, FLOAT bestInventoryWeight, INT bPredictRespawns)
{
	guard(APawn::breadthPathToInventory);

	ANavigationPoint* currentnode = (ANavigationPoint *)start;
	ANavigationPoint* nextnode;
	ANavigationPoint* BinTree = currentnode;
	ANavigationPoint* BestDest = NULL;

	int iRadius = (int)CollisionRadius;
	int iHeight = (int)CollisionHeight;
	int p = 0;
	int n = 0;
	int realSplit = 1;

	FReachSpec *spec;
	while ( currentnode )
	{
		//debugf(NAME_DevPath,"Distance to %s is %d", currentnode->GetName(), currentnode->visitedWeight);
		if ( currentnode->bEndPoint )
		{
			//debugf("start path is %s",currentnode->GetName());
			currentnode->startPath = currentnode;
			currentnode->previousPath = NULL;
		}

		if ( currentnode->IsA(AInventorySpot::StaticClass()) )
		{
			AInventory* item = ((AInventorySpot *)currentnode)->markedItem;
			//if ( item )
			//	debugf(NAME_DevPath,"looking at %s with weight %f (dist %d) (and touch %d with latent %f)", item->GetName(), item->eventBotDesireability(this)/currentnode->visitedWeight, currentnode->visitedWeight, item->IsProbing(NAME_Touch), item->LatentFloat );
			if ( item && (item->IsProbing(NAME_Touch) || (bPredictRespawns && (item->LatentFloat < 5.0))) 
					&& (item->MaxDesireability/currentnode->visitedWeight > bestInventoryWeight) )
			{
				FLOAT thisItemWeight = item->eventBotDesireability(this)/currentnode->visitedWeight;
				// debugf(NAME_DevPath,"looking at %s with weight %f (dist %d) (and touch %d with latent %f)", item->GetName(), thisItemWeight, currentnode->visitedWeight, item->IsProbing(NAME_Touch), item->LatentFloat );
				if ( thisItemWeight > bestInventoryWeight )
				{
					bestInventoryWeight = thisItemWeight;
					bestPath = currentnode->startPath;
					BestDest = currentnode;
				}
			} 
		}

		int i = 0;
		int nextweight = 0;
		while ( i<16 )
		{
			ANavigationPoint* endnode = NULL;
			if ( currentnode->Paths[i] == -1 )
				i = 16;
			else
			{
				spec = &GetLevel()->ReachSpecs(currentnode->Paths[i]);
				//debugf(NAME_DevPath,"check path from %s to %s with %d, %d",spec->Start->GetName(), spec->End->GetName(), spec->CollisionRadius, spec->CollisionHeight);
				if (spec->supports(iRadius, iHeight, moveFlags))
				{
					endnode = (ANavigationPoint* )spec->End;
					nextweight = spec->distance;
				}
				i++;
			}
			if ( endnode )
			{
				nextweight += endnode->cost;
				int newVisit = nextweight + currentnode->visitedWeight; // + endnode->bEndPoint * endnode->bestPathWeight; 
				//debugf(NAME_DevPath,"Path from %s to %s costs %d total %d", spec->Start->GetName(), spec->End->GetName(), nextweight, newVisit);
				if ( endnode->visitedWeight > newVisit )
				{
					endnode->startPath = currentnode->startPath;
					endnode->previousPath = currentnode;
					if ( endnode->prevOrdered ) //remove from old position
					{
						endnode->prevOrdered->nextOrdered = endnode->nextOrdered;
						if (endnode->nextOrdered)
							endnode->nextOrdered->prevOrdered = endnode->prevOrdered;
						if ( BinTree == endnode )
						{
							if ( endnode->prevOrdered->visitedWeight > newVisit )
								BinTree = endnode->prevOrdered;
						}
						else if ( (endnode->visitedWeight > BinTree->visitedWeight)
							&& (newVisit < BinTree->visitedWeight) )
							realSplit--;	
					}
					else if ( newVisit > BinTree->visitedWeight )
						realSplit++;
					else
						realSplit--;
					//debugf(NAME_DevPath,"find spot for %s with BinTree = %s",endnode->GetName(), BinTree->GetName());
					endnode->visitedWeight = newVisit;
					if (  BinTree->visitedWeight < endnode->visitedWeight )
						nextnode = BinTree;
					else
						nextnode = currentnode;
					//debugf(NAME_DevPath," start at %s with %d to place %s with %d",nextnode->GetName(),nextnode->visitedWeight, endnode->GetName(), endnode->visitedWeight);
					int numList = 0; //TEMP FIXME
					while ( nextnode->nextOrdered && (nextnode->nextOrdered->visitedWeight < endnode->visitedWeight) )
					{
						numList++;
						if ( numList > 500 )
						{
							//debugf("Breadth path list %d", numList);
							return 0;
						}
						nextnode = nextnode->nextOrdered;
					}
					if (nextnode->nextOrdered != endnode)
					{
						if (nextnode->nextOrdered)
							nextnode->nextOrdered->prevOrdered = endnode;
						endnode->nextOrdered = nextnode->nextOrdered;
						nextnode->nextOrdered = endnode;
						endnode->prevOrdered = nextnode;
					}
				}
			}
		}
		realSplit++;
		int move = (int)(0.5 * realSplit);
		while ( p < move )
		{
			p++;
			if (BinTree->nextOrdered)
				BinTree = BinTree->nextOrdered;
		}

		n++;
		if ( n > 250 )
		{
			// debugf("exceeded 250 nodes!");
			if ( bestInventoryWeight > 0 )
			{
				ReverseRouteFor(BestDest);
				return bestInventoryWeight;
			}
			else
				n = 200;
		}

		//}
		//debugf(NAME_DevPath,"Done with %s",currentnode->GetName());
		currentnode = currentnode->nextOrdered;
	}
	ReverseRouteFor(BestDest);
	return bestInventoryWeight;
	
	unguard;
}

/* SetRouteCache() puts the first 16 navigationpoints in the best route found in the 
Pawn's RouteCache[].  
*/
void APawn::SetRouteCache(ANavigationPoint *BestPath)
{
	guard(APawn::SetRouteCache);

	for ( int i=0; i<16; i++ )
	{
		RouteCache[i] = BestPath;
		if ( BestPath )
		{
			//debugf("route %s", BestPath->GetName());
			BestPath = BestPath->previousPath;
		}
	}
	//debugf("------------------------------------");

	unguard;
}

/* ReverseRouteFor() changes the direction that the previousPath linked list enumerates the route.
*/
void APawn::ReverseRouteFor(ANavigationPoint *BestDest)
{
	guard(APawn::ReverseRouteFor);

	if ( !BestDest )
		return;
	//debugf("BestDest is %s at %f",BestDest->GetName(), GetLevel()->TimeSeconds);
	ANavigationPoint *RoutePath = BestDest;
	ANavigationPoint *NextRoutePath = NULL;
	ANavigationPoint *LastRoutePath = NULL;

	while ( RoutePath->previousPath )
	{
		//debugf("Reverse from %s",RoutePath->GetName());
		LastRoutePath = RoutePath->previousPath;
		RoutePath->previousPath = NextRoutePath;
		NextRoutePath = RoutePath;
		RoutePath = LastRoutePath;
	}
	RoutePath->previousPath = NextRoutePath;
	unguard;
}
