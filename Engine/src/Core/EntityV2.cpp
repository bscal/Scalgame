#include "EntityV2.h"

global_var EntityManager EntityMgr;

EntityManager* GetEntityMgr()
{
	return &EntityMgr;
}

NPC* SpawnNPC()
{
	UID uid;
	if (EntityMgr.UnusedNpcId.Count > 0)
	{
		EntityMgr.UnusedNpcId.Pop(&uid);
	}
	else
	{
		uid.Id = EntityMgr.NextNpcId++;
		uid.Generation = 0;
		uid.Type = (uint32_t)EntityTypes::Npc;
	}

	NPC* npc = EntityMgr.Npcs.Add(uid.Packed, NPC{});
	return npc;
}

Monster* SpawnMonster();
TileEntity* SpawnTileEntity();