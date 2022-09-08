#include "Player.h"

#include "Game.h"
#include "TileMap.h"

#include <cassert>

bool InitializePlayer(GameApplication* gameApp, Player* outPlayer)
{
	outPlayer->Position = { 0, 0 };
	outPlayer->TexturePosition = PLAYER_SPRITE;
	outPlayer->MaxEnergy = 2;
	outPlayer->Energy = outPlayer->MaxEnergy;
	outPlayer->LookDirection = Direction::East;
	return true;
}

void UpdatePlayer(GameApplication* gameApp, Player* player)
{
	if (IsKeyPressed(KEY_D))
		MovePlayer(gameApp, player, 1, 0);
	else if (IsKeyPressed(KEY_A))
		MovePlayer(gameApp, player, -1, 0);
	else if (IsKeyPressed(KEY_S))
		MovePlayer(gameApp, player, 0, 1);
	else if (IsKeyPressed(KEY_W))
		MovePlayer(gameApp, player, 0, -1);
}

void RenderPlayer(GameApplication* gameApp, Player* player)
{
	Rectangle rect = player->TexturePosition;

	if (player->LookDirection == Direction::West ||
		player->LookDirection == Direction::South)
		rect.width = -rect.width;

	DrawTextureRec(
		gameApp->Resources->EntitySpriteSheet,
		rect,
		player->Position,
		WHITE);

}

constexpr global_var float DirectionAngles[4] =
	{ 0.75f * TAO, 0.0f * TAO, 0.25f * TAO, 0.5f * TAO };

float AngleFromDirection(Direction dir)
{
	assert((uint8_t)dir > -1);
	assert((uint8_t)dir < 4);
	return DirectionAngles[(uint8_t)dir];
}

void MovePlayer(GameApplication* gameApp, Player* player, int tileX, int tileY)
{
	Vector2i playerTilePos = player->TilePosition;
	Vector2i newPlayerTilePos = { playerTilePos.x + tileX, playerTilePos.y + tileY };

	if (tileX < 0)
		player->LookDirection = Direction::West;
	else if (tileX > 0)
		player->LookDirection = Direction::East;
	else if (tileY < 0)
		player->LookDirection = Direction::North;
	else if (tileY > 0)
		player->LookDirection = Direction::South;

	TileMap* tileMap = &gameApp->Game->World.MainTileMap;
	if (!IsInBounds(newPlayerTilePos.x, newPlayerTilePos.y,
		tileMap->MapWidth, tileMap->MapHeight))
	{
		return;
	}

	Tile* tile = GetTile(tileMap, playerTilePos.x, playerTilePos.y);
	Tile* tileMoveTo = GetTile(tileMap, newPlayerTilePos.x, newPlayerTilePos.y);

	TileType* tileType = GetTileInfo(tileMap, tile->TileId);
	TileType* tileTypeMoveTo = GetTileInfo(tileMap, tileMoveTo->TileId);

	if (ProcessEnergy(gameApp, player, tileTypeMoveTo->MovementCost))
	{
		player->TilePosition.x = newPlayerTilePos.x;
		player->TilePosition.y = newPlayerTilePos.y;
		player->Position.x += tileX * 16;
		player->Position.y += tileY * 16;
	}

}

bool ProcessEnergy(GameApplication* gameApp, Player* player, int cost)
{
	int newEnergy = player->Energy - cost;
	if (newEnergy < 0) return false;

	player->Energy = newEnergy;
	if (newEnergy == 0)
	{
		UpdateTime(gameApp, 1);
	}
	return true;
}
