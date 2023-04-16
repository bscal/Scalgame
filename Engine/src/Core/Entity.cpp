#include "Entity.h"

#include "Game.h"
#include "ComponentTypes.h"

#include "Inventory.h"

#include "raylib/src/raymath.h"

void InitializeEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	componentMgr->Register<TransformComponent>();
	componentMgr->Register<Renderable>();
	componentMgr->Register<Attachable>();
	componentMgr->Register<UpdatingLightSource>();
	componentMgr->Register<CreatureEntity>();
}

void CreatePlayer(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	uint32_t entity = entityMgr->CreateEntity();
	entityMgr->Player.EntityId = entity;

	//entityMgr->Player.Transform.Origin = { 8.0f, 8.0f };
	//entityMgr->Player.Transform.Position = { -8.0f, -8.0f };
	componentMgr->AddComponent(entity, entityMgr->Player.Transform);

	Renderable* renderable = componentMgr->AddComponent(entity, Renderable{});
	renderable->SrcWidth = 16;
	renderable->SrcHeight = 16;
	renderable->DstWidth = 16;
	renderable->DstHeight = 16;

	uint32_t torch = entityMgr->CreateEntity();
	componentMgr->AddComponent(torch, TransformComponent{});

	Renderable* torchRenderable = componentMgr->AddComponent(torch, Renderable{});
	torchRenderable->x = 0;
	torchRenderable->y = 32;
	torchRenderable->SrcWidth = 4;
	torchRenderable->SrcHeight = 4;
	torchRenderable->DstWidth = 4;
	torchRenderable->DstHeight = 4;

	Attachable* torchAttachable = componentMgr->AddComponent(torch, Attachable{});
	torchAttachable->EntityId = entity;
	torchAttachable->EntityOrigin = { 8.0f, 8.0f };
	torchAttachable->Local.Origin.x = 2.0f;
	torchAttachable->Local.Origin.y = 2.0f;
	torchAttachable->Local.Position.x = 4.0f;
	torchAttachable->Local.Position.y = -3.0f;
	torchAttachable->Local.Rotation = 0.0f;

	UpdatingLightSource* torchLight = componentMgr->AddComponent(torch, UpdatingLightSource{});
	torchLight->MinRadius = 6;
	torchLight->MaxRadius = 7;
	torchLight->Colors[0] = { 250, 190, 200, 200 };
	torchLight->Colors[1] = { 255, 200, 210, 200 };

	Inventory* inv = GetGame()->InventoryMgr.CreateInvetory(entity);
	Equipment* equipment = GetGame()->InventoryMgr.CreateEquipment();

	CreatureEntity* creature = componentMgr->AddComponent(torch, CreatureEntity{});
	creature->InventoryId = inv->InventoryId;
	creature->EquipmentId = equipment->EquipmentId;

	ItemStack stack = {};
	stack.ItemId = Items::TORCH;
	equipment->EquipItem(entity, creature, 0, &stack);
}

uint32_t EntityMgr::CreateEntity()
{
	uint32_t result;
	if (FreeIds.HasNext())
		result = FreeIds.PopValue();
	else
		result = Entities.Count;

	uint32_t id = GetId(result);
	Entities.EnsureSize(id + 1);
	Entities[id].Gen = GetGen(result);
	Entities[id].IsAlive = true;

	SLOG_INFO("Created Entity(%u), Id: %u Gen: %u", result, id, GetGen(result));

	return result;
}

void EntityMgr::RemoveEntity(uint32_t entityId)
{
	uint32_t id = GetId(entityId);
	Entities[id].Gen = IncGen(entityId);
	Entities[id].IsAlive = false;
	FreeIds.Push(&entityId);
}

bool EntityMgr::IsAlive(uint32_t entityId) const
{
	EntityStatus status = Entities[GetId(entityId)];
	return (status.IsAlive && status.Gen == GetGen(entityId));
}

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
		Vector2 moveDir = TileDirectionVectors[(uint8_t)inputMoveDir];
		Vector2 moveAmount = Vector2Scale(moveDir, TILE_SIZE_F);
		Vector2 movePoint = Vector2Add(Transform.Position, moveAmount);

		Vector2i tile = Vector2i::FromVec2(Vector2Scale(movePoint, INVERSE_TILE_SIZE));
		if (CanMoveToTile(&GetGame()->World, tile))
		{
			TilePos = tile;
			Transform.Position = movePoint;
			GetGame()->CameraLerpTime = 0.0f;
		}
		Transform.LookDir = inputMoveDir;

		TransformComponent* playerTransform = GetGame()->ComponentMgr.GetComponent<TransformComponent>(EntityId);
		*playerTransform = Transform;
	}
}