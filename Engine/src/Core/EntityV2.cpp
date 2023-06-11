#include "EntityV2.h"

global_var EntityManager EntityMgr;

void EntityMgrInitialize(GameApplication* gameApp)
{
	EntityMgr.Entities.Reserve(256);
}

EntityManager* GetEntityMgr()
{
	return &EntityMgr;
}

internal void 
NewEntity(Entity* uid, EntityTypes type, void* entityData)
{
	uid->Id = EntityMgr.NextUid++;
	uid->Type = static_cast<uint32_t>(type);
	EntityMgr.Entities.Insert(&uid->Mask, &entityData);
}

internal void
InitializeCreature(Creature* creature)
{
}

Monster* SpawnMonster()
{
	Monster* res = EntityMgr.Monsters.allocate();
	NewEntity(&res->Uid, EntityTypes::Monster, res);
	InitializeCreature(&res->Creature);
	SASSERT(res);
	return res;
}

void DeleteMonster(Monster* monster)
{
	SASSERT(monster);
	bool wasRemoved = EntityMgr.Entities.Remove(&monster->Uid.Mask);
	if (wasRemoved)
	{
		EntityMgr.Monsters.destroy(monster);
	}
}

void* GetEntity(Entity ent)
{
	if (ent.Mask == 0)
		return &EntityMgr.Player;

	return EntityMgr.Entities.Get(&ent.Mask);
}

bool DoesEntityExist(Entity ent)
{
	return EntityMgr.Entities.Contains(&ent.Mask);
}

TileEntity* SpawnTileEntity(Vector2i pos)
{
	return EntityMgr.TileEntities.InsertKey(&pos);
}