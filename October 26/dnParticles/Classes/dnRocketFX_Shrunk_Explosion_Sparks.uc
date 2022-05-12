//=============================================================================
// dnRocketFX_Shrunk_Explosion_Sparks. 				  August 8th, 2000 - Charlie Wiederhold
//=============================================================================
class dnRocketFX_Shrunk_Explosion_Sparks expands dnRocketFX_Shrunk;

defaultproperties
{
     Enabled=False
     DestroyWhenEmptyAfterSpawn=True
     AdditionalSpawn(0)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     AdditionalSpawn(1)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     AdditionalSpawn(2)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     AdditionalSpawn(3)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     AdditionalSpawn(4)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     AdditionalSpawn(5)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     AdditionalSpawn(6)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance= 32.000000)
     AdditionalSpawn(7)=(SpawnClass=Class'dnParticles.dnRocketFX_Shrunk_Explosion_SparkStreamer',SpawnRotationVariance=(Pitch=28768,Yaw=65535,Roll=28768),SpawnSpeed=320.000000,SpawnSpeedVariance=32.000000)
     SpawnNumber=0
     SpawnPeriod=0.000000
     PrimeCount=1
     MaximumParticles=1
     Lifetime=1.000000
     InitialVelocity=(Z=0.000000)
     MaxVelocityVariance=(X=0.000000,Y=0.000000)
     BounceElasticity=0.100000
     UseZoneGravity=False
     UseZoneVelocity=False
     ConstantLength=True
     LineStartColor=(R=255,G=255,B=255)
     LineEndColor=(R=255,G=255,B=255)
     LineStartWidth=1.500000
     LineEndWidth=1.500000
     StartDrawScale=0.000000
     EndDrawScale=0.000000
     UpdateWhenNotVisible=True
     TriggerType=SPT_None
     PulseSeconds=4.000000
     AlphaVariance=0.250000
     AlphaEnd=0.000000
     bBurning=True
     bHidden=True
     CollisionRadius=4.000000
     CollisionHeight=4.000000
     Style=STY_Translucent
     bUnlit=True
     bIgnoreBList=True
}
