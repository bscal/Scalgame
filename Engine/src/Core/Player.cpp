#include "Player.h"

#include "Core.h"
#include "Game.h"
#include "Entity.h"

#include <raylib/src/raymath.h>

void
UpdatePlayer(Player* player, Game* game)
{
	SASSERT(player);
	SASSERT(game);
	PlayerClient* playerClient = &player->PlayerClient;

	// Get move input & direction
	bool hasMoved = false;
	TileDirection inputMoveDir;
	if (IsKeyPressed(KEY_D))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::East;
	}
	else if (IsKeyPressed(KEY_A))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::West;
	}
	else if (IsKeyPressed(KEY_S))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::South;
	}
	else if (IsKeyPressed(KEY_W))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::North;
	}

	// Handle movement
	if (hasMoved)
	{
		Vector2i tileToMoveTo = player->TilePos + Vec2i_NEIGHTBORS[(uint8_t)inputMoveDir];
		player->LookDir = inputMoveDir;
		if (CanMoveToTile(&game->Universe.World, tileToMoveTo))
		{
			player->TilePos = tileToMoveTo;
			game->CameraLerpTime = 0.0f;
		}
	}

	if (IsKeyPressed(KEY_EIGHT))
	{
		if (game->IsInventoryOpen && !playerClient->CursorStack.IsEmpty())
		{
			Inventory* playerInv = GetInventory(player->Creature.InventoryId);
			if (playerInv && playerInv->InsertStack(playerClient->CursorStackLastPos
				, playerClient->ItemSlotOffsetSlot
				, &playerClient->CursorStack
				, playerClient->IsCursorStackFlipped))
			{
				playerClient->CursorStack.Remove();
				playerClient->IsCursorStackFlipped = false;
				playerClient->CursorStackLastPos = {};
				playerClient->ItemSlotOffset = {};
			}
		}
		game->IsInventoryOpen = !game->IsInventoryOpen;
	}

	if (IsKeyPressed(KEY_NINE))
	{
		Inventory* playerInv = GetInventory(player->Creature.InventoryId);
		SASSERT(playerInv);

		ItemStack itemStack = ItemStackNew(Items::TORCH, 1);
		playerInv->InsertStack({ 2, 2 }, { 0, 0 }, &itemStack, false);
		ItemStack itemStack2 = ItemStackNew(Items::FIRE_STAFF, 1);
		playerInv->InsertStack({ 0, 2 }, { 0, 0 }, &itemStack2, false);

		ItemStack stack = ItemStackNew(Items::TORCH, 1);
		EquipItem(player, &player->Creature, &stack, 0);
	}

	if (IsKeyPressed(KEY_ZERO))
	{
	}
}

void DrawPlayer(Player* player, Game* game)
{
	Texture2D* texture = &game->Resources.EntitySpriteSheet;
	CreatureData* creature = GetCreatureType(player);
	Vector2 worldPos = player->AsPosition();
	SDrawSprite(texture, player, worldPos, creature->Sprite);

	if (!player->Creature.Equipment.MainHand.IsEmpty())
	{
		Sprite sprite = player->Creature.Equipment.MainHand.GetItem()->Sprite;
		Vector2 pos = Vector2Add(worldPos, player->Creature.Skeleton.RHand);
		SDrawSprite(texture, player, pos, sprite);
	}

	if (!player->Creature.Equipment.OffHand.IsEmpty())
	{
		Sprite sprite = player->Creature.Equipment.OffHand.GetItem()->Sprite;
		Vector2 pos = Vector2Add(worldPos, player->Creature.Skeleton.LHand);
		SDrawSprite(texture, player, pos, sprite);
	}
}
