#include "Creature.h"

#include "Game.h"
#include "ResourceManager.h"
#include "Player.h"
#include "World.h"
#include "SMemory.h"
#include <raymath.h>

namespace Scal
{
namespace Creature
{

constexpr static Vector2
PlayerFowardVectors[TileDirection::MaxDirs] =
{ { 0.0f, -1.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f } };

Vector2 TileDirToVec2(TileDirection dir)
{
	assert(dir >= 0);
	assert(dir < TileDirection::MaxDirs);
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

#define ESTIMATED_ENTITIES 32
EntityMgr::EntityMgr()
	: EntityMap(),
	Players(),
	Creatures(),
	EntitiesToRemove(),
	NextEntityId(0)
{
	EntityMap.reserve(ESTIMATED_ENTITIES);
	Players.reserve(1);
	Creatures.reserve(ESTIMATED_ENTITIES);
	EntitiesToRemove.reserve(8);
	S_LOG_INFO("Entity Manager Initialized!");
}

void EntityMgr::Update(::Game* game, float dt)
{
	for (size_t i = 0; i < Players.size(); ++i)
	{
		Players[i].UpdatePlayer(game, dt);
		Players[i].Update(game, dt);
	}
	for (size_t i = 0; i < Creatures.size(); ++i)
	{
		Creatures[i].Update(game, dt);
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

	const auto creatureCache = findResult->second;
	if (creatureCache.IsPlayer)
	{
		return &(Players[creatureCache.Index]);
	} else
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

void SCreature::Update(::Game* game, float dt)
{
	const auto& sheet = GetGameApp()->Resources->EntitySpriteSheet;
	Rectangle rect = TextureInfo.Rect;
	if (LookDirection == TileDirection::West ||
		LookDirection == TileDirection::South)
	{
		rect.width = -rect.width;
	}
	Vector2 drawPosSS = GetWorldToScreen2D(Transform.Pos, game->Camera);
	DrawTextureRec(sheet, rect, Transform.Pos, WHITE);

	// 
	//texcoords[0] = { 0.0f, 1.0f }; // x, h
	//texcoords[1] = { 0.0f, 0.0f }; // x, w
	//texcoords[2] = { 1.0f, 0.0f }; // y, w
	//texcoords[3] = { 1.0f, 1.0f }; // y, h
	//Rectangle texCoords = RectToTextCoords(sheet, rect);
	//Model.meshes[0].texcoords[0] = 0.0f;
	//Model.meshes[0].texcoords[1] = texCoords.width;
	//Model.meshes[0].texcoords[2] = 0.0f;
	//Model.meshes[0].texcoords[3] = 0.0f;
	//Model.meshes[0].texcoords[4] = texCoords.width;
	//Model.meshes[0].texcoords[5] = 0.0f;
	//Model.meshes[0].texcoords[6] = texCoords.width;
	//Model.meshes[0].texcoords[7] = texCoords.width;
	//int uvSize = sizeof(float) * 8;
	//UpdateMeshBuffer(Model.meshes[0], 1, Model.meshes[0].texcoords, uvSize, 0);
	////Matrix matrix = MatrixIdentity();
	////MatrixTranslate(Transform.Pos.x, Transform.Pos.y, 0.1f);
	////DrawMesh(Mesh, Mat, matrix);
	//rlPushMatrix();
	//if (b)
	//{
	//	rlRotatef(180.0f, 1.0, 0.0, 0.0);
	//}
	//DrawModel(Model, { Transform.Pos.x, Transform.Pos.y, 0.1f }, 1.0f, WHITE);
	//rlPopMatrix();

	//DrawModelEx(Model,
	//	{ Transform.Pos.x, Transform.Pos.y, 0.1f },
	//	{ 0, 1, 0 },
	//	90.0f,
	//	Vector3Zero(),
	//	WHITE
	//);
	//DrawBillboardPro(
	//	game->Camera3D,
	//	sheet,
	//	rect,
	//	{ Transform.Pos.x, Transform.Pos.y, 0.1f },
	//	{ 0.0f, 1.0f, 0.0f },
	//	{ 16.0f, 16.0f },
	//	{ 0.0f, 0.0f },
	//	Transform.Rotation,
	//	WHITE
	//);
}

void Move(Vector2 newPos)
{

}

void SCreature::Initialize(struct ::World* world)
{
	WorldRef = world;
	constexpr size_t size = sizeof(uint32_t) * CREATURE_MAX_COMPONENTS;
	Scal::MemSet(&ComponentIndex, CREATURE_EMPTY_COMPONENT, size);

	//Mesh = GenMeshPlaneXY(16.0f, 16.0f, 2, 2);
	//Model = LoadModelFromMesh(Mesh);
	//Mat = LoadMaterialDefault();
	//Texture2D sheet = GetGameApp()->Resources->EntitySpriteSheet;
	//SetMaterialTexture(&Mat, MATERIAL_MAP_DIFFUSE, sheet);
	//Model.materials[0] = Mat;
	//SetModelMeshMaterial(&Model, 0, 0);

	//Pos = { 16.0f / 2.0f, 16.0f / 2.0f };
}

}
}