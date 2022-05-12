//=============================================================================
// WayBeacon.
//=============================================================================
class WayBeacon extends Keypoint;

//temporary beacon for serverfind navigation

function PostBeginPlay()
{
	local class<actor> NewClass;

	Super.PostBeginPlay();
	// !BR FIXME: Add mesh.
//	NewClass = class<actor>( DynamicLoadObject( "Unreali.Lamp4", class'Class' ) );
	if( NewClass!=None )
		Mesh = NewClass.Default.Mesh;
}

function touch(actor other)
{
	if (other == owner)
	{
		if ( Owner.IsA('PlayerPawn') )
			PlayerPawn(owner).ShowPath();
		Destroy();
	}
}

defaultproperties
{
     bStatic=False
     bHidden=False
     DrawType=DT_Mesh
     Mesh=Lamp4
     DrawScale=+00000.500000
     AmbientGlow=40
     bCollideActors=True
     LightType=LT_Steady
     LightBrightness=125
     LightSaturation=125
     LifeSpan=+00006.000000
	 RemoteRole=ROLE_None
}
