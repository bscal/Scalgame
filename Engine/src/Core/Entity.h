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
	uint32_t Components[MAX_COMPONENTS];
	uint32_t EntityId;
	uint32_t EntityIndex;

	inline bool Has(uint32_t componentId) 
	{ 
		return Components[componentId] != EMPTY_COMPONENT;
	}
};

internal bool ComponentDeleteInternal(EntitiesManager* entityManager,
	uint32_t componentId, uint32_t componentIndex);

global_var uint32_t NextComponentId;

template<typename T>
struct ComponentRegisterData
{
	T* (*AllocateComponent)();
	void(*FreeComponent)();
	T(*CreateComponent)();
};

struct BaseComponent
{
	uint32_t OwningEntity;
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
	int Health;
	int MaxHealth;
};

struct Burnable : public Component<Burnable>
{
	float BurnTime;
	uint32_t BurnLevel;
};

struct EntitiesManager;

struct System
{
	SList<uint64_t> Entities;
	bool IsEnabled;

	void Initialize(size_t initialCapacity);
	void Free();
};

struct BurnSystem : public System
{
	float SystemTickCounter;

	void Update(EntitiesManager* manager, GameApplication* gameApp,
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
};

struct ComponentAddEvent
{
	Entity* Entity;
	BaseComponent Component;
	void(*SetComponentValues)(EntitiesManager* entityManager, BaseComponent* component);
	uint32_t ComponentId;
};

struct ComponentEvent
{
	uint32_t EntityId;
	uint32_t ComponentId;
};

void PostProcessComponents(EntitiesManager* entityManager)
{
	for (int i = 0; i < entityManager->ComponentRemoval.Length; ++i)
	{
		auto evt = entityManager->ComponentRemoval[i];
		ComponentDeleteInternal(entityManager, evt.EntityId, evt.ComponentId);
	}

	for (int i = 0; i < entityManager->ComponentAddition.Length; ++i)
	{
		auto evt = entityManager->ComponentAddition[i];
		
		evt.Component.OwningEntity = evt.Entity->EntityId;

		if (evt.SetComponentValues)
			evt.SetComponentValues(entityManager, &evt.Component);

		SArray* components = entityManager->ComponentMap.Get(&evt.ComponentId);
		ArrayPush(components, &evt.Component);
		uint32_t insertedAt = components->Length - 1;
		evt.Entity->Components[evt.ComponentId] = insertedAt;
	}

	Health h = {};

	ComponentAddEvent c = {};
	c.Component = h;
}

struct EntitiesManager
{
	STable<uint32_t, SArray> ComponentMap;
	//std::unordered_map<uint32_t, Component<void*>*> ComponentMap;
	SList<Entity> EntityArray;

	SList<ComponentEvent> ComponentRemoval;
	SList<ComponentAddEvent> ComponentAddition;

	BurnSystem BurnSystem;

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

	for (uint32_t i = 0; i < entityManager->BurnSystem.Entities.Length; ++i)
	{
		uint32_t entityId = entityManager->BurnSystem.Entities[i];
	}

}


void TestEntities(EntitiesManager* entityManager);