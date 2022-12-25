#include "Creature.h"

#include "Game.h"
#include "ResourceManager.h"
#include "Player.h"
#include "World.h"
#include "SMemory.h"

#include <raymath.h>

constexpr global_var Vector2
PlayerFowardVectors[TileDirection::MaxDirs] =
{ { 0.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f } };

Vector2 TileDirToVec2(TileDirection dir)
{
	return PlayerFowardVectors[dir];
}

internal void InternalRemoveEntity(EntityMgr* mgr, uint32_t entityId)
{
	assert(mgr);
	CreatureCache creatureCache = mgr->EntityMap.at(entityId);
	mgr->EntityMap.erase(entityId);
	if (creatureCache.IsPlayer)
	{
		assert(entityId < mgr->Players.size());
		const auto& lastCreature = mgr->Players.back();
		mgr->EntityMap[lastCreature.Id].Index = creatureCache.Index;
		mgr->Players[creatureCache.Index] = lastCreature;
		mgr->Players.pop_back();
	} 
	else
	{
		assert(entityId < mgr->Creatures.size());
		const auto& lastCreature = mgr->Creatures.back();
		mgr->EntityMap[lastCreature.Id].Index = creatureCache.Index;
		mgr->Creatures[creatureCache.Index] = lastCreature;
		mgr->Creatures.pop_back();
	}
}

internal void ProcessRemoveEntities(EntityMgr* mgr)
{
	for (size_t i = 0; i < mgr->EntitiesToRemove.size(); ++i)
	{
		InternalRemoveEntity(mgr, mgr->EntitiesToRemove[i]);
	}
	mgr->EntitiesToRemove.clear();
}

void EntityMgr::Initialize(Game* game)
{
	EntityMap.reserve(ESTIMATED_ENTITIES);
	Players.reserve(1);
	Creatures.reserve(ESTIMATED_ENTITIES);
	EntitiesToRemove.reserve(8);
	S_LOG_INFO("[ Entity Manager ] Initialized!");
}

void EntityMgr::Update(Game* game)
{
	for (size_t i = 0; i < Players.size(); ++i)
	{
		Players[i].UpdatePlayer(game);
		Players[i].Update(game);
	}
	for (size_t i = 0; i < Creatures.size(); ++i)
	{
		Creatures[i].Update(game);
	}

	ProcessRemoveEntities(this);
}

Player* EntityMgr::GetPlayer(uint32_t id)
{
	for (size_t i = 0; i < Players.size(); ++i)
	{
		if (Players[i].Id == id) return &Players[i];
	}
	return nullptr;
}

SCreature* EntityMgr::GetCreature(uint32_t id)
{
	for (size_t i = 0; i < Creatures.size(); ++i)
	{
		if (Creatures[i].Id == id) return &Creatures[i];
	}
	return nullptr;
}

Player& EntityMgr::CreatePlayer(::World* world)
{
	Player p = {};
	p.Id = NextEntityId++;
	p.Initialize(world);
	p.InitializePlayer(world);
	Players.push_back(p);

	size_t index = Players.size() - 1;

	CreatureCache creatureCache = {};
	creatureCache.Index = (uint32_t)index;
	creatureCache.IsPlayer = true;
	EntityMap.insert({ p.Id, creatureCache });

	return Players.back();
}

SCreature& EntityMgr::CreatureCreature(::World* world)
{
	SCreature creature = {};
	creature.Id = NextEntityId++;
	creature.Initialize(world);
	Creatures.push_back(creature);

	CreatureCache creatureCache;
	creatureCache.Index = (uint32_t)Creatures.size() - 1;
	creatureCache.IsPlayer = false;
	EntityMap.insert({ creature.Id, creatureCache });

	return Creatures.back();
}

void EntityMgr::RemoveEntity(uint32_t entityId)
{
	assert(entityId != CREATURE_EMPTY_ENTITY_ID);
	if (entityId == CREATURE_EMPTY_ENTITY_ID)
	{
		S_LOG_ERR("Cannot remove empty entity!");
		return;
	}

	auto creature = FindEntity(entityId);
	if (!creature || creature->ShouldRemove)
	{
		S_LOG_WARN("Trying to remove an entity that is "
			"Already marked for removal!");
		return;
	}

	creature->ShouldRemove = true;
	EntitiesToRemove.emplace_back(entityId);
}

SCreature* EntityMgr::FindEntity(uint32_t id)
{
	const auto findResult = EntityMap.find(id);
	if (findResult == EntityMap.end())
		return nullptr;

	const CreatureCache creatureCache = findResult->second;
	if (creatureCache.IsPlayer)
	{
		return &(Players[creatureCache.Index]);
	}
	else
	{
		return &(Creatures[creatureCache.Index]);
	}
}

size_t EntityMgr::TotalEntities() const
{
	return EntityMap.size();
}

bool EntityMgr::IsEmpty(const SCreature& creature) const
{
	return (creature.Id == CREATURE_EMPTY_ENTITY_ID);
}

ComponentMgr::ComponentMgr()
	: Components()
{
	Components.max_load_factor(COMPONENT_LOAD_FACTOR);
	Components.reserve(CREATURE_MAX_COMPONENTS);

	RegisterComponent<Human>(Human::ID);
}

internal Rectangle RectToTextCoords(const Texture2D& texture, 
	const Rectangle& rect)
{
	Rectangle result;
	result.x = rect.x / (float)texture.width;
	result.y = rect.y / (float)texture.height;
	result.width = rect.width / (float)texture.width;
	result.height = rect.height / (float)texture.height;
	return result;
}

void SCreature::Update(Game* game)
{
	const auto& sheet = GetGameApp()->Resources->EntitySpriteSheet;
	Rectangle rect = TextureInfo.Rect;
	if (LookDirection == TileDirection::West ||
		LookDirection == TileDirection::South)
	{
		rect.width = -rect.width;
	}
	DrawTextureRec(sheet, rect, Transform.Pos, WHITE);
}

void SCreature::Initialize(struct World* world)
{
	WorldRef = world;
	constexpr size_t size = sizeof(uint32_t) * CREATURE_MAX_COMPONENTS;
	Scal::MemSet(&ComponentIndex, CREATURE_EMPTY_COMPONENT, size);
}


void SCreature::SetTilePos(Vector2i tilePos)
{
	Transform.TilePos = tilePos;
	Vector2i tileSize = WorldTileScale(WorldRef);
	Transform.Pos = tilePos.Multiply(tileSize).AsVec2();
}
