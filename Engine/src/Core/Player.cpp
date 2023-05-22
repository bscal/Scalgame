#include "Player.h"

#include "Game.h"
#include "ComponentTypes.h"

#include <raylib/src/raymath.h>

void PlayerEntity::Update()
{
	HasMoved = false;
	TileDirection inputMoveDir;
	if (IsKeyPressed(KEY_D))
	{
		HasMoved = true;
		inputMoveDir = TileDirection::East;
	}
	else if (IsKeyPressed(KEY_A))
	{
		HasMoved = true;
		inputMoveDir = TileDirection::West;
	}
	else if (IsKeyPressed(KEY_S))
	{
		HasMoved = true;
		inputMoveDir = TileDirection::South;
	}
	else if (IsKeyPressed(KEY_W))
	{
		HasMoved = true;
		inputMoveDir = TileDirection::North;
	}

	if (HasMoved)
	{
		TransformComponent* transform = GetTransform();
		Vector2 moveDir = TileDirectionVectors[(uint8_t)inputMoveDir];
		Vector2 moveAmount = Vector2Scale(moveDir, TILE_SIZE_F);
		Vector2 movePoint = Vector2Add(transform->Position, moveAmount);

		Vector2i tile = Vector2i::FromVec2(Vector2Scale(movePoint, INVERSE_TILE_SIZE));
		if (CanMoveToTile(&GetGame()->Universe.World, tile))
		{
			TilePos = tile;
			transform->Position = movePoint;
			GetGame()->CameraLerpTime = 0.0f;
		}
		transform->LookDir = inputMoveDir;
	}
}

TransformComponent* PlayerEntity::GetTransform()
{
	uint32_t entityId = GetId(EntityId);
	return GetGame()->ComponentMgr.GetComponent<TransformComponent>(entityId);
}