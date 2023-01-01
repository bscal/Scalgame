#include "EntityMgr.h"

#include "Game.h"

EntityId CreateEntity(EntityMgr* entMgr, uint16_t typeId)
{
	EntityId id = (EntityId)(entMgr->NextEntityId++);
	id |= ((EntityId)(typeId) << 48);
	return id;
}

Player* CreatePlayer(EntityMgr* entMgr)
{
	EntityId entityId = CreateEntity(entMgr, PLAYER);

	Player* entityPtr = (Player*)MemPoolAlloc(&entMgr->EntityMemory, sizeof(Player));
	assert(entityPtr);

	entMgr->EntityIdToEntityPtr.insert({ entityId, entityPtr });

	entityPtr->Id = entityId;
	return entityPtr;
}

void RemoveEntity(EntityMgr* entMgr, EntityId entityId)
{
	const auto& find = entMgr->EntityIdToEntityPtr.find(entityId);
	assert(find == entMgr->EntityIdToEntityPtr.end());

	MemPoolFree(&entMgr->EntityMemory, find->second);

}

void SpawnCreature(SCreature* creature, World* world, Vector2 spawnCoords)
{
	Vector2i tileCoord = WorldToTileCoord(world, spawnCoords);



	creature->Transform.Pos = tileCoord.AsVec2();
	creature->Transform.TilePos = tileCoord;
}
