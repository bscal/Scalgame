#include "Creature.h"

#include "Game.h"
#include "ResourceManager.h"
#include "Player.h"
#include "World.h"
#include "EntityMgr.h"
#include "SMemory.h"

#include <raymath.h>

constexpr global_var Vector2
PlayerFowardVectors[TileDirection::MaxDirs] =
{ { 0.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f } };

Vector2 TileDirToVec2(TileDirection dir)
{
	return PlayerFowardVectors[dir];
}

internal Rectangle RectToTextCoords(const Texture2D& texture, 
	const Rectangle& rect)
{
	Rectangle result;
	result.x = rect.x / (float)texture.width;
	result.y = rect.y / (float)texture.height;
	result.width = rect.width / (float)texture.width;
	result.height = rect.height / (float)texture.height;
	return result;
}

void SCreature::Update(Game* game)
{
	const auto& sheet = game->Resources.EntitySpriteSheet;
	Rectangle rect = TextureInfo.Rect;
	if (LookDirection == TileDirection::West ||
		LookDirection == TileDirection::South)
	{
		rect.width = -rect.width;
	}
	DrawTextureRec(sheet, rect, Transform.Pos, WHITE);
}

void SCreature::Initialize(struct World* world)
{
	WorldRef = world;
	constexpr size_t size = sizeof(uint32_t) * CREATURE_MAX_COMPONENTS;
	SMemSet(&ComponentIndex, CREATURE_EMPTY_COMPONENT, size);
}


void SCreature::SetTilePos(Vector2i tilePos)
{
	Transform.TilePos = tilePos;
	Vector2i tileSize = WorldTileScale(WorldRef);
	Transform.Pos = tilePos.Multiply(tileSize).AsVec2();

	Vector2i newChunkPos = tilePos.Divide({ CHUNK_SIZE, CHUNK_SIZE });
	if (!newChunkPos.Equals(Transform.ChunkPos))
	{
		//UpdateEntityPosition(Id, Transform.ChunkPos, newChunkPos);
		Transform.ChunkPos = newChunkPos;
	}
}
