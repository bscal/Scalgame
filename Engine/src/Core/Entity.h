#pragma once

#include "Core.h"
#include "Player.h"
#include "Inventory.h"
#include "SUtil.h"
#include "SString.h"
#include "Sprite.h"

#include "Structures/StaticArray.h"
#include "Structures/SHashMap.h"
#include "Structures/SList.h"

#include <MemoryPool/MemoryPool.h>

struct GameApplication;
struct Game;

constexpr global_var uint32_t ENT_PLAYER = { 0 };
constexpr global_var uint32_t ENT_NOT_FOUND = UINT32_MAX;

struct EntitySkeleton
{
	Vector2 Head;
	Vector2 Body;
	Vector2 LHand;
	Vector2 RHand;
};

constexpr EntitySkeleton AsSkeleton(Vector2 head, Vector2 body, Vector2 lHand, Vector2 rHand)
{
	EntitySkeleton res = {};
	res.Head = head;
	res.Body = body;
	res.LHand = lHand;
	res.RHand = rHand;
	return res;
}

constexpr global_var EntitySkeleton SKELETON_HUMAN = AsSkeleton({ 6.0f, 4.0f }, { 8.0f, 8.0f }, { 4.0f, 8.0f }, { 12.0f, 8.0f});

enum class EntitySize : uint8_t
{
	Tiny,
	Small,

	Medium,
	Large,
	Huge,
	Giant,

	MaxSize
};

enum class EntityTypes : uint8_t
{
	Player = 0,
	Npc,
	Monster,
	TileEntity,

	MaxSize
};

enum class Pain : uint8_t
{
	None,
	Scratch,	// Twisted ankle, minor wound
	Hurt,		// Bleeding
	Very,		// Major injury, Broken bone
	Extreme,	

	MaxSize
};

namespace Groups
{
enum
{
	Player = 0,
	Wild,
	Monsters,

	MaxSize
};
}

enum CreatureType : uint16_t
{
	Human = 0,

	MaxSize
};

struct CreatureData
{
	SString Name;				// Monster internal name
	SString DefaultDisplayName;	// Display name given to initilized creature, can be overriden
	SString Desc;				// Description
	SString Lore;				// Learnable lore

	Sprite Sprite;				// Src Rect on sprite sheet		
	EntitySkeleton Skeleton;	// Body part points on creature

	short MaxEnergy;
	short MaxHealth;
	uint16_t YoungAge;
	uint16_t OldAge;
	EntitySize Size;	// Size of creature
	uint8_t GroupId;	// Groups define Friendly/Neutral/Enemy relations
	bool IsAggresive;	
};

struct WorldEntity
{
	Vector2i TilePos;
	Color Color;
	uint32_t StorageIdx;
	TileDirection LookDir;
	EntityTypes EntityType;
};

struct Creature
{
	SString DisplayName;	

	uint32_t InventoryId;	
	uint32_t EquipmentId;

	uint16_t CreatureType;
	uint16_t Age;

	short Energy;
	short Health;
	short Stamina;
	short Morale;
	short Sanity;

	Pain Pain;

	bool IsMale;
	bool IsSleeping;

	Equipment Equipment;
};

struct Character
{
	SString FirstName;
	SString LastName;
	SString Title;
};

struct PlayerClient
{
	ItemStack CursorStack;
	Vector2 ItemSlotOffset;
	Vector2i16 ItemSlotOffsetSlot;
	Vector2i16 CursorStackLastPos;
	bool IsCursorStackFlipped;
};

struct Player : public WorldEntity
{
	PlayerClient PlayerClient;
	Creature Creature;
	Character Character;
	uint32_t Uid;

	_FORCE_INLINE_ Vector2 AsPosition() const { return { TilePos.x * TILE_SIZE_F, TilePos.y * TILE_SIZE_F }; }
};

struct Monster : public WorldEntity
{
	Creature Creature;
	uint32_t Uid;
};

struct EntityManager
{
	constexpr static size_t MONSTER_BLOCK_SZ = AlignPowTwo64Ceil((sizeof(Monster) * 256));

	Player Player;
	
	uint32_t NextUid = 1; // 0 Is always player

	SHashMap<uint32_t, void*> Entities;
	SList<Monster*> Monsters;

	MemoryPool<Monster, MONSTER_BLOCK_SZ> MonsterPool;

	StaticArray<CreatureData, CreatureType::MaxSize> CreatureDB;
};

void EntityManagerInitialize();
EntityManager* GetEntityMgr();

void UpdateEntities(Game* game);
void DrawEntities(Game* game);

void CreatePlayer(Player* player);

Monster* SpawnMonster();
void DeleteMonster(Monster* monster);

void* GetEntity(uint32_t ent);
bool DoesEntityExist(uint32_t ent);

CreatureData* GetCreatureType(Player* player);
CreatureData* GetCreatureType(Monster* monster);
CreatureData* GetCreatureType(Creature* creature);