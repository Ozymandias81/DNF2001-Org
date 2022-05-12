//=============================================================================
// dnTripmineFX_LineExp_Small.            Created by Charlie Wiederhold June 14, 2000
//=============================================================================
class dnTripmineFX_LineExp_Small expands dnTripmineFX;

// Laser trip mine explosion effects.
// Does NOT do damage. 
// Spawns the line explosion effect.

#exec OBJ LOAD FILE=..\Textures\t_explosionfx.dtx

defaultproperties
{
     DestroyWhenEmptyAfterSpawn=True
     SpawnNumber=16
     SpawnPeriod=0.200000
     PrimeTimeIncrement=0.000000
     MaximumParticles=16
     Lifetime=1.500000
     RelativeLocation=True
     RelativeRotation=True
     InitialVelocity=(X=16.000000,Z=-64.000000)
     MaxVelocityVariance=(X=24.000000,Y=48.000000,Z=48.000000)
     UseZoneGravity=False
     UseZoneVelocity=False
     Textures(0)=Texture't_explosionFx.explosions.X_fi_001'
     DieOnLastFrame=True
     StartDrawScale=0.750000
     EndDrawScale=1.500000
     RotationVariance=65535.000000
     RotationVelocityMaxVariance3d=(Pitch=65535,Yaw=65535,Roll=65535)
     UpdateWhenNotVisible=True
     TriggerAfterSeconds=0.300000
     TriggerType=SPT_Disable
     AlphaEnd=0.000000
     CollisionRadius=0.000000
     CollisionHeight=192.000000
     bIgnoreBList=True
}
