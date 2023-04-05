#include "ComponentSystems.h"

#include "Globals.h"
#include "Game.h"
#include "Renderer.h"
#include "Entity.h"
#include "ComponentTypes.h"

void UpdateEntities(EntityMgr* entityMgr, ComponentMgr* componentMgr)
{
	const Texture2D* spriteTextureSheet = &GetGame()->Resources.EntitySpriteSheet;

	ComponentArray<TransformComponent>* transforms = componentMgr->GetArray<TransformComponent>();

	ComponentArray<Renderable>* renderables = componentMgr->GetArray<Renderable>();
	for (uint32_t i = 0; i < renderables->Indices.Count; ++i)
	{
		uint32_t entity = renderables->Indices.Dense[i];
		Renderable renderable = renderables->Values[i];
		const TransformComponent* transform = transforms->Get(entity);
		if (!transform) continue;
		if (transform->LookDir == TileDirection::South || transform->LookDir == TileDirection::West)
			renderable.SrcWidth = -renderable.SrcWidth;
		SDrawSprite(spriteTextureSheet, transform, renderable);
	}
}