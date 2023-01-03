#pragma once

#include "Core.h"
#include "Components.h"
#include "SUtil.h"
#include "Sprite.h"
#include "Structures/SArray.h"

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

constexpr global_var float
TileDirectionToTurns[TileDirection::MaxDirs] =
{ TAO * 0.75f, 0.0f, TAO * 0.25f, TAO * 0.5f };

inline constexpr float GetRadiansFromDirection(TileDirection dir)
{
	return TileDirectionToTurns[dir];
}

struct EntityTransform
{
	Vector2 Pos;
	Vector2i TilePos;
	Vector2i ChunkPos;
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
	uint64_t Id = CREATURE_EMPTY_ENTITY_ID;
	uint32_t ComponentIndex[CREATURE_MAX_COMPONENTS];
	TextureInfo TextureInfo;
	EntityTransform Transform;
	TileDirection LookDirection;
	bool ShouldRemove;
	bool IsPlayer;
	bool HasMoved;
	bool IsFrozen;

	void Initialize(World* world);
	void Update(Game* game);

	void SetTilePos(Vector2i tilePos);
};

Vector2 TileDirToVec2(TileDirection dir);

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