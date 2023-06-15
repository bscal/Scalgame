#include "Player.h"

#include "Core.h"
#include "Game.h"
#include "Entity.h"

#include <raylib/src/raymath.h>

void
PlayerUpdate(Player* player, Game* game)
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
			Inventory* playerInv = game->InventoryMgr.Inventories.Get(&playerCreature->InventoryId);
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
		CreatureEntity* creature = game->ComponentMgr.AddComponent(GetClientPlayer()->EntityId, CreatureEntity {});
		Inventory* playerInv = game->InventoryMgr.Inventories.Get(&creature->InventoryId);
		SASSERT(playerInv);
		Equipment* playerEquipment = game->InventoryMgr.Equipments.Get(&creature->InventoryId);
		SASSERT(playerEquipment);

		ItemStack itemStack = ItemStackNew(Items::TORCH, 1);
		playerInv->InsertStack({ 2, 2 }, { 0, 0 }, &itemStack, false);
		ItemStack itemStack2 = ItemStackNew(Items::FIRE_STAFF, 1);
		playerInv->InsertStack({ 0, 2 }, { 0, 0 }, &itemStack2, false);

		ItemStack stack = ItemStackNew(Items::TORCH, 1);
		playerEquipment->EquipItem(creature, &stack, 0);
	}

	if (IsKeyPressed(KEY_ZERO))
	{
	}
}

void DrawPlayer(Player* player, Game* game)
{
	Texture2D* texture = &game->Resources.EntitySpriteSheet;
	Sprite sprite = GetCreatureType(&player->Creature)->Sprite;
	SDrawSprite(texture, sprite, player->TilePos, player->Color);
}
