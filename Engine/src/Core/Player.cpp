#include "Player.h"

#include "Game.h"
#include "ResourceManager.h"
#include "TileMap.h"
#include "raymath.h"

#include <assert.h>



namespace Scal
{
namespace Creature
{

void Player::InitializePlayer(::World* world)
{
	TextureInfo.Rect = PLAYER_SPRITE;
	S_LOG_INFO("Player Initialized!");
}

void Player::UpdatePlayer(::Game* game, float dt)
{
	HandleInput(game, dt);
}

void Player::HandleInput(::Game* game, float dt)
{
	HasMoved = false;

	bool TryMove = false;
	TileDirection inputMoveDir;
	if (IsKeyPressed(KEY_D))
	{
		TryMove = true;
		inputMoveDir = TileDirection::East;
	}
	else if (IsKeyPressed(KEY_A))
	{
		TryMove = true;
		inputMoveDir = TileDirection::West;
	}
	else if (IsKeyPressed(KEY_S))
	{
		TryMove = true;
		inputMoveDir = TileDirection::South;
	}
	else if (IsKeyPressed(KEY_W))
	{
		TryMove = true;
		inputMoveDir = TileDirection::North;
	}

	if (TryMove)
	{
		Vector2 result = TileDirToVec2(inputMoveDir);
		Vector2i moved = Transform.TilePos.Add(Vec2fToVec2i(result));
		if (CanMoveToTile(WorldRef, moved))
		{
			Transform.TilePos = moved;
			Vector2 movedWorld = Vector2Multiply(moved.AsVec2(), { 16, 16 });
			Transform.Pos.x = movedWorld.x;
			Transform.Pos.y = movedWorld.y;
			HasMoved = true;
		}
		LookDirection = inputMoveDir;
	}
}

//void UpdatePlayer(GameApplication* gameApp, Player* player)
//{
//	if (IsKeyPressed(KEY_D))
//		PlayerMove(gameApp, player, Direction::East);
//	else if (IsKeyPressed(KEY_A))
//		PlayerMove(gameApp, player, Direction::West);
//	else if (IsKeyPressed(KEY_S))
//		PlayerMove(gameApp, player, Direction::South);
//	else if (IsKeyPressed(KEY_W))
//		PlayerMove(gameApp, player, Direction::North);
//}
//
//void RenderPlayer(GameApplication* gameApp, Player* player)
//{
//	Rectangle rect = player->TexturePosition;
//
//	if (player->LookDirection == Direction::West ||
//		player->LookDirection == Direction::South)
//		rect.width = -rect.width;
//
//	DrawTextureRec(
//		gameApp->Resources->EntitySpriteSheet,
//		rect,
//		player->Position,
//		WHITE);
//
//}
//
//constexpr float DirectionAngles[(int)Direction::MaxDirections] =
//{ 0.75f * TAO, 0.0f * TAO, 0.25f * TAO, 0.5f * TAO };
//
//constexpr Vector2i PlayerFowardVectors[(int)Direction::MaxDirections] =
//{ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };
//
//float AngleFromDirection(Direction dir)
//{
//	return DirectionAngles[(int)dir];
//}
//
//Vector2i DirectionToVec(Direction dir)
//{
//	return PlayerFowardVectors[(int)dir];
//}
//
//Vector2i PlayerFoward(Player* player)
//{
//	return DirectionToVec(player->LookDirection);
//}
//
//void PlayerMove(GameApplication* gameApp, Player* player, Direction direction)
//{
//	return;
//	Vector2i playerTilePos = player->TilePosition;
//	Vector2i newPlayerDir = DirectionToVec(direction);
//	Vector2i newPlayerTilePos = 
//	{
//		playerTilePos.x + newPlayerDir.x,
//		playerTilePos.y + newPlayerDir.y
//	};
//
//	player->LookDirection = direction;
//
//	TileMap* tileMap = nullptr; //&gameApp->Game->World.MainTileMap;
//	if (!IsInBounds(newPlayerTilePos.x, newPlayerTilePos.y,
//		tileMap->MapWidth, tileMap->MapHeight))
//		return;
//
//	Tile* tile = GetTile(tileMap, playerTilePos.x, playerTilePos.y);
//	Tile* tileMoveTo = GetTile(tileMap, newPlayerTilePos.x, newPlayerTilePos.y);
//
//	TileData* tileType = GetTileData(tileMap, tile->TileId);
//	TileData* tileTypeMoveTo = GetTileData(tileMap, tileMoveTo->TileId);
//
//	if (tileTypeMoveTo->TileType == TileType::Solid)
//		return;
//
//	if (PlayerProcessEnergy(gameApp, player, tileTypeMoveTo->MovementCost))
//	{
//		player->TilePosition.x = newPlayerTilePos.x;
//		player->TilePosition.y = newPlayerTilePos.y;
//		player->Position.x += (float)newPlayerDir.x * 16.0f;
//		player->Position.y += (float)newPlayerDir.y * 16.0f;
//	}
//}
//
//void PlayerOnNewTurn(GameApplication* gameApp, Player* player)
//{
//	player->Energy = player->MaxEnergy;
//}
//
//bool PlayerProcessEnergy(GameApplication* gameApp, Player* player, int cost)
//{
//	int newEnergy = player->Energy - cost;
//	if (newEnergy < 0) return false;
//
//	player->Energy = newEnergy;
//	if (newEnergy == 0)
//	{
//		UpdateTime(gameApp, 1);
//	}
//	return true;
//}
}
}