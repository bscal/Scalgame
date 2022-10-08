#include "Entity.h"

#include "SMemory.h"

#include <assert.h>

#define DEFAULT_COMPONENT 256

void InitializeEntitiesManager(EntitiesManager* entityManager)
{
	entityManager->ComponentRegistry.InitializeEx(DEFAULT_COMPONENT, 2);
	entityManager->EntityArray.InitializeEx(DEFAULT_COMPONENT, 2);
	entityManager->ComponentMap.Initialize(DEFAULT_COMPONENT);
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
	
}

internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex)
{
	SList<void*>* components = entityManager->ComponentMap.Get(&componentId);
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

	char* memory = (char*)components->Memory;
	size_t stride = components->Stride;
	size_t dstOffset = stride * (components->Length - 1);
	size_t srcOffset = stride * componentIndex;
	Scal::MemCopy(memory + dstOffset, memory + srcOffset, stride);

	// TODO not sure if this is a good way, Component only contains
	// 1 value uint64_t of OwningEntityId. But its templated to have
	// some additional data. Could this template be remove? But to avoid
	// having another struct to inherit from containing the OwningEntityId
	// We can just cast straight to the uint64_t
	Entity* moveOwningEntity = (Entity*)(memory + (components->Length - 1));
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

	EntityRemove(entity, entityManager);

}