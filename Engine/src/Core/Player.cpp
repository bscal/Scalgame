#include "Player.h"

#include "Game.h"
#include "ResourceManager.h"
#include "TileMap.h"
#include "raymath.h"

#include <assert.h>

void Player::InitializePlayer(World* world)
{
	TextureInfo.Rect = PLAYER_SPRITE.TexCoord;
	SLOG_INFO("Player Initialized!");
}

void Player::UpdatePlayer(Game* game)
{
	HandleInput(game);
}

void Player::HandleInput(Game* game)
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
			Transform.Pos.x = (float)moved.x * 16.0f;
			Transform.Pos.y = (float)moved.y * 16.0f;
			HasMoved = true;
			game->CameraLerpTime = 0.0f;
		}
		LookDirection = inputMoveDir;
	}
}