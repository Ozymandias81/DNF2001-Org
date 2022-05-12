//=============================================================================
// G_Pipe1A.
//===========================================Created Feb 24th, 1999 - Stephen Cole
class G_Pipe1A expands Generic;

#exec OBJ LOAD FILE=..\meshes\c_generic.dmx
#exec OBJ LOAD FILE=..\textures\m_generic.dtx
// September 18th, 2000 - "Nobody expects the Actor Inquisition!" - Charlie Wiederhold

defaultproperties
{
     FragType(0)=Class'dnParticles.dnDebrisMesh_Generic1'
     FragType(1)=Class'dnParticles.dnDebrisMesh_Generic1a'
     FragType(2)=Class'dnParticles.dnDebrisMesh_Generic1b'
     FragType(3)=Class'dnParticles.dnDebris_Smoke'
     FragType(4)=Class'dnParticles.dnDebris_Metal1_Small'
     SpawnOnHit=Class'dnParticles.dnBulletFX_CementSpawner'
     bTakeMomentum=False
     MassPrefab=MASS_Light
     bLandForward=True
     bLandBackwards=True
     bLandLeft=True
     bLandRight=True
     PlayerViewOffset=(X=3.000000,Y=3.000000,Z=5.000000)
     Mesh=DukeMesh'c_generic.pipe1A'
     ItemName="Pipe"
     CollisionRadius=5.000000
     CollisionHeight=33.000000
}
