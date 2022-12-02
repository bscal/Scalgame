#pragma once

#include "Core.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/BitArray.h"

#define MAX_COMPONENTS 32
#define EMPTY_COMPONENT uint32_t(UINT32_MAX)

struct GameApplication;

global_var uint32_t NextComponentId;

struct EntityHandle
{
	uint32_t EntityId;
	uint32_t EntityIndex;

	inline bool EntityHandle::operator == (EntityHandle rhs) const
	{
		return EntityId == rhs.EntityId && EntityIndex == rhs.EntityIndex;
	}

	inline bool EntityHandle::operator != (EntityHandle rhs) const
	{
		return !(*this == rhs);
	}
};

struct Entity
{
	EntityHandle Handle;
	size_t ComponentsLength;
	uint32_t Components[MAX_COMPONENTS];

	inline bool Has(uint32_t componentId) 
	{ 
		return Components[componentId] != EMPTY_COMPONENT;
	}
};

struct BaseComponent
{
	EntityHandle OwningEntityHandle;
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
	bool IsEnabled;
};

struct BurnSystem : public System
{
	SArray* Burnables;
	SArray* Healths;
	float CounterTime;
	float CounterIntervalInSec;

	void Initialize(EntitiesManager* manager);

	void Process(EntitiesManager* manager, GameApplication* gameApp);

	void UpdateComponent(EntitiesManager* manager, GameApplication* gameApp,
		EntityHandle entityHandle, Health* health, Burnable* burnable) const;
};

struct ComponentEvent
{
	EntityHandle EntityHandle;
	uint64_t ComponentId;
};

void PostProcessComponents(EntitiesManager* entityManager);

struct ComponentRegisterData
{
	void (*CreateComponent)(BaseComponent* component);
	BaseComponent* (*AllocateComponent)();
	void (*FreeComponent)();
};

struct EntitiesManager
{
	STable<uint32_t, SArray> ComponentMap;
	STable<uint32_t, ComponentRegisterData> ComponentTypes;
	SList<Entity> EntityArray;
	SList<ComponentEvent> ComponentRemoval;

	BurnSystem BurnSystem;

	uint32_t NextEntityId;
};

void InitializeEntitiesManager(EntitiesManager* entityManager);

template<typename T>
void RegisterComponent(EntitiesManager* entityManager, 
	uint32_t componentId);

Entity* CreateEntity(EntitiesManager* entityManager);
void EntityRemove(Entity* entity, EntitiesManager* entityManager);
Entity* GetEntity(EntitiesManager* entityManager, EntityHandle entityHandle);

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, Component<T>* component);

void AddComponentId(EntitiesManager* entityManager,
	EntityHandle entityHandle, uint32_t componentId);

bool RemoveComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, uint32_t componentId);

void* GetComponent(EntitiesManager* entityManager, EntityHandle entHandle,
	uint32_t componentId);

void SystemRemoveComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, uint32_t componentId);

template<typename T>
T* GetComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId);

void UpdateSystems(EntitiesManager* entityManager, GameApplication* gameApp);

void TestEntities(EntitiesManager* entityManager);