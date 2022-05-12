//=============================================================================
// dnTripmineFX_Shrunk_LineExp_Large.       Created by Charlie Wiederhold June 14, 2000
//=============================================================================
class dnTripmineFX_Shrunk_LineExp_Large expands dnTripmineFX_Shrunk;

// Laser trip mine explosion effects.
// Does NOT do damage. 
// Spawns the line explosion effect, part 2!!!
// This does not spawn Natalie Portman or Britney Spears though. I tried.

#exec OBJ LOAD FILE=..\Textures\t_explosionfx.dtx

defaultproperties
{
     DestroyWhenEmptyAfterSpawn=True
     SpawnNumber=6
     SpawnPeriod=0.300000
     PrimeTimeIncrement=0.000000
     MaximumParticles=6
     Lifetime=1.500000
     RelativeLocation=True
     RelativeRotation=True
     InitialVelocity=(X=4.000000,Z=-16.000000)
     MaxVelocityVariance=(X=6.000000,Y=12.000000,Z=12.000000)
     UseZoneGravity=False
     UseZoneVelocity=False
     Textures(0)=Texture't_explosionFx.explosions.X_fi_001'
     DieOnLastFrame=True
     StartDrawScale=0.325000
     EndDrawScale=0.750000
     RotationVariance=65535.000000
     RotationVelocityMaxVariance3d=(Pitch=65535,Yaw=65535,Roll=65535)
     UpdateWhenNotVisible=True
     TriggerAfterSeconds=0.400000
     TriggerType=SPT_Disable
     AlphaEnd=0.000000
     CollisionRadius=0.000000
     CollisionHeight=48.000000
     bIgnoreBList=True
}
