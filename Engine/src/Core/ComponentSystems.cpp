#include "ComponentSystems.h"

#include "Globals.h"
#include "Game.h"
#include "Lighting.h"
#include "Renderer.h"
#include "Entity.h"
#include "ComponentTypes.h"

#include "raylib/src/raymath.h"

internal void UpdateAttachables(EntityMgr* entityMgr, ComponentMgr* componentMgr);

void 
UpdateEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	UpdateAttachables(entityMgr, componentMgr);

	const Texture2D* spriteTextureSheet = &GetGame()->Resources.EntitySpriteSheet;

	ComponentArray* renderables = componentMgr->GetArray<SpriteRenderer>();

	for (uint32_t i = 0; i < renderables->Size(); ++i)
	{
		uint32_t entity = renderables->Indices.Dense[i];
		SpriteRenderer* renderable = renderables->Index<SpriteRenderer>(i);
		TransformComponent* transform = componentMgr->GetComponent<TransformComponent>(entity);

		bool flipX = transform->LookDir == TileDirection::South || transform->LookDir == TileDirection::West;
		SDrawSprite(spriteTextureSheet, transform, renderable, flipX);
	}

	ComponentArray* updatingLights = componentMgr->GetArray<UpdatingLightSource>();

	for (uint32_t i = 0; i < updatingLights->Size(); ++i)
	{
		UpdatingLightSource* light = updatingLights->Index<UpdatingLightSource>(i);
		light->UpdateCounter -= GetDeltaTime();
		if (light->UpdateCounter < 0)
		{
			light->UpdateCounter = 0.2;
			int rand = (int)SRandNextRange(GetGlobalRandom(), 0, 1);
			light->FinalColor = light->Colors[rand];
			light->Radius = SRandNextFloatRange(GetGlobalRandom(), light->MinRadius, light->MaxRadius);
		}

		uint32_t entity = renderables->Indices.Dense[i];
		const TransformComponent* transform = componentMgr->GetComponent<TransformComponent>(entity);
		SASSERT(transform);
		DrawLightWithShadows(transform->TilePos().AsVec2(), *light);
	}
}

internal void 
UpdateAttachables(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	SList<uint32_t> entToDelete = {};
	entToDelete.Allocator = SAllocator::Temp;

	ComponentArray* attachables = componentMgr->GetArray<Attachable>();

	for (uint32_t i = 0; i < attachables->Size(); ++i)
	{
		uint32_t entity = attachables->Indices.Dense[i];
		Attachable* attachable = attachables->Index<Attachable>(i);
		TransformComponent* entityTransform = componentMgr->GetComponent<TransformComponent>(entity);
		TransformComponent* parentTransform = componentMgr->GetComponent<TransformComponent>(attachable->ParentEntity);
		
		entityTransform->LookDir = parentTransform->LookDir;
		bool flipX = entityTransform->LookDir == TileDirection::South || entityTransform->LookDir == TileDirection::West;
		float offsetX = (flipX) ? -attachable->Local.Position.x - attachable->Width : attachable->Local.Position.x;

		entityTransform->Position.x = parentTransform->Position.x + attachable->Target.x + offsetX;
		entityTransform->Position.y = parentTransform->Position.y + attachable->Target.y + attachable->Local.Position.y;

		if (!entityMgr->IsAlive(attachable->ParentEntity))
		{
			uint32_t entity = attachables->Indices.Dense[i];
			entToDelete.Push(&entity);
		}
	}

	for (uint32_t entity : entToDelete)
	{
		entityMgr->RemoveEntity(entity);
	}
}