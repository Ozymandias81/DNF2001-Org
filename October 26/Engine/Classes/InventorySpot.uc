//=============================================================================
// InventorySpot.
//=============================================================================
class InventorySpot extends NavigationPoint
	native;

var Inventory markedItem;

defaultproperties
{
     CollisionRadius=+00020.000000
     CollisionHeight=+00040.000000
     bCollideWhenPlacing=False
	 bHiddenEd=true
	 bEndPointOnly=true
}
