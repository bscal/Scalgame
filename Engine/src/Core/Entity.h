#pragma once

#include "Core.h"
#include "Game.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/BitArray.h"

#include <type_traits>

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

struct BaseComponent
{
	Entity* OwningEntity;
};

template<typename T>
struct Component : public BaseComponent
{
	const static uint32_t ID;
	const static size_t SIZE;
};

template<typename T>
const uint32_t Component<T>::ID = NextComponentId++;

template<typename T>
const size_t Component<T>::SIZE = sizeof(T);

struct GameObject : public Component<GameObject>
{
	Transform Transform;
};

struct Transform2D : public Component<Transform2D>
{
	Vector2 Position;
	int ZIndex;
	float Rotation;
};

struct Health : public Component<Health>
{
	uint32_t Health;
	uint32_t MaxHealth;
};

struct System
{
	SList<uint64_t> Entities;

	void Update(EntitiesManager* manager, GameApplication* gameApp, uint32_t entityId);
};

struct EntitiesManager
{
	STable<uint32_t, SArray> ComponentMap;
	//std::unordered_map<uint32_t, Component<void*>*> ComponentMap;
	SList<Entity> EntityArray;
	SArray Systems;
	uint64_t NextEntityId;
};

void InitializeEntitiesManager(EntitiesManager* entityManager);

template<typename T>
void RegisterComponent(EntitiesManager* entityManager, 
	uint32_t componentId);

Entity* CreateEntity();
void EntityRemove(Entity* entity, EntitiesManager* entityManager);
Entity* GetEntity(uint32_t entityId);

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	Entity* entity, Component<T>* component);

bool RemoveComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId);

template<typename T>
T* GetComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId)
{
	static_assert(!std::is_same<BaseComponent, T>());

	auto componentIndex = entity->Components[componentId];
	if (componentIndex == EMPTY_COMPONENT)
		return nullptr;

	SArray* componentArray = entityManager->ComponentMap.Get(&componentId);
	T component = ((T*)componentArray->Memory)[componentIndex];
	return &component;
}

void AddSystem(EntitiesManager* entityManager, System* system)
{

}

void RemoveSystem(EntitiesManager* entityManager, System* system)
{

}

void ProcessSystems(EntitiesManager* entityManager, GameApplication* gameApp)
{
	for (uint32_t i = 0; i < entityManager->Systems.Length; ++i)
	{
		System* system = (System*)ArrayPeekAt(&entityManager->Systems, i);
		for (uint32_t j = 0; j < system->Entities.Length; ++j)
		{
			system->Update(entityManager, gameApp, j);
		}
	}
}


internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex);

void TestEntities(EntitiesManager* entityManager);