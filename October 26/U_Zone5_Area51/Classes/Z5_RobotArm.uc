//=============================================================================
// Z5_RobotArm. 						November 10th, 2000 - Charlie Wiederhold
//=============================================================================
class Z5_RobotArm expands Zone5_Area51;

#exec OBJ LOAD FILE=..\meshes\c_zone1_vegas.dmx
#exec OBJ LOAD FILE=..\textures\m_zone1_vegas.dtx

defaultproperties
{
     FragType(0)=Class'dnParticles.dnDebris_Metal1'
     FragType(1)=Class'dnParticles.dnDebris_Sparks1'
     FragType(2)=Class'dnParticles.dnDebrisMesh_Metal1'
     FragType(3)=Class'dnParticles.dnDebrisMesh_Metal1a'
     FragType(4)=Class'dnParticles.dnDebrisMesh_Metal1b'
     DestroyedSound=Sound'a_impact.metal.ImpactMtl07'
     SpawnOnDestroyed(0)=(SpawnClass=Class'dnParticles.dnExplosion3_SElec_Spawner2')
     HealthPrefab=HEALTH_SortaHard
     ItemName="Robot Arm"
     bTakeMomentum=False
     CollisionRadius=41.000000
     CollisionHeight=12.000000
     Mesh=DukeMesh'c_zone5_area51.RobotArm'
}
