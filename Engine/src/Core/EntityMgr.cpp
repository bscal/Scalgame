#include "EntityMgr.h"

#include "Core.h"
#include "Game.h"
#include "SMemory.h"

ComponentMgr::ComponentMgr()
{
	ComponentArray.EnsureSize(CREATURE_MAX_COMPONENTS);

	RegisterComponent<Human>(Human::ID);
}

void EntityMgrInitialize(Game* game)
{
	game->EntityMgr.Players.Reserve(1);
	game->EntityMgr.Creatures.Reserve(ESTIMATED_ENTITIES);

	SLOG_INFO("[ Entity Manager ] Initialized!");
}

void EntityMgrUpdate(EntityMgr* entMgr, Game* game)
{
	PROFILE_BEGIN();

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

	PROFILE_END();
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
	uint32_t id;
	if (entMgr->FreeIds.HasNext())
		id = entMgr->FreeIds.PopValue();
	else
		id = entMgr->NextEntityId++;

	EntityId entity = CREATURE_EMPTY_ENTITY_ID;
	SetEntityId(&entity, id);
	SetEntityType(&entity, type);
	SASSERT(GetEntityId(entity) != CREATURE_EMPTY_ENTITY_ID);
	SASSERT(GetEntityType(entity) != EntityType::UNKNOWN);
	SASSERT(GetEntityType(entity) < EntityType::MAX_TYPES);
	return entity;
}

Player* CreatePlayer(EntityMgr* entMgr, World* world)
{
	EntityId ent = CreateEntityId(entMgr, PLAYER);

	Player* player = entMgr->Players.PushNew();
	player->WorldRef = world;
	player->Id = ent;
	SMemSet(player->ComponentIndex, CREATURE_EMPTY_COMPONENT, sizeof(player->ComponentIndex));

	uint32_t id = GetEntityId(ent);
	entMgr->Entities.EnsureSize(id + 1);
	entMgr->Entities[id] = entMgr->Players.LastIndex();
	SASSERT(player);
	return player;
}

SCreature* CreateCreature(EntityMgr* entMgr, World* world)
{
	EntityId ent = CreateEntityId(entMgr, CREATURE);

	SCreature* creature = entMgr->Creatures.PushNew();
	creature->WorldRef = world;
	creature->Id = ent;
	SMemSet(creature->ComponentIndex, CREATURE_EMPTY_COMPONENT, sizeof(creature->ComponentIndex));

	uint32_t id = GetEntityId(ent);
	entMgr->Entities.EnsureSize(id + 1);
	entMgr->Entities[id] = entMgr->Creatures.LastIndex();
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


internal void
SwapPlayer(EntityMgr* entMgr, uint32_t index)
{
	bool isRemovingLast = (entMgr->Players.Count > 1 &&
		index != entMgr->Players.LastIndex());

	entMgr->Players.RemoveAtFast(index);
	if (isRemovingLast)
	{
		SCreature* swapped = entMgr->Players.PeekAt(index);
		SASSERT(swapped);
		uint32_t swappedId = GetEntityId(swapped->Id);
		entMgr->Entities[swappedId] = index;
	}
}

internal void
SwapCreature(EntityMgr* entMgr, uint32_t index)
{
	bool isRemovingLast = (entMgr->Creatures.Count > 1 && 
		index != entMgr->Creatures.LastIndex());
	
	entMgr->Creatures.RemoveAtFast(index);
	if (isRemovingLast)
	{
		SCreature* swapped = entMgr->Creatures.PeekAt(index);
		SASSERT(swapped);
		uint32_t swappedId = GetEntityId(swapped->Id);
		entMgr->Entities[swappedId] = index;
	}
}

global_var constexpr void (*SwapFuncs[MAX_TYPES])(EntityMgr*, uint32_t) = {
	nullptr,
	SwapPlayer,
	SwapCreature,
	nullptr,
	nullptr,
	nullptr
};

void RemoveEntity(EntityMgr* entMgr, EntityId ent)
{
	// Gets all the entity ids
	uint32_t id = GetEntityId(ent);
	SASSERT(id != CREATURE_EMPTY_ENTITY_ID);
	EntityType type = GetEntityType(ent);
	SASSERT(type != UNKNOWN);
	uint32_t index = entMgr->Entities[id];
	SASSERT(index != CREATURE_EMPTY_ENTITY_ID);

	// Handles removal of object and if there
	// was a swapped element
	auto entitySwapFunc = SwapFuncs[type];
	SASSERT(entitySwapFunc);
	entitySwapFunc(entMgr, index);

	// Frees up id
	entMgr->FreeIds.Push(&id);
	entMgr->Entities[id] = CREATURE_EMPTY_ENTITY_ID;
}