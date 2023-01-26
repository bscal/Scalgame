#pragma once

#include "Core.h"
#include "Creature.h"
#include "Player.h"

#include "Structures/SList.h"
#include "Structures/SLinkedList.h"

struct Game;
struct World;

// NOTE current 16 bits are not used
// bottom bits are entity id
// top 16 bits are used for entity type
typedef uint64_t EntityId;

#define ENTITY_TYPE_MASK (0xffffffff00ffffff)
#define ENTITY_TYPE_OFFSET (32)

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

constexpr inline uint32_t GetEntityId(EntityId ent)
{
	return static_cast<uint32_t>(ent);
}

constexpr inline EntityType GetEntityType(EntityId ent)
{
	uint8_t type = static_cast<uint8_t>(ent >> ENTITY_TYPE_OFFSET);
	//SASSERT_CONSTEXPR(type > UNKNOWN);
	return (EntityType)type;
}

constexpr inline void SetEntityType(EntityId* ent, EntityType type)
{
	EntityId editedEnt = *ent;
	editedEnt &= ENTITY_TYPE_MASK;
	editedEnt |= (0ULL | (EntityId)(type) << ENTITY_TYPE_OFFSET);
	*ent = editedEnt;
}

constexpr inline bool IsEmptyEntityId(EntityId ent)
{
	return (GetEntityId(ent) == CREATURE_EMPTY_ENTITY_ID);
}

#define IS_TYPE(entityId, type) (GetEntityType(entityId) == static_cast<uint16_t>(type))

#define IS_PLAYER(entityId) (IS_TYPE(entityId, PLAYER))
#define IS_CREATURE(entityId) (IS_TYPE(entityId, CREATURE))
#define IS_TILE(entityId) (IS_TYPE(entityId, TILE))
#define IS_TRIGGER(entityId) (IS_TYPE(entityId, TRIGGER))
#define IS_ITEM(entityId) (IS_TYPE(entityId, ITEM))

struct ComponentMgr
{
	SList<SArray> ComponentArray;

	ComponentMgr();

	template<typename T>
	void RegisterComponent(ComponentId componentId)
	{
		SASSERT(componentId < CREATURE_MAX_COMPONENTS);
		SArray* componentArray = ComponentArray.PeekAt(componentId);
		if (!componentArray->Memory)
		{
			*componentArray = ArrayCreate(16, sizeof(T));
		}
	}

	template<typename T>
	void AddComponent(SCreature* creature, SComponent<T>* component)
	{
		SASSERT(component->ID < CREATURE_MAX_COMPONENTS);
		SArray* componentArray = ComponentArray.PeekAt(T::ID);
		SASSERT(componentArray);
		SASSERT(componentArray->Memory);
		component->EntityId = creature->Id;
		ArrayPush(componentArray, component);
		creature->ComponentIndex[T::ID] = (uint32_t)componentArray->Count - 1;
	}

	template<typename T>
	void RemoveComponent(SCreature* creature,
		ComponentId componentId)
	{
		SASSERT(componentId < CREATURE_MAX_COMPONENTS);
		SArray* componentArray = ComponentArray.PeekAt(componentId);
		SASSERT(componentArray);
		SASSERT(componentArray->Memory);

		if (creature->ComponentIndex[componentId] == CREATURE_EMPTY_COMPONENT)
		{
			SLOG_WARN("Failed removing component from entity that doesnt"
				"have the required component!");
			return;
		}

		uint32_t index = creature->ComponentIndex[componentId];
		creature->ComponentIndex[componentId] = CREATURE_EMPTY_COMPONENT;

		if (ArrayRemoveAt(componentArray, static_cast<uint64_t>(index)))
		{
			SComponent<T>* movedComponent = (SComponent<T>*)ArrayPeekAt(componentArray, index);
			SASSERT(!IsEmptyEntityId(movedComponent->EntityId));
			// Currently only SCreatures support components, Players are creatures
			SCreature* creature = (SCreature*)FindEntity
				(&GetGame()->EntityMgr, movedComponent->EntityId);
			creature->ComponentIndex[componentId] = index;
		}
	}

	template<typename T>
	T* GetComponent(const SCreature* creature,
		ComponentId componentId) const
	{
		SASSERT(creature);
		SASSERT(componentId < CREATURE_MAX_COMPONENTS);
		uint32_t index = creature->ComponentIndex[componentId];
		if (index == CREATURE_EMPTY_COMPONENT) return NULL;
		SArray* componentArray = ComponentArray.PeekAt(componentId);
		SASSERT(componentArray);
		return (T*)ArrayPeekAt(componentArray, (uint64_t)index);
	}
};

// A possible upgrade could be we reuse entity ids, which
// would allow use to store all entity indecies in an array
struct EntityMgr
{
	ComponentMgr ComponentManager;

	SLinkedList<EntityId> EntitiesToRemove;
	SLinkedList<uint64_t> FreeIds;

	SList<uint32_t> Entities;

	SList<Player> Players;
	SList<SCreature> Creatures;

	uint32_t NextEntityId;
	uint32_t ActiveEntities;
	uint32_t TotalEntities;
};

void EntityMgrInitialize(Game* game);

void EntityMgrUpdate(EntityMgr* entMgr, Game* game);

void MarkEntityForRemove(EntityMgr* entMgr, EntityId entId);

EntityId CreateEntityId(EntityMgr* entMgr, EntityType type);

void SpawnEntity(EntityMgr* entMgr, EntityId ent, uint32_t index);

Player* CreatePlayer(EntityMgr* entMgr, World* world);
SCreature* CreateCreature(EntityMgr* entMgr, World* world);

void* FindEntity(EntityMgr* entMgr, EntityId ent);

void RemoveEntity(EntityMgr* entMgr, EntityId ent);

inline void TestEntities()
{
	EntityId ent = 1005;
	SetEntityType(&ent, CREATURE);

	uint32_t id = GetEntityId(ent);
	SASSERT(id == 1005);

	EntityType type = GetEntityType(ent);
	SASSERT(type == CREATURE);
}