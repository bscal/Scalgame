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

void HandlePlayerInput(GameApplication* gameApp, PlayerEntity* player)
{
	SASSERT(gameApp);
	SASSERT(player);
	if (!gameApp->IsGameInputDisabled)
	{
		Game* game = gameApp->Game;
		PlayerClient* playerClient = &player->PlayerClient;

		if (IsKeyPressed(KEY_EIGHT))
		{
			PlayerEntity* player = GetClientPlayer();
			if (game->IsInventoryOpen && !playerClient->CursorStack.IsEmpty())
			{
				CreatureEntity* playerCreature = game->ComponentMgr.GetComponent<CreatureEntity>(player->EntityId);
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
			static uint32_t curId = 1;

			uint32_t gen = GetGame()->EntityMgr.FindGen(curId);
			if (gen != ENT_NOT_FOUND)
			{
				uint32_t entity = SetGen(curId, gen);
				GetGame()->EntityMgr.RemoveEntity(entity);

				Attachable* a = game->ComponentMgr.GetComponent<Attachable>(entity);
				SASSERT(!a);
			}
		}
	}
}
