#include "Creature.h"

#include "Game.h"
#include "SMemory.h"

namespace Scal
{
namespace Creature
{

internal void InternalRemoveEntity(EntityMgr* mgr, uint32_t entityId)
{
	assert(mgr);
	CreatureCache creatureCache = mgr->EntityMap.at(entityId);
	mgr->EntityMap.erase(entityId);
	if (creatureCache.IsPlayer)
	{
		assert(entityId < mgr->Players.size());
		const auto& lastCreature = mgr->Players.back();
		mgr->EntityMap[lastCreature.Id].Index = creatureCache.Index;
		mgr->Players[creatureCache.Index] = lastCreature;
		mgr->Players.pop_back();
	} 
	else
	{
		assert(entityId < mgr->Creatures.size());
		const auto& lastCreature = mgr->Creatures.back();
		mgr->EntityMap[lastCreature.Id].Index = creatureCache.Index;
		mgr->Creatures[creatureCache.Index] = lastCreature;
		mgr->Creatures.pop_back();
	}
}

internal void ProcessRemoveEntities(EntityMgr* mgr)
{
	for (size_t i = 0; i < mgr->EntitiesToRemove.size(); ++i)
	{
		InternalRemoveEntity(mgr, mgr->EntitiesToRemove[i]);
	}
	mgr->EntitiesToRemove.clear();
}

#define ESTIMATED_ENTITIES 32
EntityMgr::EntityMgr()
	: EntityMap(),
	Players(),
	Creatures(),
	EntitiesToRemove(),
	NextEntityId(0)
{
	EntityMap.reserve(ESTIMATED_ENTITIES);
	Players.reserve(1);
	Creatures.reserve(ESTIMATED_ENTITIES);
	EntitiesToRemove.reserve(8);
	S_LOG_INFO("Entity Manager Initialized!");
}

void EntityMgr::Update(::Game* game, float dt)
{
	for (size_t i = 0; i < Players.size(); ++i)
	{
		Players[i].Update(game, dt);
	}
	for (size_t i = 0; i < Creatures.size(); ++i)
	{
		Creatures[i].Update(game, dt);
	}

	ProcessRemoveEntities(this);
}

SPlayer* EntityMgr::GetPlayer(uint32_t id)
{
	for (size_t i = 0; i < Players.size(); ++i)
	{
		if (Players[i].Id == id) return &Players[i];
	}
	return nullptr;
}

SCreature* EntityMgr::GetCreature(uint32_t id)
{
	for (size_t i = 0; i < Creatures.size(); ++i)
	{
		if (Creatures[i].Id == id) return &Creatures[i];
	}
	return nullptr;
}

SPlayer& EntityMgr::CreatePlayer()
{
	SPlayer p = {};
	p.Id = NextEntityId++;
	Players.push_back(p);

	size_t index = Players.size() - 1;

	CreatureCache creatureCache = {};
	creatureCache.Index = index;
	creatureCache.IsPlayer = true;
	EntityMap.insert({ p.Id, creatureCache });

	return Players.back();
}

SCreature& EntityMgr::CreatureCreature()
{
	SCreature creature = {};
	creature.Initialize();
	creature.Id = NextEntityId++;
	Creatures.push_back(creature);

	CreatureCache creatureCache;
	creatureCache.Index = Creatures.size() - 1;
	creatureCache.IsPlayer = false;
	EntityMap.insert({ creature.Id, creatureCache });

	return Creatures.back();
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
	EntitiesToRemove.emplace_back(entityId);
}

SCreature* EntityMgr::FindEntity(uint32_t id)
{
	const auto findResult = EntityMap.find(id);
	if (findResult == EntityMap.end())
		return nullptr;

	const auto creatureCache = findResult->second;
	if (creatureCache.IsPlayer)
	{
		return &(Players[creatureCache.Index]);
	} else
	{

		return &(Creatures[creatureCache.Index]);
	}
}

size_t EntityMgr::TotalEntities() const
{
	return EntityMap.size();
}

bool EntityMgr::IsEmpty(const SCreature& creature) const
{
	return (creature.Id == CREATURE_EMPTY_ENTITY_ID);
}

ComponentMgr::ComponentMgr()
	: Components()
{
	Components.max_load_factor(COMPONENT_LOAD_FACTOR);
	Components.reserve(CREATURE_MAX_COMPONENTS);
}

void SCreature::Update(::Game* game, float dt)
{

}

void SCreature::Initialize()
{
	constexpr size_t size = sizeof(uint32_t) * CREATURE_MAX_COMPONENTS;
	Scal::MemSet(&ComponentIndex, CREATURE_EMPTY_COMPONENT, size);
}

}
}