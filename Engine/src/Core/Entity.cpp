#include "Entity.h"

#include "Game.h"
#include "Renderer.h"

#include "raylib/src/raymath.h"

void InitializeEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	componentMgr->Register<TransformComponent>();
	componentMgr->Register<Renderable>();

	CreatePlayer(entityMgr, componentMgr);
}

void CreatePlayer(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	uint32_t entity = entityMgr->CreateEntity();
	entityMgr->Player.EntityId = entity;

	componentMgr->AddComponent(entity, entityMgr->Player.Transform);
	Renderable* renderable = componentMgr->AddComponent(entity, Renderable{});
	renderable->Width = 16;
	renderable->Height = 16;
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

void UpdateEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	const Texture2D* spriteTextureSheet = &GetGame()->Resources.EntitySpriteSheet;

	ComponentArray<TransformComponent>* transforms = componentMgr->GetArray<TransformComponent>();
	
	ComponentArray<Renderable>* renderables = componentMgr->GetArray<Renderable>();
	for (uint32_t i = 0; i < renderables->Values.Count; ++i)
	{
		uint32_t entity = renderables->Indices.Dense[i];
		Renderable renderable = renderables->Values[i];
		const TransformComponent* transform = transforms->Get(entity);
		SASSERT(transform); // TODO

		//Rectangle src = { renderable.x, renderable.y, renderable.Width, renderable.Height };
		//Rectangle dst = { transform->Position.x, transform->Position.y, renderable.Width, renderable.Height };
		//DrawTexturePro(spriteTextureSheet, src, dst, {}, 0, WHITE);
		SDrawSprite(spriteTextureSheet, transform, renderable);
	}

}

void PlayerEntity::Update(GameApplication* gameApp)
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
		Vector2 moveDir = TileDirectionVectors[(uint8_t)inputMoveDir];
		Vector2 moveAmount = Vector2Scale(moveDir, TILE_SIZE_F);
		Vector2 movePoint = Vector2Add(Transform.Position, moveAmount);

		Vector2i tile = Vector2i::FromVec2(Vector2Scale(movePoint, INVERSE_TILE_SIZE));
		if (CanMoveToTile(&GetGame()->World, tile))
		{
			Transform.Position = movePoint;
			HasMoved = true;
			GetGame()->CameraLerpTime = 0.0f;
		}
		Transform.LookDir = inputMoveDir;

		*gameApp->Game->ComponentMgr.GetComponent<TransformComponent>(EntityId) = Transform;

	}
}