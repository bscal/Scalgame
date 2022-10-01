#pragma once

#include "Core.h"
#include "Structures/STable.h"

struct Entity
{
	uint32_t EntityId;
};

struct EntityManager
{
	uint32_t NextEntityId;
	uint32_t TotalLiveEntities;

	STable<uint32_t, void*> EntityData;
};

void EntityManagerInitialize(EntityManager* entityManager);

void* GetEntity(EntityManager* entityManager, uint32_t entityId);