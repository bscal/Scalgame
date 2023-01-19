#include "EntityMgr.h"

#include "Game.h"

ComponentMgr::ComponentMgr()
{
	ComponentArray.EnsureSize(CREATURE_MAX_COMPONENTS);

	RegisterComponent<Human>(Human::ID);
}

void EntityMgrInitialize(Game* game)
{
	game->EntityMgr.Players.Resize(1);
	game->EntityMgr.Creatures.Resize(ESTIMATED_ENTITIES);

	SLOG_INFO("[ Entity Manager ] Initialized!");
}

void EntityMgrUpdate(EntityMgr* entMgr, Game* game)
{
	for (size_t i = 0; i < entMgr->Players.Count; ++i)
	{
		auto player = entMgr->Players.PeekAt(i);
		player->UpdatePlayer(game);
		player->Update(game);
	}

	for (size_t i = 0; i < entMgr->Creatures.Count; ++i)
	{
		auto creature = entMgr->Creatures.PeekAt(i);
		if (!creature->IsFrozen) creature->Update(game);
	}

	// Process removal of entities
	while (entMgr->EntitiesToRemove.HasNext())
	{
		EntityId entToRemove = entMgr->EntitiesToRemove.PopValue();
		RemoveEntity(entMgr, entToRemove);
	}
}

void MarkEntityForRemove(EntityMgr* entMgr, EntityId ent)
{
	SASSERT(!IsEmptyEntityId(ent));
	entMgr->EntitiesToRemove.Push(&ent);
	EntityType type = GetEntityType(ent);
	if (type == PLAYER || type == CREATURE)
	{
		SCreature* creature = (SCreature*)FindEntity(entMgr, ent);
		SASSERT(!creature->ShouldRemove);
		creature->ShouldRemove = true;
	}
}

EntityId CreateEntityId(EntityMgr* entMgr, EntityType type)
{
	EntityId id;

	if (entMgr->FreeIds.HasNext())
		id = entMgr->FreeIds.PopValue();
	else
		id = entMgr->NextEntityId++;

	SetEntityType(&id, type);
	return id;
}

void SpawnEntity(EntityMgr* entMgr, EntityId ent, uint32_t index)
{
	SASSERT(!IsEmptyEntityId(ent));

	uint32_t id = GetEntityId(ent);
	entMgr->Entities.EnsureSize(id + 1);
	entMgr->Entities.Set(id, &index);
}

Player* CreatePlayer(EntityMgr* entMgr, World* world)
{
	EntityId ent = CreateEntityId(entMgr, PLAYER);

	Player* player = entMgr->Players.PushZero();
	player->WorldRef = world;
	player->Id = ent;
	SMemSet(player->ComponentIndex, CREATURE_EMPTY_COMPONENT, sizeof(player->ComponentIndex));

	SpawnEntity(entMgr, ent, entMgr->Players.End());
	SASSERT(player);
	return player;
}

SCreature* CreateCreature(EntityMgr* entMgr, World* world)
{
	EntityId ent = CreateEntityId(entMgr, CREATURE);

	SCreature* creature = entMgr->Creatures.PushZero();
	creature->WorldRef = world;
	creature->Id = ent;
	SMemSet(creature->ComponentIndex, CREATURE_EMPTY_COMPONENT, sizeof(creature->ComponentIndex));

	SpawnEntity(entMgr, ent, entMgr->Creatures.End());
	SASSERT(creature);
	return creature;
}

void* FindEntity(EntityMgr* entMgr, EntityId ent)
{
	if (IsEmptyEntityId(ent)) return NULL;
	uint32_t id = GetEntityId(ent);
	EntityType type = GetEntityType(ent);

	uint32_t index = entMgr->Entities[id];

	void* entity;
	switch (type)
	{
		case PLAYER: entity = entMgr->Players.PeekAt(index); break;
		case CREATURE: entity = entMgr->Creatures.PeekAt(index); break;
		default: entity = NULL;
	}
	SASSERT(entity);
	return entity;
}

void RemoveEntity(EntityMgr* entMgr, EntityId ent)
{
	SASSERT(!IsEmptyEntityId(ent));

	uint32_t id = GetEntityId(ent);
	SASSERT(id != CREATURE_EMPTY_ENTITY_ID);
	EntityType type = GetEntityType(ent);
	SASSERT(type != UNKNOWN);

	uint32_t* index = entMgr->Entities.PeekAt(id);
	auto entitySwapFunc = SwapFuncs[type];
	SASSERT(entitySwapFunc);
	entitySwapFunc(entMgr, *index);

	entMgr->FreeIds.Push((uint64_t*)index);
	*index = CREATURE_EMPTY_ENTITY_ID;
}