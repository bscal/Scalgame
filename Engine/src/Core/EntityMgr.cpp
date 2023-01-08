#include "EntityMgr.h"

#include "Game.h"

internal uint32_t InternalFindEntityIndex(EntityMgr* entMgr, EntityId entId)
{
	const auto& find = entMgr->EntityIdToIndex.find(entId);
	assert(find != entMgr->EntityIdToIndex.end());
	return find->second;
}

ComponentMgr::ComponentMgr()
	: Components()
{
	Components.max_load_factor(COMPONENT_LOAD_FACTOR);
	Components.reserve(CREATURE_MAX_COMPONENTS);

	RegisterComponent<Human>(Human::ID);
}

size_t EntityMgr::TotalEntities() const
{
	return EntityIdToIndex.size();
}

bool EntityMgr::IsEmpty(const SCreature& creature) const
{
	return (creature.Id == CREATURE_EMPTY_ENTITY_ID);
}

void EntityMgrInitialize(Game* game)
{
	game->EntityMgr.NextEntityId = 1; // 0 is empty id
	game->EntityMgr.Players.InitializeCap(1);
	game->EntityMgr.Creatures.InitializeCap(ESTIMATED_ENTITIES);

	SLOG_INFO("[ Entity Manager ] Initialized!");
}


void EntityMgrUpdate(EntityMgr* entMgr, Game* game)
{
	for (size_t i = 0; i < entMgr->Players.Count; ++i)
	{
		auto player = entMgr->Players.PeekAtPtr(i);
		player->UpdatePlayer(game);
		player->Update(game);
	}

	for (size_t i = 0; i < entMgr->Creatures.Count; ++i)
	{
		auto creature = entMgr->Creatures.PeekAtPtr(i);
		if (!creature->IsFrozen) creature->Update(game);
	}

	// Process removal of entities
	while (entMgr->EntitiesToRemove.HasNext())
	{
		EntityId entToRemove = entMgr->EntitiesToRemove.PopValue();
		RemoveEntity(entMgr, entToRemove);
	}
}

Player* GetPlayer(EntityMgr* entMgr, EntityId entId)
{
	uint32_t index = InternalFindEntityIndex(entMgr, entId);
	return entMgr->Players.PeekAtPtr(index);
}

SCreature* GetCreature(EntityMgr* entMgr, EntityId entId)
{
	uint32_t index = InternalFindEntityIndex(entMgr, entId);
	return entMgr->Creatures.PeekAtPtr(index);
}

EntityId CreateEntity(EntityMgr* entMgr, EntityType typeId)
{
	assert(entMgr->NextEntityId != 0);
	EntityId id = entMgr->NextEntityId++;
	id |= (ENTITY_TYPE_MASK & ((uint64_t)(typeId) << ENTITY_TYPE_OFFSET));

	assert(GetEntityType(id) != UNKNOWN);
	assert(GetEntityType(id) < MAX_TYPES);
	return id;
}

void* FindEntity(EntityMgr* entMgr, EntityId entId, uint32_t entIndex)
{
	EntityType typeId = (EntityType)GetEntityType(entId);

	assert(entIndex != CREATURE_EMPTY_ENTITY_ID);

	void* entMemory;
	switch (typeId)
	{
		case PLAYER: entMemory = (void*)&entMgr->Players[entIndex]; break;
		case CREATURE: entMemory = (void*)&entMgr->Creatures[entIndex]; break;
		default: entMemory = NULL; break;
	}

	assert(entMemory);
	return entMemory;
}

void* FindEntityById(EntityMgr* entMgr, EntityId entId)
{
	uint32_t entityId = TryFindEntityIndex(entMgr, entId);
	return FindEntity(entMgr, entId, entityId);
}

uint32_t TryFindEntityIndex(EntityMgr* entMgr, EntityId entId)
{
	if (IsEmptyEntityId(entId)) return CREATURE_EMPTY_ENTITY_ID;
	auto find = entMgr->EntityIdToIndex.find(entId);
	assert(find != entMgr->EntityIdToIndex.end());
	return find->second;
}

internal void 
InsertEntity(EntityMgr* entMgr, EntityId entId, Vector2i chunkPos)
{
	auto& chunkEntityMap = entMgr->ChunkToEntities;
	auto find = chunkEntityMap.find(chunkPos);
	if (find == chunkEntityMap.end())
	{
		auto pair = chunkEntityMap.emplace(std::make_pair(chunkPos, std::vector<EntityId>()));
		pair.first->second.push_back(entId);
	}
	else
	{
		find->second.push_back(entId);
	}
}

void UpdateEntityPosition(EntityId id, Vector2i oldPos, Vector2i newPos)
{
	auto& old = GetGame()->EntityMgr.ChunkToEntities[oldPos];
	for (auto it = old.begin(); it != old.end(); ++it)
	{
		if (*it == id) old.erase(it);
	}

	InsertEntity(&GetGame()->EntityMgr, id, newPos);
}


Player* CreatePlayer(EntityMgr* entMgr, World* world)
{
	EntityId entityId = CreateEntity(entMgr, PLAYER);

	Player player = {};
	player.Id = entityId;
	player.WorldRef = world;
	entMgr->Players.Push(&player);

	uint32_t index = (uint32_t)entMgr->Players.End();
	entMgr->EntityIdToIndex.insert({ entityId, index });
	InsertEntity(entMgr, entityId, player.Transform.ChunkPos);
	
	return entMgr->Players.Last();
}


SCreature* CreateCreature(EntityMgr* entMgr, World* world)
{
	EntityId entityId = CreateEntity(entMgr, CREATURE);

	SCreature creature = {};
	creature.Id = entityId;
	creature.WorldRef = world;
	entMgr->Creatures.Push(&creature);

	uint32_t index = (uint32_t)entMgr->Creatures.End();
	entMgr->EntityIdToIndex.insert({ entityId, index });
	InsertEntity(entMgr, entityId, creature.Transform.ChunkPos);

	return entMgr->Creatures.Last();
}

void RemoveEntity(EntityMgr* entMgr, EntityId entityId)
{
	assert(!IsEmptyEntityId(entityId));
	auto find = entMgr->EntityIdToIndex.find(entityId);
	assert(find != entMgr->EntityIdToIndex.end());

	uint32_t entIndex = find->second;
	EntityId movedEntityId = CREATURE_EMPTY_ENTITY_ID;
	EntityType typeId = (EntityType)GetEntityType(entityId);
	switch (typeId)
	{
		case PLAYER: {
			entMgr->Players.RemoveAtFast(entIndex);
			if (entMgr->Players.Count < entIndex)
				movedEntityId = entMgr->Players[entIndex].Id;
			break;
		}
		case CREATURE: {
			entMgr->Creatures.RemoveAtFast(entIndex);
			if (entMgr->Creatures.Count < entIndex)
				movedEntityId = entMgr->Creatures[entIndex].Id;
			break;
		}
		default: {
			SLOG_ERR("Removed entity did not have a type!");
			assert(false);
			break;
		}
	}

	entMgr->EntityIdToIndex.erase(entityId);

	if (movedEntityId != CREATURE_EMPTY_ENTITY_ID)
	{
		assert(entMgr->EntityIdToIndex.find(movedEntityId) == entMgr->EntityIdToIndex.end());
		entMgr->EntityIdToIndex.at(movedEntityId) = entIndex;
	}
}

void MarkEntityForRemove(EntityMgr* entMgr, EntityId entId)
{
	entMgr->EntitiesToRemove.Push(&entId);
}
