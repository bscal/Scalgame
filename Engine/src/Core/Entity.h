#pragma once

#include "Core.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/BitArray.h"

#include <bitset>

#define MAX_COMPONENTS 255

struct Entity
{
	BitArray<MAX_COMPONENTS> Components;
	uint64_t EntityId;
};

global_var uint32_t NextComponentId;

template<typename T>
struct Component
{
	const static uint32_t ID = NextComponentId++;
	const static size_t SIZE = sizeof<T>;
};

struct ComponentManager
{
	STable<uint32_t, SList<void*>> ComponentMap;
};

struct EntityManager
{
	SList<Entity> EntityList;
	STable<uint32_t, Entity*> EntityDataTable;

	uint32_t NextEntityId;
};

constexpr void RegisterEntityTypes();

void EntityManagerInitialize(EntityManager* entityManager);

bool EntityInitialize(Entity* entity);

Entity* FindEntity(EntityManager* entityManager, uint32_t entityId);