#pragma once

#include "Core.h"
#include "Structures/SArray.h"
#include "SUtil.h"
#include "Sprite.h"

#include <assert.h>
#include <vector>
#include <unordered_map>

typedef uint32_t ComponentId;

#define CREATURE_START_ID 1
#define CREATURE_EMPTY_ENTITY_ID 0
#define CREATURE_EMPTY_COMPONENT UINT32_MAX
#define CREATURE_MAX_COMPONENTS 32
#define COMPONENT_LOAD_FACTOR 0.5f
#define ESTIMATED_ENTITIES 32

struct Game;
struct World;
struct Player;

enum TileDirection : uint8_t
{
	North,
	East,
	South,
	West,
	MaxDirs
};

constexpr float Radian = 6.283185;

constexpr static float
TileDirectionToTurns[TileDirection::MaxDirs] =
{ Radian * 0.75f, 0.0f, Radian * 0.25f, Radian * 0.5f };

constexpr float GetRadiansFromDirection(TileDirection dir)
{
	assert(dir < TileDirection::MaxDirs);
	return TileDirectionToTurns[dir];
}

struct EntityTransform
{
	Vector2 Pos;
	Vector2i TilePos;
	float Rotation; // Degrees
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
	uint32_t ComponentIndex[CREATURE_MAX_COMPONENTS];
	TextureInfo TextureInfo;
	EntityTransform Transform;
	TileDirection LookDirection;
	bool ShouldRemove;
	bool IsPlayer;
	bool HasMoved;

	void Initialize(World* world);
	void Update(Game* game);
};

Vector2 TileDirToVec2(TileDirection dir);

static uint32_t SNextComponentId;

template<typename T>
struct SComponent
{
	static const uint32_t ID;
};

template<typename T>
const uint32_t SComponent<T>::ID = SNextComponentId++;

struct Human : SComponent<Human>
{
	uint32_t Age;
};

struct ComponentMgr
{
	std::unordered_map
		<ComponentId, SArray,
		std::hash<ComponentId>,
		std::equal_to<ComponentId>,
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
		creature->ComponentIndex[T::ID] = (uint32_t)componentArray.Length - 1;
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

	void Initialize(Game* game);
	void Update(Game* game);
	void RemoveEntity(uint32_t id);

	Player& CreatePlayer(World* world);
	SCreature& CreatureCreature(World* world);

	Player* GetPlayer(uint32_t id);
	SCreature* GetCreature(uint32_t id);
	SCreature* FindEntity(uint32_t id);

	size_t TotalEntities() const;
	bool IsEmpty(const SCreature& entity) const;
};
