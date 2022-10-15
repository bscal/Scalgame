#include "Entity.h"

#include "SMemory.h"
#include "Game.h"

#include <assert.h>

#define DEFAULT_COMPONENT 256
#define COMPONENTS_ARRAY_SIZE (DEFAULT_COMPONENT * sizeof(void*))

#define EMPTY_ENTITY EntityHandle{ UINT32_MAX, UINT32_MAX }

internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex);

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
	entityManager->EntityArray.InitializeCap(10);
	entityManager->ComponentMap.KeyHashFunction = EntityHash;
	entityManager->ComponentMap.KeyEqualsFunction = EntityEquals;
	entityManager->ComponentMap.Initialize(DEFAULT_COMPONENT);
	entityManager->ComponentRemoval.InitializeCap(64);
	entityManager->ComponentTypes.Initialize(MAX_COMPONENTS);
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
	Entity entity;
	entity.Handle =
	{
		entityManager->NextEntityId++,
		(uint32_t)entityManager->EntityArray.Length
	};
	Scal::MemSet(&entity.Components, EMPTY_COMPONENT, COMPONENTS_ARRAY_SIZE);

	entityManager->EntityArray.Push(&entity);
	return entityManager->EntityArray.Last();
}

void EntityRemove(Entity* entity, EntitiesManager* entityManager)
{
	for (int i = 0; i < MAX_COMPONENTS; ++i)
	{
		uint32_t componentIndex = entity->Components[i];
		if (componentIndex == EMPTY_COMPONENT) continue;
		ComponentDeleteInternal(entityManager, i, componentIndex);
	}

	uint32_t index = entity->Handle.EntityIndex;
	entityManager->EntityArray.RemoveAtFast(index);
	entityManager->EntityArray.Memory[index].Handle.EntityIndex = index;
}

EntityHandle GetEntityHandleById(EntitiesManager* entityManager, uint32_t entityId)
{
	for (uint32_t i = 0; i < entityManager->EntityArray.Length; ++i)
	{
		if (entityManager->EntityArray.Memory[i].Handle.EntityId == entityId)
		{
			return { entityId, i };
		}
	}
	return EMPTY_ENTITY;
}

Entity* GetEntity(EntitiesManager* entityManager, EntityHandle entityHandle)
{
	assert(entityManager);
	assert(entityHandle != EMPTY_ENTITY);
	return &entityManager->EntityArray[entityHandle.EntityIndex];
}

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, Component<T>* component)
{
	assert(entityManager);
	assert(entityHandle != EMPTY_ENTITY);
	assert(component);

	SArray* components = entityManager->ComponentMap.Get(&component->ID);
	ArrayPush(components, component);
	uint32_t insertedAt = components->Length - 1;

	Entity* entity = entityManager->EntityArray.PeekAtPtr(entityHandle.EntityIndex);
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
		Entity* entity = entityManager->EntityArray.PeekAtPtr(
			movedComponent->OwningEntityHandle.EntityIndex);
		entity->Components[componentId] = componentIndex;
	}
}

bool RemoveComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, uint32_t componentId)
{
	assert(entityManager);
	assert(componentId <= MAX_COMPONENTS);

	if (componentId >= MAX_COMPONENTS)
	{
		TraceLog(LOG_ERROR, "ComponentId is > MAX_COMPONENTS");
		return false;
	}
	if (entityHandle == EMPTY_ENTITY)
	{
		TraceLog(LOG_ERROR, "Entity is not valid!");
		return false;
	}

	Entity* entity = entityManager->EntityArray.PeekAtPtr(entityHandle.EntityIndex);
	uint32_t componentIndex = entity->Components[componentId];
	if (componentIndex == EMPTY_COMPONENT) return false;
	ComponentDeleteInternal(entityManager, componentId, componentIndex);
	entity->Components[componentId] = EMPTY_COMPONENT;
	return true;
}

void SystemRemoveComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, uint32_t componentId)
{
	ComponentEvent evt = {};
	evt.EntityHandle = entityHandle;
	evt.ComponentId = componentId;
	entityManager->ComponentRemoval.Push(&evt);
}

void PostProcessComponents(EntitiesManager* entityManager)
{
	for (int i = 0; i < entityManager->ComponentRemoval.Length; ++i)
	{
		auto evt = entityManager->ComponentRemoval[i];
		RemoveComponent(entityManager, evt.EntityHandle, evt.ComponentId);
	}

	entityManager->ComponentRemoval.Clear();
}

void ProcessSystems(EntitiesManager* entityManager, GameApplication* gameApp)
{
	for (uint32_t i = 0; i < entityManager->BurnSystem.Entities.Length; ++i)
	{
		uint32_t entityId = entityManager->BurnSystem.Entities[i];
	}

	PostProcessComponents(entityManager);
}

void TestEntities(EntitiesManager* entityManager)
{
	auto entity = CreateEntity(entityManager);

	Transform2D transform2D = {};
	transform2D.Position = { 5, 10 };
	transform2D.ZIndex = 1;
	AddComponent(entityManager, entity->Handle, &transform2D);

	Health health = {};
	health.MaxHealth = 20;
	health.Health = health.MaxHealth;
	AddComponent(entityManager, entity->Handle, &health);

	Transform2D* trans = GetComponent<Transform2D>(
		entityManager, entity, Transform2D::ID);

	RemoveComponent(entityManager, entity->Handle, health.ID);

	EntityRemove(entity, entityManager);

}

void BurnSystem::Update(EntitiesManager* manager, GameApplication* gameApp,
	uint32_t entityId, Health* health, Burnable* burnable)
{
	SystemTickCounter += gameApp->DeltaTime;
	if (SystemTickCounter > 1.0f)
	{
		SystemTickCounter = 0.0f;

		burnable->BurnTime -= gameApp->DeltaTime;
		if (burnable->BurnTime <= 0)
		{

		}

		health->Health -= 1.0f;
	}
}