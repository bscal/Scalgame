#include "Actor.h"

#include "Game.h"
#include "SMemory.h"

internal void InternalRemoveEntity(EntityMgr* mgr, uint32_t entityId)
{
	assert(mgr);

	//TODO
	//CreatureCache creatureCache = mgr->EntityMap[entityId];
	//mgr->EntityMap.erase(entityId);
	//if (creatureCache.IsPlayer)
	//{
	//	auto lastCreature = mgr->Players.back();
	//	mgr->EntityMap[lastCreature.Id].Index = creatureCache.Index;
	//	mgr->Players[creatureCache.Index] = lastCreature;
	//	mgr->Players.pop_back();
	//}
	//else
	//{
	//	auto lastCreature = mgr->Creatures.back();
	//	mgr->EntityMap[lastCreature.Id].Index = creatureCache.Index;
	//	mgr->Creatures[creatureCache.Index] = lastCreature;
	//	mgr->Creatures.pop_back();
	//}
}

internal void ProcessRemoveEntities(EntityMgr* mgr)
{
	for (size_t i = 0; i < mgr->EntitiesToRemove->size(); ++i)
	{
		InternalRemoveEntity(mgr, (*mgr->EntitiesToRemove)[i]);
	}
	mgr->EntitiesToRemove->clear();
}

EntityMgr::EntityMgr()
{
	EntityMap = new std::unordered_map<ComponentId, CreatureCache>();
	Players = new std::vector<SPlayer>();
	Creatures = new std::vector<SCreature>();
	EntitiesToRemove = new std::vector<uint32_t>();
	NextEntityId = 0;

	S_LOG_INFO("Entity Manager Initialized!");
}

void EntityMgr::Update(Game* game, float dt)
{
	for (size_t i = 0; i < Players->size(); ++i)
	{
		//Players[i]->Update(game, dt);
	}
	for (size_t i = 0; i < Creatures->size(); ++i)
	{
		//Creatures[i].Update(game, dt);
	}

	ProcessRemoveEntities(this);
}

SPlayer* EntityMgr::GetPlayer(uint32_t id)
{
	for (size_t i = 0; i < Players->size(); ++i)
	{
		auto player = (*Players)[i];
		if (player.Id == id) return &player;
	}
	return nullptr;
}

SCreature* EntityMgr::GetCreature(uint32_t id)
{
	for (size_t i = 0; i < Creatures->size(); ++i)
	{
		if ((*Creatures)[i].Id == id) return &(*Creatures)[i];
	}
	return nullptr;
}

SPlayer& EntityMgr::CreatePlayer()
{
	SPlayer p = {};
	p.Id = NextEntityId++;
	Players->push_back(p);

	size_t index = Players->size() - 1;

	CreatureCache creatureCache = {};
	creatureCache.Index = index;
	creatureCache.IsPlayer = true;
	EntityMap->insert({ p.Id, creatureCache });

	return Players->back();
}

SCreature& EntityMgr::CreatureCreature()
{
	SCreature creature = {};
	creature.Initialize();
	creature.Id = NextEntityId++;
	Creatures->push_back(creature);

	CreatureCache creatureCache;
	creatureCache.Index = Creatures->size() - 1;
	creatureCache.IsPlayer = false;
	EntityMap->insert({ creature.Id, creatureCache });

	return Creatures->back();
}

void EntityMgr::RemoveEntity(uint32_t entityId)
{
	if (entityId == CREATURE_EMPTY_ENTITY_ID)
	{
		S_LOG_ERR("Cannot remove empty entity!");
		return;
	}
	
	auto creature = FindEntity(entityId);
	if (!creature || creature->ShouldRemove)
	{
		S_LOG_WARN("Trying to remove an entity that is "
			"Already marked for removal!");
		return;
	}

	creature->ShouldRemove = true;
	EntitiesToRemove->emplace_back(entityId);
}

SCreature* EntityMgr::FindEntity(uint32_t id)
{
	const auto findResult = EntityMap->find(id);
	if (findResult == EntityMap->end())
		return nullptr;

	const auto creatureCache = findResult->second;
	if (creatureCache.IsPlayer)
	{
		return &(Players->at(creatureCache.Index));
	}
	else
	{
		
		return &(Creatures->at(creatureCache.Index));
	}
}

size_t EntityMgr::TotalEntities() const
{
	return EntityMap->size();
}

bool EntityMgr::IsEmpty(const SCreature& creature) const
{
	return (creature.Id == CREATURE_EMPTY_ENTITY_ID);
}

ComponentMgr::ComponentMgr()
	: Components(new std::unordered_map<ComponentId, SArray>())
{
	Components->max_load_factor(COMPONENT_LOAD_FACTOR);
	Components->reserve(CREATURE_MAX_COMPONENTS);
}

void SCreature::Initialize()
{
	constexpr size_t size = sizeof(uint32_t) * CREATURE_MAX_COMPONENTS;
	Scal::MemSet(&ComponentIndex, CREATURE_EMPTY_COMPONENT, size);
}