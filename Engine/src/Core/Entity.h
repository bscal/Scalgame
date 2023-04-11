#pragma once

#include "Core.h"
#include "Globals.h"
#include "ComponentTypes.h"

#include "Structures/ComponentArray.h"
#include "Structures/SHoodSet.h"
#include "Structures/SLinkedList.h"
#include "Structures/StaticArray.h"

#define ENT_MAX_ENTITIES UINT16_MAX
#define ENT_MAX_COMPONENTS 64

typedef uint32_t Entity;

struct PlayerEntity
{
	TransformComponent Transform;
	Vector2i TilePos;
	uint32_t EntityId;
	bool HasMoved;

	void Update();
	void Move(Vector2 to);
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

struct ComponentMgr
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

		SLOG_INFO("Registered Component: %u", ComponentType::Id);
	}


	inline SList<uint32_t> FindEntities(uint32_t* ids, size_t idsCount) const
	{
		PROFILE_BEGIN();
		SASSERT(idsCount > 1)

		SList<uint32_t> result = {};
		result.Allocator = SAllocator::Temp;

		const ComponentArray<void*>* firstArr = Components[ids[0]];

		uint32_t count = firstArr->Indices.DenseCapacity;
		result.Reserve(count);

		for (uint32_t i = 0; i < count; ++i)
		{
			uint32_t entity = firstArr->Indices.Dense[i];
			uint32_t entityId = GetId(entity);
			bool containsEntity = true;
			for (size_t nextComponentId = 1; nextComponentId < idsCount; ++nextComponentId)
			{
				const ComponentArray<void*>* arr = Components[ids[nextComponentId]];
				containsEntity = entityId < arr->Indices.SparseCapacity
					&& arr->Indices.Sparse[entityId] != SPARE_EMPTY_ID;
				if (!containsEntity) 
					break;
			}

			if (containsEntity)
				result.Push(&entity);
		}
		PROFILE_END();
		return result;
	}

	template<typename ComponentType>
	inline ComponentArray<ComponentType>* GetArray() const
	{
		return (ComponentArray<ComponentType>*)Components[ComponentType::Id];
	}

	template<typename ComponentType>
	inline ComponentType* GetComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];
		SASSERT(componentArray);

		ComponentType* component = componentArray->Get(entity);
		return component;
	}

	template<typename ComponentType>
	inline ComponentType* AddComponent(Entity entity, const ComponentType& component)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];
		SASSERT(componentArray);

		ComponentType* result = componentArray->Add(entity, component);
		SLOG_INFO("Added Component(%u) to Entity(%u,%u)", ComponentType::Id, GetId(entity), GetGen(entity));
		return result;
	}

	template<typename ComponentType>
	inline void RemoveComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];
		SASSERT(componentArray);

		componentArray->Remove(entity);
		SLOG_INFO("Removed Component(%u) to Entity(%u,%u)", ComponentType::Id, GetId(entity), GetGen(entity));
	}

	template<typename ComponentType>
	inline bool HasComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;

		using ArrayType = ComponentArray<ComponentType>;
		ArrayType* componentArray = (ArrayType*)Components[componentId];
		SASSERT(componentArray);

		return componentArray->Contains(entity);
	}

};

void CreatePlayer(EntityMgr* entityMgr, ComponentMgr* componentMgr);
void InitializeEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr);

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
	return true;
}

