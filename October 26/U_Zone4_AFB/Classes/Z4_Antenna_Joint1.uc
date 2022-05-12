//=============================================================================
// Z4_Antenna_Joint1.					November 9th, 2000 - Charlie Wiederhold
//=============================================================================
class Z4_Antenna_Joint1 expands Z4_Antenna_Base;

#exec OBJ LOAD FILE=..\Textures\m_zone4_afb.dtx
#exec OBJ LOAD FILE=..\Meshes\c_zone4_afb.dmx

defaultproperties
{
     CollisionRadius=120.000000
     CollisionHeight=157.000000
     Mesh=DukeMesh'c_zone4_afb.ant02'
}
