#pragma once

#include "Core.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/BitArray.h"

#include <type_traits>

#define MAX_COMPONENTS 256
#define EMPTY_COMPONENT UINT32_MAX

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
		uint32_t entityId, Health* health, Burnable* burnable);
};

struct ComponentEvent
{
	uint32_t EntityId;
	uint32_t ComponentId;
};

void PostProcessComponents(EntitiesManager* entityManager);

struct ComponentRegisterData
{
	void(*CreateComponent)(BaseComponent* component);
	void(*FreeComponent)();
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

Entity* CreateEntity();
void EntityRemove(Entity* entity, EntitiesManager* entityManager);
Entity* GetEntity(uint32_t entityId);

template<typename T>
bool AddComponent(EntitiesManager* entityManager,
	EntityHandle entityHandle, Component<T>* component);

bool RemoveComponent(EntitiesManager* entityManager,
	Entity* entity, uint32_t componentId);

void SystemRemoveComponent(EntitiesManager* entityManager,
	uint32_t entity, uint32_t componentId);

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

void ProcessSystems(EntitiesManager* entityManager, GameApplication* gameApp);

void TestEntities(EntitiesManager* entityManager);