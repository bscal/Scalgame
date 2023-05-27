#include "Entity.h"

#include "Game.h"
#include "ComponentTypes.h"
#include "Inventory.h"

#include <raylib/src/raymath.h>

void InitializeEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	componentMgr->Register<SpriteRenderer>();
	componentMgr->Register<Attachable>();
	componentMgr->Register<UpdatingLightSource>();
	componentMgr->RegisterWithCallback<CreatureEntity>(CreatureEntity::OnAdd, CreatureEntity::OnRemove);
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

uint32_t EntityMgr::FindGen(uint32_t entityId)
{
	if (entityId >= Entities.Count)
		return ENT_NOT_FOUND;

	EntityStatus status = Entities[GetId(entityId)];
	
	if (!status.IsAlive)
		return ENT_NOT_FOUND;

	return status.Gen;
}

bool EntityMgr::IsAlive(Entity entity) const
{
	EntityStatus status = Entities[GetId(entity)];
	return (status.IsAlive && status.Gen == GetGen(entity));
}
