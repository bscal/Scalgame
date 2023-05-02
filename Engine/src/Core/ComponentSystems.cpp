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

	ComponentArray<TransformComponent>* transforms = componentMgr->GetArray<TransformComponent>();

	ComponentArray<SpriteRenderer>* renderables = componentMgr->GetArray<SpriteRenderer>();
	for (uint32_t i = 0; i < renderables->Size(); ++i)
	{
		uint32_t entity = renderables->Indices.Dense[i];
		SpriteRenderer renderable = renderables->Values[i];
		const TransformComponent* transform = transforms->Get(entity);
		SASSERT(transform);

		bool flipX = transform->LookDir == TileDirection::South || transform->LookDir == TileDirection::West;
		SDrawSprite(spriteTextureSheet, transform, renderable, flipX);
	}

	ComponentArray<UpdatingLightSource>* updatingLights = componentMgr->GetArray<UpdatingLightSource>();
	for (uint32_t i = 0; i < updatingLights->Size(); ++i)
	{
		UpdatingLightSource& light = updatingLights->Values[i];
		light.UpdateCounter -= GetDeltaTime();
		if (light.UpdateCounter < 0)
		{
			light.UpdateCounter = 0.2;
			int rand = (int)SRandNextRange(GetGlobalRandom(), 0, 1);
			light.FinalColor = light.Colors[rand];
			light.Radius = SRandNextFloatRange(GetGlobalRandom(), light.MinRadius, light.MaxRadius);
		}

		uint32_t entity = renderables->Indices.Dense[i];
		const TransformComponent* transform = transforms->Get(entity);
		SASSERT(transform);
		DrawLightWithShadows(transform->TilePos().AsVec2(), light);
	}
}

internal void 
UpdateAttachables(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	ComponentArray<TransformComponent>* transforms = componentMgr->GetArray<TransformComponent>();
	ComponentArray<Attachable>* attachables = componentMgr->GetArray<Attachable>();

	for (uint32_t i = 0; i < attachables->Size(); ++i)
	{
		uint32_t entity = attachables->Indices.Dense[i];
		const Attachable& attachable = attachables->Values[i];

		TransformComponent* entityTransform = transforms->Get(entity);
		SASSERT(entityTransform);
		TransformComponent* parentTransform = transforms->Get(attachable.EntityId);
		SASSERT(parentTransform);
		
		entityTransform->LookDir = parentTransform->LookDir;
		entityTransform->Position.x = parentTransform->Position.x + attachable.EntityOrigin.x + attachable.Local.Position.x;
		entityTransform->Position.y = parentTransform->Position.y + attachable.EntityOrigin.y + attachable.Local.Position.y;
	}

	SList<uint32_t> entToDelete = {};
	entToDelete.Allocator = SAllocator::Temp;
	for (uint32_t i = 0; i < attachables->Size(); ++i)
	{
		uint32_t parentEntity = attachables->Values[i].EntityId;
		if (!entityMgr->IsAlive(parentEntity))
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