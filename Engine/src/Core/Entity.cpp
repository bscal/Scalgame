#include "Entity.h"

#include "Game.h"
#include "ComponentTypes.h"

#include "Inventory.h"

#include "raylib/src/raymath.h"

void InitializeEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	componentMgr->Register<TransformComponent>();
	componentMgr->Register<SpriteRenderer>();
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

	SpriteRenderer* renderable = componentMgr->AddComponent(entity, SpriteRenderer{});
	renderable->Sprite = Sprites::PLAYER_SPRITE;
	renderable->DstWidth = 16;
	renderable->DstHeight = 16;
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
		if (CanMoveToTile(&GetGame()->Universe.World, tile))
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
