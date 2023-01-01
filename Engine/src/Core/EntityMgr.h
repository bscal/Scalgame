#pragma once

#include "Core.h"
#include "Creature.h"
#include "Player.h"
#include "SMemory.h"

#include <vector>
#include <unordered_map>

struct Game;
struct World;


// NOTE current 16 bits are not used
// bottom bits are entity id
// top 16 bits are used for entity type
typedef uint64_t EntityId;

constexpr uint32_t GetEntityId(uint64_t val)
{
	return static_cast<uint32_t>(val);
}

constexpr uint16_t GetEntityType(uint64_t val)
{
	return static_cast<uint16_t>(val >> 48);
}

enum EntityType : uint16_t
{
	UNKNOWN		= 0,
	PLAYER		= 1,
	CREATURE	= 2,
	TILE		= 4,
	TRIGGER		= 8,
	ITEM		= 16
};

#define IS_TYPE(entityId, type) (GetEntityType(entityId) == static_cast<uint16_t>(type))

#define IS_PLAYER(entityId) (IS_TYPE(entityId, PLAYER))
#define IS_CREATURE(entityId) (IS_TYPE(entityId, CREATURE))
#define IS_TILE(entityId) (IS_TYPE(entityId, TILE))
#define IS_TRIGGER(entityId) (IS_TYPE(entityId, TRIGGER))
#define IS_ITEM(entityId) (IS_TYPE(entityId, ITEM))

struct CreatureCache
{
	uint32_t Index;
	bool IsPlayer;
};

struct EntityMgr
{
	std::unordered_map<uint32_t, CreatureCache,
		std::hash<uint32_t>, std::equal_to<uint32_t>,
		SAllocator<std::pair<const uint32_t, CreatureCache>>> EntityMap;
	std::vector<Player, SAllocator<Player>> Players;
	std::vector<SCreature, SAllocator<SCreature>> Creatures;
	std::vector<uint32_t, SAllocator<uint32_t>> EntitiesToRemove;
	ComponentMgr ComponentManager;

	MemPool EntityMemory;
	std::unordered_map<EntityId, void*> EntityIdToEntityPtr;

	uint32_t NextEntityId;

	void Initialize(Game* game);
	void Update(Game* game);
	void RemoveEntity(uint32_t id);

	Player& CreatePlayer(World* world);
	SCreature& CreatureCreature(World* world);

	Player* GetPlayer(uint32_t id);
	SCreature* GetCreature(uint32_t id);
	SCreature* FindEntity(uint32_t id);

	size_t TotalEntities() const;
	bool IsEmpty(const SCreature& entity) const;
};

EntityId CreateEntity(EntityMgr* entMgr, uint16_t typeId);
Player* CreatePlayer(EntityMgr* entMgr);
void RemoveEntity(EntityMgr* entMgr, EntityId entityId);
void SpawnCreature(SCreature* creature, World* world, Vector2 spawnCoords);
