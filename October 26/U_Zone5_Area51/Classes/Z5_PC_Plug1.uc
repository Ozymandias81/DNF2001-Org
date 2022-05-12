//=============================================================================
// Z5_PC_Plug1. 						November 10th, 2000 - Charlie Wiederhold
//=============================================================================
class Z5_PC_Plug1 expands Zone5_Area51;

#exec OBJ LOAD FILE=..\meshes\c_zone1_vegas.dmx
#exec OBJ LOAD FILE=..\textures\m_zone1_vegas.dtx

defaultproperties
{
     HealthPrefab=HEALTH_NeverBreak
     bNotTargetable=True
     bTakeMomentum=False
     CollisionRadius=52.000000
     CollisionHeight=47.000000
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=True
     Mesh=DukeMesh'c_zone5_area51.PC_plug1'
}
