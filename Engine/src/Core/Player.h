#pragma once

#include "Core.h"
#include "Inventory.h"

struct TransformComponent;

struct PlayerClient
{
	ItemStack CursorStack;
	Vector2i16 CursorStackLastPos;
	Vector2i16 CursorStackClickedPos;
	bool IsCursorStackFlipped;
};

struct PlayerEntity
{
	PlayerClient PlayerClient;
	Vector2i TilePos;
	uint32_t EntityId;
	bool HasMoved;

	void Update();

	TransformComponent* GetTransform();
};

