//=============================================================================
// Generic_Wall.
//=============================================================================
class Generic_Wall expands dnMaterial;

defaultproperties
{
     DamageCategoryEffect(0)=(HitEffect=Class'dnParticles.dnBulletWallFX_CementSpawner')
     FootstepSounds(0)=Sound'dnsMaterials.Mud_Squishy.LeatherMud14'
     FootstepSoundsCount=1
     FootstepLandSound=Sound'dnsMaterials.Mud_Squishy.LeatherMud79l'
     bPenetrable=True
}
