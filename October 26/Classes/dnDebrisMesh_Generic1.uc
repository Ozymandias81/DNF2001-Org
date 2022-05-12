//=============================================================================
// dnDebrisMesh_Generic1.  			  September 21st, 2000 - Charlie Wiederhold
//=============================================================================
class dnDebrisMesh_Generic1 expands dnDebris;

// Root of the generic mesh debris spawners. Normal sized chunks

#exec OBJ LOAD FILE=..\Meshes\c_FX.dmx

defaultproperties
{
     Enabled=False
     DestroyWhenEmpty=True
     SpawnNumber=0
     PrimeCount=3
     MaximumParticles=3
     Lifetime=5.000000
     LifetimeVariance=2.000000
     InitialVelocity=(Z=318.000000)
     MaxVelocityVariance=(X=512.000000,Y=512.000000,Z=318.000000)
     LocalFriction=256.000000
     BounceElasticity=0.500000
     Bounce=True
     ParticlesCollideWithWorld=True
     DrawScaleVariance=0.500000
     RotationVariance3d=(Pitch=65535,Yaw=65535,Roll=65535)
     TriggerType=SPT_None
     LodMode=LOD_Disabled
     CollisionRadius=1.000000
     CollisionHeight=1.000000
     DrawType=DT_Mesh
     Mesh=DukeMesh'c_FX.Gib_GenericA'
     LightDetail=LTD_NormalNoSpecular
}
