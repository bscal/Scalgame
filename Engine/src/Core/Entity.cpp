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

}

void TestEntities(EntitiesManager* entityManager)
{
	
}