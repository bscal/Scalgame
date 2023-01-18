#pragma once

#include "Core.h"
#include "Components.h"
#include "SMemory.h"
#include "SUtil.h"
#include "Sprite.h"
#include "Structures/STable.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/BitArray.h"
#include "Vector2i.h"

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
