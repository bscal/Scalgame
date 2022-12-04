#pragma once

#include "Core.h"
#include "Structures/SArray.h"
#include "SUtil.h"
#include "Sprite.h"

#include <assert.h>
#include <vector>
#include <unordered_map>

typedef uint32_t ComponentId;

#define CREATURE_EMPTY_ENTITY_ID UINT32_MAX
#define CREATURE_EMPTY_COMPONENT UINT32_MAX
#define CREATURE_MAX_COMPONENTS 32
#define COMPONENT_LOAD_FACTOR 0.5f

struct Game;
struct World;

enum TileDirection
{
	North,
	East,
	South,
	West,
	MaxDirs
};

namespace Scal
{
namespace Creature
{

struct EntityTransform
{
	Vector3 Pos;
	Vector3 Scale;
	Vector2i TilePos;
	float Rotation;
};

struct Transform2D
{
	Vector2 Pos;
	int Z;
	float Rotation;
	Vector2i TilePos;
};

struct TextureInfo
{
	Rectangle Rect;
	Rectangle CollisionBox;
};

struct SCreature
{
	World* WorldRef;
	uint32_t Id = CREATURE_EMPTY_ENTITY_ID;
	uint32_t NetId;
	uint32_t TypeId;
	uint32_t ComponentIndex[CREATURE_MAX_COMPONENTS];
	TextureInfo TextureInfo;
	Transform2D Transform;
	TileDirection LookDirection;
	bool ShouldRemove;
	bool IsPlayer;
	bool HasMoved;

	void Update(::Game* game, float dt);

	void Move(Vector2 newPos);

	void Initialize(World* world);
};

Vector2 TileDirToVec2(TileDirection dir);

struct Player;

global_var uint32_t SNextComponentId;

template<typename T>
struct SComponent
{
	static const uint32_t ID;
};

template<typename T>
const uint32_t SComponent<T>::ID = SNextComponentId++;

struct SHealth : SComponent<SHealth>
{
	uint32_t MaxHealth;
	uint32_t Health;
};

struct ComponentMgr
{
	std::unordered_map<ComponentId, SArray,
		std::hash<ComponentId>, std::equal_to<ComponentId>,
		SAllocator<std::pair<const ComponentId, SArray>>> Components;

	ComponentMgr();

	template<typename T>
	void RegisterComponent(ComponentId componentId)
	{
		assert(componentId < CREATURE_MAX_COMPONENTS);
		if (Components.find(componentId) == Components.end())
		{
			// NOT FOUND
			SArray sArray = {};
			ArrayCreate(16, sizeof(T), &sArray);
			Components.insert({ componentId, sArray });
		}
	}

	template<typename T>
	void AddComponent(SCreature* creature,
		const SComponent<T>* component)
	{
		assert(component->ID < CREATURE_MAX_COMPONENTS);
		SArray componentArray = Components[T::ID];
		ArrayPush(&componentArray, component);
		creature->ComponentIndex[T::ID] = componentArray.Length - 1;
	}

	template<typename T>
	void RemoveComponent(SCreature* creature,
		ComponentId componentId)
	{
		assert(componentId < CREATURE_MAX_COMPONENTS);
		assert(creature->ComponentIndex[componentId]
			!= CREATURE_EMPTY_COMPONENT);
		uint32_t index = creature->ComponentIndex[T::ID];
		creature->ComponentIndex[T::ID] = CREATURE_EMPTY_COMPONENT;
		SArray componentArray = Components[T::ID];
		ArrayRemoveAt(&componentArray, static_cast<uint64_t>(index));
	}

	template<typename T>
	T* GetComponent(SCreature* creature,
		ComponentId componentId) const
	{
		assert(componentId < CREATURE_MAX_COMPONENTS);
		assert(creature->ComponentIndex[componentId]
			!= CREATURE_EMPTY_COMPONENT);
		uint32_t index = creature->ComponentIndex[componentId];
		SArray componentArray = Components.at(componentId);
		return (T*)ArrayPeekAt(&componentArray, (uint64_t)index);
	}
};

struct CreatureCache
{
	uint32_t Index;
	bool IsPlayer;
};

struct EntityMgr
{
	std::unordered_map<uint32_t, CreatureCache,
		std::hash<uint32_t>, std::equal_to<uint32_t>,
		SAllocator<std::pair<const uint32_t, CreatureCache>>> EntityMap;
	std::vector<Player, SAllocator<Player>> Players;
	std::vector<SCreature, SAllocator<SCreature>> Creatures;
	std::vector<uint32_t, SAllocator<uint32_t>> EntitiesToRemove;
	ComponentMgr ComponentManager;
	uint32_t NextEntityId;

	EntityMgr();

	void Update(::Game* game, float dt);

	void RemoveEntity(uint32_t id);

	Player& CreatePlayer(::World* world);
	SCreature& CreatureCreature(::World* world);

	Player* GetPlayer(uint32_t id);
	SCreature* GetCreature(uint32_t id);
	SCreature* FindEntity(uint32_t id);

	size_t TotalEntities() const;
	bool IsEmpty(const SCreature& entity) const;
};

void TestCreature(::Game* game);

}
}