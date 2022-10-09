#include "Entity.h"

#include "SMemory.h"

#include <assert.h>

#define DEFAULT_COMPONENT 256

internal uint64_t EntityHash(const uint32_t* key)
{
	return *key;
}

internal bool EntityEquals(const uint32_t* key0, const uint32_t* key1)
{
	return *key0 == *key1;
}

void InitializeEntitiesManager(EntitiesManager* entityManager)
{
	entityManager->ComponentRegistry.InitializeEx(DEFAULT_COMPONENT, 2);
	entityManager->EntityArray.InitializeEx(DEFAULT_COMPONENT, 2);

	entityManager->ComponentMap.KeyHashFunction = EntityHash;
	entityManager->ComponentMap.KeyEqualsFunction = EntityEquals;
	entityManager->ComponentMap.Initialize(DEFAULT_COMPONENT);

	RegisterComponent<Health>(entityManager, Health::ID, Health::SIZE);
}

template<typename T>
void RegisterComponent(EntitiesManager* entityManager,
	uint32_t componentId, size_t size)
{
	ComponentRegisterEntry entry;
	entry.Size = size;
	entityManager->ComponentRegistry.Push(&entry);

	SList<T>* componentList = new SList<T>();
	componentList->InitializeEx(16, 2);
	void* ptr = componentList;
	entityManager->ComponentMap.Put(&componentId, &ptr);
}

Entity* CreateEntity(EntitiesManager* entityManager)
{
	Entity entity = {};
	entity.Components.InitializeEx(MAX_COMPONENTS, 2);
	entity.EntityId = entityManager->NextEntityId++;
	entity.EntityIndex = entityManager->EntityArray.Length;

	entityManager->EntityArray.Push(&entity);
	return entityManager->EntityArray.Last();
}

void EntityRemove(Entity* entity, EntitiesManager* entityManager)
{
	uint64_t index = entity->EntityIndex;

	for (int i = 0; i < entity->Components.Length; ++i)
	{
		uint32_t componentIndex = entity->Components[i];
		if (componentIndex == EMPTY_COMPONENT) continue;
		ComponentDeleteInternal(entityManager, i, componentIndex);
	}

	entity->Components.Free();

	entityManager->EntityArray.RemoveAtFast(index);
	entityManager->EntityArray.Memory[index].EntityIndex = index;
}

Entity* GetEntity(EntitiesManager* entityManager, uint32_t entityId)
{
	return nullptr;
}

internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex)
{
	SList<char>* components = (SList<char>*)entityManager->ComponentMap.Get(&componentId);
	if (!components)
	{
		TraceLog(LOG_ERROR, "Components list does not exist!");
		return false;
	}
	assert(components->Memory);
	if (components->Length == 0)
	{
		TraceLog(LOG_ERROR, "Deleting a component from a empty list!");
		return false;
	}

	//char* memory = (char*)components->Memory;
	size_t stride = components->Stride;
	size_t dstOffset = stride * (components->Length - 1);
	size_t srcOffset = stride * componentIndex;
	Scal::MemCopy(components->Memory + dstOffset, components->Memory + srcOffset, stride);

	// TODO not sure if this is a good way, Component only contains
	// 1 value uint64_t of OwningEntityId. But its templated to have
	// some additional data. Could this template be remove? But to avoid
	// having another struct to inherit from containing the OwningEntityId
	// We can just cast straight to the uint64_t
	Entity* moveOwningEntity = (Entity*)(components->Memory + (components->Length - 1));
	moveOwningEntity->Components[componentId] = componentIndex;
}

bool RemoveComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId)
{
	assert(entityManager);
	if (!entity)
	{
		TraceLog(LOG_ERROR, "Entity is not valid!");
		return false;
	}
	uint32_t componentIndex = entity->Components[componentId];
	if (componentIndex == EMPTY_COMPONENT) return false;
	ComponentDeleteInternal(entityManager, componentId, componentIndex);
	entity->Components[componentId] = EMPTY_COMPONENT;
	return true;
}

void TestEntities(EntitiesManager* entityManager)
{
	auto entity = CreateEntity(entityManager);

	Health health = {};
	health.MaxHealth = 20;
	health.Health = health.MaxHealth;
	AddComponent(entityManager, entity, &health);

	RemoveComponent(entityManager, entity, health.ID);

	EntityRemove(entity, entityManager);

}