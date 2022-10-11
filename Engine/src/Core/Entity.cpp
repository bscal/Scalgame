#include "Entity.h"

#include "SMemory.h"

#include <assert.h>

#define DEFAULT_COMPONENT 256
#define COMPONENTS_ARRAY_SIZE DEFAULT_COMPONENT * sizeof(void*)


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
	entityManager->EntityArray.InitializeCap(DEFAULT_COMPONENT);
	entityManager->ComponentMap.KeyHashFunction = EntityHash;
	entityManager->ComponentMap.KeyEqualsFunction = EntityEquals;
	entityManager->ComponentMap.Initialize(DEFAULT_COMPONENT);

	RegisterComponent<Transform2D>(entityManager, Transform2D::ID);
	RegisterComponent<Health>(entityManager, Health::ID);

	assert(entityManager->ComponentMap.Size == NextComponentId);
}

template<typename T>
void RegisterComponent(EntitiesManager* entityManager,
	uint32_t componentId)
{
	SArray componentsArray = {};
	ArrayCreate(32, sizeof(T), &componentsArray);
	entityManager->ComponentMap.Put(&componentId, &componentsArray);
}

Entity* CreateEntity(EntitiesManager* entityManager)
{
	Entity entity = {};
	entity.Components.InitializeCap(MAX_COMPONENTS);
	Scal::MemSet(entity.Components.Memory, 0xff, COMPONENTS_ARRAY_SIZE);
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

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	Entity* entity, Component<T>* component)
{
	component->OwningEntity = entity;

	SArray* components = entityManager->ComponentMap.Get(&component->ID);
	ArrayPush(components, component);
	uint32_t insertedAt = components->Length - 1;
	entity->Components[component->ID] = insertedAt;
	return true;
}

internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex)
{
	SArray* components = entityManager->ComponentMap.Get(&componentId);

	assert(components);
	assert(components->Memory);

	if (components->Length == 0)
	{
		TraceLog(LOG_WARNING, "Deleting a component from a empty list!");
		return false;
	}

	bool movedLasted = ArrayRemoveAt(components, componentIndex);
	if (movedLasted)
	{
		BaseComponent* movedComponent = (BaseComponent*)ArrayPeekAt(components, componentIndex);
		movedComponent->OwningEntity->Components[componentId] = componentIndex;
	}
}

bool RemoveComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId)
{
	assert(entityManager);
	assert(componentId <= MAX_COMPONENTS);

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

	Transform2D transform2D = {};
	transform2D.Position = { 5, 10 };
	transform2D.ZIndex = 1;
	AddComponent(entityManager, entity, &transform2D);

	Health health = {};
	health.MaxHealth = 20;
	health.Health = health.MaxHealth;
	AddComponent(entityManager, entity, &health);

	Transform2D* trans = GetComponent<Transform2D>(
		entityManager, entity, Transform2D::ID);

	RemoveComponent(entityManager, entity, health.ID);

	EntityRemove(entity, entityManager);

}