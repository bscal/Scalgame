#pragma once

#include "Core.h"
#include "Inventory.h"

struct GameApplication;
struct TransformComponent;

struct PlayerClient
{
	ItemStack CursorStack;
	Vector2 ItemSlotOffset;
	Vector2i16 ItemSlotOffsetSlot;
	Vector2i16 CursorStackLastPos;
	bool IsCursorStackFlipped;
};

struct PlayerEntity
{
	PlayerClient PlayerClient;
	Vector2i TilePos;
	uint32_t EntityId;
	bool HasMoved;

	TransformComponent* GetTransform();
};

void HandlePlayerInput(GameApplication* gameApp, PlayerEntity* player);

void UpdatePlayer(PlayerEntity* player, Game* game);
