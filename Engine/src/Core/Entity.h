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
	const static uint32_t ID;
	const static size_t SIZE;
};

template<typename T>
const uint32_t Component<T>::ID = NextComponentId++;

template<typename T>
const size_t Component<T>::SIZE = sizeof(T);

struct Health : public Component<Health>
{
	uint32_t Health;
	uint32_t MaxHealth;
};

struct EntitiesManager
{
	STable<uint32_t, SList<char*>> ComponentMap;
	SList<Entity> EntityArray;
};

void InitializeEntitiesManager(EntitiesManager* entityManager);

Entity* CreateEntity();

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	Entity* entity, Component<T> component)
{
	entity->Components.SetBit(componentId, true);

	auto componentsMap = entityManager->ComponentMap.Get(&componentId);
	auto componentList = componentsMap->PeekAt(entity->EntityId);

	Scal::MemCopy()
}

void TestEntities(EntitiesManager* entityManager);