#pragma once

#include "Core.h"
#include "Player.h"
#include "ComponentTypes.h"

#include "Structures/SArray.h"
#include "Structures/ComponentArray.h"
#include "Structures/SLinkedList.h"

#include <typeinfo>

#define ENT_MAX_ENTITIES UINT16_MAX
#define ENT_MAX_COMPONENTS 64
#define ENT_NOT_FOUND UINT32_MAX

struct EntityMgr
{
	struct EntityStatus
	{
		uint8_t Gen;
		bool IsAlive;
	};

	PlayerEntity Player;
	SList<EntityStatus> Entities;
	SLinkedList<uint32_t> FreeIds;

	uint32_t CreateEntity();
	void RemoveEntity(uint32_t entity);
	
	uint32_t FindGen(uint32_t entityId);
	bool IsAlive(uint32_t entity) const;
};

struct ComponentQuery
{
	uint32_t ComponentIds[4];

	inline bool operator==(const ComponentQuery& other) const
	{
		return (ComponentIds[0] == other.ComponentIds[0]
			&& ComponentIds[1] == other.ComponentIds[1]
			&& ComponentIds[2] == other.ComponentIds[2]
			&& ComponentIds[3] == other.ComponentIds[3]);
	}

	inline bool operator!=(const ComponentQuery& other) const
	{
		return (ComponentIds[0] != other.ComponentIds[0]
			&& ComponentIds[1] != other.ComponentIds[1]
			&& ComponentIds[2] != other.ComponentIds[2]
			&& ComponentIds[3] != other.ComponentIds[3]);
	}
};

struct ComponentMgr
{
	SList<ComponentArray> Components;
	SList<TransformComponent> Transforms;

	template<typename ComponentType>
	inline void Register()
	{
		RegisterWithCallback<ComponentType>(nullptr, nullptr);
	}

	template<typename ComponentType>
	inline void RegisterWithCallback(OnAdd addCallback, OnRemove removeCallback)
	{
		uint32_t componentId = ComponentType::Id;
		Components.EnsureSize(componentId + 1);
		Components[componentId].Initialize(1, sizeof(ComponentType), addCallback, removeCallback);

		#if SCAL_DEBUG
		const std::type_info& typeInfo = typeid(ComponentType);
		SLOG_INFO("Registered Component: %u, Name: %s", componentId, typeInfo.name());
		#endif
	}

	inline SList<uint32_t> FindEntities(uint32_t* ids, size_t idsCount) const
	{
		PROFILE_BEGIN();
		SASSERT(idsCount > 1)

		SList<uint32_t> result = {};
		result.Allocator = SAllocator::Temp;

		const ComponentArray& firstArr = Components[ids[0]];

		uint32_t count = firstArr.Indices.DenseCapacity;
		result.Reserve(count);

		for (uint32_t i = 0; i < count; ++i)
		{
			uint32_t entity = firstArr.Indices.Dense[i];
			uint32_t entityId = GetId(entity);
			bool containsEntity = true;
			for (size_t nextComponentId = 1; nextComponentId < idsCount; ++nextComponentId)
			{
				const ComponentArray& arr = Components[ids[nextComponentId]];
				containsEntity = entityId < arr.Indices.SparseCapacity
					&& arr.Indices.Sparse[entityId] != SPARSE_EMPTY_ID;
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
	inline ComponentArray* GetArray()
	{
		return &Components[ComponentType::Id];
	}

	template<typename ComponentType>
	inline ComponentType* GetComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);
		return Components[componentId].Get<ComponentType>(entity);
	}

	template<typename ComponentType>
	inline ComponentType* AddComponent(Entity entity, const ComponentType& component)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		ComponentArray& componentArray = Components[componentId];

		ComponentType* result = componentArray.Add(entity, component);

		#if SCAL_DEBUG
		const std::type_info& typeInfo = typeid(ComponentType);
		SLOG_INFO("Added Component(%u, %s) to Entity(%u,%u)", ComponentType::Id, typeInfo.name(), GetId(entity), GetGen(entity));
		#endif

		return result;
	}

	template<typename ComponentType>
	inline void RemoveComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;
		SASSERT(componentId < Components.Count);

		ComponentArray& componentArray = Components[componentId];
		SASSERT(componentArray);

		componentArray.Remove(entity);

		#if SCAL_DEBUG
		const std::type_info& typeInfo = typeid(ComponentType);
		SLOG_INFO("Removed Component(%u, %s) to Entity(%u,%u)", ComponentType::Id, typeInfo.name(), GetId(entity), GetGen(entity));
		#endif
	}

	template<typename ComponentType>
	inline bool HasComponent(Entity entity)
	{
		uint32_t componentId = ComponentType::Id;

		const ComponentArray& componentArray = Components[componentId];
		SASSERT(componentArray);

		return componentArray.Contains(entity);
	}

	template<>
	inline TransformComponent* GetComponent(uint32_t entity)
	{
		uint32_t entityId = GetId(entity);
		SASSERT(entityId < Components.Count);
		return &Transforms[entityId];
	}

	template<>
	inline TransformComponent* AddComponent(uint32_t entity, const TransformComponent& component)
	{

		uint32_t entityId = GetId(entity);
		Transforms.EnsureSize(entityId + 1);
		SASSERT(entityId < Components.Count);
		Transforms[entityId] = component;
		return &Transforms[entityId];
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

