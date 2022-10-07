#include "Entity.h"

#include "SMemory.h"

#include <assert.h>

void InitializeEntitiesManager(EntitiesManager* entityManager)
{
	entityManager->EntityArray.InitializeEx(256, 2);
	entityManager->ComponentMap.Initialize(256);
}

Entity* CreateEntity(EntitiesManager* entityManager)
{
	Entity entity = {};
	entity.EntityId = entityManager->NextEntityId++;
	entity.EntityIndex = entityManager->EntityArray.Length;

	entityManager->EntityArray.Push(&entity);
	return entityManager->EntityArray.Last();
}

void EntityRemove(Entity* entity, EntitiesManager* entityManager)
{
	uint64_t index = entity->EntityIndex;
	entityManager->EntityArray.RemoveAtFast(index);
	entityManager->EntityArray.Memory[index].EntityIndex = index;
}

void TestEntities(EntitiesManager* entityManager)
{
	auto entity = CreateEntity(entityManager);

	EntityRemove(entity, entityManager);

}