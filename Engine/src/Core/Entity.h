#pragma once

#include "Core.h"

#include "Structures/ComponentArray.h"
#include "Structures/SHoodSet.h"
#include "Structures/SLinkedList.h"
#include "Structures/StaticArray.h"

#define ENT_MAX_ENTITIES UINT16_MAX
#define ENT_MAX_COMPONENTS 64

typedef uint32_t Entity;

#define SetId(entity, id) (entity | (0x00ffffff & id))
#define SetGen(entity, gen) ((entity & 0x00ffffff) | ((uint8_t)gen << 24u))
#define GetId(entity) (entity & 0x00ffffff)
#define GetGen(entity) (uint8_t)((entity & 0xff000000) >> 24u)
#define IncGen(entity) SetGen(entity, GetGen(entity) + 1)


global_var inline uint32_t NextId = 0;

template<typename T>
struct Component
{
	static const uint32_t Id;
};

template<typename T>
const uint32_t Component<T>::Id = NextId++;

struct TransformT : Component<TransformT>
{
	Vector2 Position;
	Vector2 Scale;
	float Rotation;
};

struct Actor : Component<Actor>
{
};

struct Renderable : Component<Renderable>
{
	uint8_t x;
	uint8_t y;
	uint8_t Width;
	uint8_t Height;
};

struct PlayerEntity
{
	uint32_t Entity;
	TransformT Transform;
};

struct EntityMgr
{
	struct EntityStatus
	{
		uint8_t Gen;
		bool IsAlive;
	};

	PlayerEntity Player;
	SList<EntityStatus> Entities;
	SLinkedList<Entity> FreeIds;

	Entity CreateEntity();
	void RemoveEntity(Entity entityId);
	
	bool IsAlive(Entity entityId) const;
};

struct ComponentsManager
{
	SList<ComponentArray<void*>*> Components;

	template<typename ComponentType>
	inline void Register()
	{
		ComponentArray<ComponentType>* componentArray;
		componentArray = (ComponentArray<ComponentType>*)SMemAlloc(sizeof(ComponentArray<ComponentType>));
		componentArray->Initialize(); // Default reserve 1 slot, this is so we never has 
		Components.EnsureSize(ComponentType::Id + 1);
		Components[ComponentType::Id] = (ComponentArray<void*>*)componentArray;
	}

	template<typename ComponentType>
	inline ComponentType* GetComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];

		uint32_t id = GetId(entity);
		ComponentType* component = componentArray->Get(id);
		return component;
	}

	template<typename ComponentType>
	inline void RemoveComponent(Entity id)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];

		uint32_t id = GetId(entity);
		componentArray->Remove(id);
	}

	template<typename ComponentType>
	inline bool HasComponent(Entity id)
	{
		uint32_t componentId = ComponentType::Id;

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];

		uint32_t id = GetId(entity);
		return componentArray->Contains(id);
	}

};

inline bool TestEntityId()
{
	uint32_t ent1 = 0;
	uint32_t ent2 = 0;

	ent1 = SetId(ent1, 5);
	ent1 = SetGen(ent1, 0);
	SASSERT(ent1 == 5);
	SASSERT(GetId(ent1) == 5);

	ent2 = SetId(ent2, 105024);
	ent2 = SetGen(ent2, 255);
	SASSERT(ent2 > 105024);
	SASSERT(GetId(ent2) == 105024);
	SASSERT(GetGen(ent2) == 255);

	ent2 = SetGen(ent2, GetGen(ent2) + 1);
	SASSERT(ent2 == 105024);
	SASSERT(GetId(ent2) == 105024);
	SASSERT(GetGen(ent2) == 0);

	return true;
}

inline bool TestComponents()
{
	ComponentsManager cm = {};

	cm.Register<TransformT>();

	TransformT* transform = cm.GetComponent<TransformT>(0);

	return true;
}

