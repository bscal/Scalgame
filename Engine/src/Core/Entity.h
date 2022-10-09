#pragma once

#include "Core.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/BitArray.h"

#include <unordered_map>
#include <bitset>

#define MAX_COMPONENTS 256
#define EMPTY_COMPONENT UINT32_MAX

struct Entity
{
	SList<uint32_t> Components;
	uint64_t EntityId;
	uint64_t EntityIndex;

	inline bool Has(uint32_t componentId) 
	{ 
		return Components[componentId] != EMPTY_COMPONENT;
	}
};

global_var uint32_t NextComponentId;

template<typename T>
struct Component
{
	const static uint32_t ID;
	const static size_t SIZE;
	void* OwningEntity;
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

struct ComponentRegisterEntry
{
	size_t Size;
};

struct ComponentStorage
{
	SList<uint8_t> Memory;
};

struct EntitiesManager
{
	SList<ComponentRegisterEntry> ComponentRegistry;
	STable<uint32_t, void*> ComponentMap;
	//std::unordered_map<uint32_t, Component<void*>*> ComponentMap;
	SList<Entity> EntityArray;
	uint64_t NextEntityId;
};

void InitializeEntitiesManager(EntitiesManager* entityManager);

template<typename T>
void RegisterComponent(EntitiesManager* entityManager, 
	uint32_t componentId, size_t size);

Entity* CreateEntity();
void EntityRemove(Entity* entity, EntitiesManager* entityManager);
Entity* GetEntity(uint32_t entityId);

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	Entity* entity, Component<T>* component)
{
	component->OwningEntity = entity;

	void** list = entityManager->ComponentMap.Get(&component->ID);

	void* c = *list;
	SList<Component<T>>* components = c;
	components->Push(component);

	uint32_t insertedAt = components->Length - 1;
	entity->Components[component->ID] = insertedAt;
	return true;
}

bool RemoveComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId);

internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex);

void TestEntities(EntitiesManager* entityManager);