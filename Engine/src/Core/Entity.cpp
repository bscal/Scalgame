#include "Entity.h"

#include "Game.h"
#include "ComponentTypes.h"

#include "Inventory.h"

#include "raylib/src/raymath.h"

void InitializeEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	componentMgr->Register<SpriteRenderer>();
	componentMgr->Register<Attachable>();
	componentMgr->Register<UpdatingLightSource>();
	componentMgr->Register<CreatureEntity>();
}

void CreatePlayer(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	uint32_t entity = entityMgr->CreateEntity();
	entityMgr->Player.EntityId = entity;

	componentMgr->AddComponent<TransformComponent>(entity, {});

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

	SLOG_INFO("Created %s", FMT_ENTITY(result));

	return result;
}

void EntityMgr::RemoveEntity(Entity entity)
{
	uint32_t id = GetId(entity);
	
	if (!Entities[id].IsAlive)
		return;

	SLOG_INFO("Removed %s", FMT_ENTITY(entity));

	SList<ComponentArray>& components = GetGame()->ComponentMgr.Components;
	for (uint32_t i = 0; i < components.Count; ++i)
	{
		ComponentArray& componentArray = components[i];
		componentArray.Remove(entity);
	}

	entity = IncGen(entity);
	Entities[id].Gen = GetGen(entity);
	Entities[id].IsAlive = false;
	FreeIds.Push(&entity);
}

bool EntityMgr::IsAlive(Entity entity) const
{
	EntityStatus status = Entities[GetId(entity)];
	return (status.IsAlive && status.Gen == GetGen(entity));
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
