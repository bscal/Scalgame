#pragma once

#include "Core.h"
#include "Creature.h"
#include "Player.h"
#include "SMemory.h"

#include "Structures/SList.h"
#include "Structures/SLinkedList.h"

#include <vector>
#include <unordered_map>

struct Game;
struct World;


// NOTE current 16 bits are not used
// bottom bits are entity id
// top 16 bits are used for entity type
typedef uint64_t EntityId;

#define ENTITY_TYPE_MASK (0xffffffff00ffffff)
#define ENTITY_TYPE_OFFSET (32)

constexpr uint32_t GetEntityId(EntityId ent)
{
	return static_cast<uint32_t>(ent);
}

constexpr uint8_t GetEntityType(EntityId ent)
{
	return static_cast<uint8_t>(ent >> ENTITY_TYPE_OFFSET);
}

constexpr bool IsEmptyEntityId(EntityId ent)
{
	return (GetEntityId(ent) == CREATURE_EMPTY_ENTITY_ID);
}

enum EntityType : uint8_t
{
	UNKNOWN = 0,
	PLAYER,
	CREATURE,
	TILE,
	TRIGGER,
	ITEM,
	MAX_TYPES
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

// A possible upgrade could be we reuse entity ids, which
// would allow use to store all entity indecies in an array
struct EntityMgr
{
	ComponentMgr ComponentManager;

	std::unordered_map<EntityId, uint32_t> EntityIdToIndex;

	SLinkedList<EntityId> EntitiesToRemove;

	SList<Player> Players;
	SList<SCreature> Creatures;

	uint32_t NextEntityId;

	size_t TotalEntities() const;
	bool IsEmpty(const SCreature& entity) const;
};

void EntityMgrInitialize(Game* game);
void EntityMgrUpdate(EntityMgr* entMgr, Game* game);

EntityId CreateEntity(EntityMgr* entMgr, EntityType type);

Player* CreatePlayer(EntityMgr* entMgr, World* world);
SCreature* CreateCreature(EntityMgr* entMgr, World* world);

void RemoveEntity(EntityMgr* entMgr, EntityId entityId);
void MarkEntityForRemove(EntityMgr* entMgr, EntityId entId);

void* FindEntity(EntityMgr* entMgr, EntityId entId, uint32_t entIndex);
// If you know the type you should use the specific FindType function;
void* FindEntityById(EntityMgr* entMgr, EntityId entId);

uint32_t TryFindEntityIndex(EntityMgr* entMgr, EntityId entId);

Player* FindPlayer(EntityMgr* entMgr, EntityId entId);
SCreature* FindCreature(EntityMgr* entMgr, EntityId entId);
