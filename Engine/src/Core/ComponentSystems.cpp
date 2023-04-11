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

	ComponentArray<Renderable>* renderables = componentMgr->GetArray<Renderable>();
	for (uint32_t i = 0; i < renderables->Size(); ++i)
	{
		uint32_t entity = renderables->Indices.Dense[i];
		Renderable renderable = renderables->Values[i];
		const TransformComponent* transform = transforms->Get(entity);
		SASSERT(transform);

		if (transform->LookDir == TileDirection::South || transform->LookDir == TileDirection::West)
			renderable.SrcWidth = -renderable.SrcWidth;
		SDrawSprite(spriteTextureSheet, transform, renderable);
	}

	ComponentArray<UpdatingLightSource>* updatingLights = componentMgr->GetArray<UpdatingLightSource>();
	for (uint32_t i = 0; i < updatingLights->Size(); ++i)
	{
		UpdatingLightSource& light = updatingLights->Values[i];
		light.UpdateCounter -= GetDeltaTime();
		if (light.UpdateCounter < 0)
		{
			light.UpdateCounter = 0.2;
			int rand = SRandNextRange(GetGlobalRandom(), 0, 1);
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

	//uint32_t queryIds[2] = { Attachable::Id, TransformComponent::Id };
	//SList<uint32_t> queryAttachables = componentMgr->FindEntities(queryIds, ArrayLength(queryIds));
	// 
	//for (uint32_t i = 0; i < queryAttachables.Count; ++i)
	//{
	//	uint32_t entity = queryAttachables[i];
	//	auto attachable = attachables->Get(entity);
	//	auto transform = transforms->Get(entity);
	//	auto parentTransform = transforms->Get(attachable->EntityId);
	//	transform->LookDir = parentTransform->LookDir;
	//	bool isLookingRight = (parentTransform->LookDir == TileDirection::South || parentTransform->LookDir == TileDirection::West);
	//	transform->Position = Vector2Add(parentTransform->Position, entityAttachable.EntityOrigin);
	//	transform->Position.x += (isLookingRight) ? -entityAttachable.Local.Position.x : entityAttachable.Local.Position.x;
	//	transform->Position.y += entityAttachable.Local.Position.y;
	//}

	for (uint32_t i = 0; i < attachables->Size(); ++i)
	{
		uint32_t entity = attachables->Indices.Dense[i];
		const Attachable& entityAttachable = attachables->Values[i];
		TransformComponent* entityTransform = transforms->Get(entity);

		const TransformComponent* parent = transforms->Get(entityAttachable.EntityId);
		SASSERT(parent);
		entityTransform->LookDir = parent->LookDir;
		bool isLookingRight = (parent->LookDir == TileDirection::South || parent->LookDir == TileDirection::West);
		entityTransform->Position = Vector2Add(parent->Position, entityAttachable.EntityOrigin);
		entityTransform->Position.x += (isLookingRight) ? -entityAttachable.Local.Position.x : entityAttachable.Local.Position.x;
		entityTransform->Position.y += entityAttachable.Local.Position.y;
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