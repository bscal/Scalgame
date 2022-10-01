#include "Entity.h"

void EntityManagerInitialize(EntityManager* entityManager)
{
	entityManager->EntityData.Initialize(256);
}

void* GetEntity(EntityManager* entityManager, uint32_t entityId)
{
	void** entityData = entityManager->EntityData.Get(&entityId);
	return *entityData;
}