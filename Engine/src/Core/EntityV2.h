#pragma once

#include "Core.h"
#include "Player.h"
#include "SUtil.h"
#include "SString.h"
#include "Sprite.h"

#include "Structures/StaticArray.h"
#include "Structures/SHashMap.h"

#include <MemoryPool/MemoryPool.h>

struct Game;

union EntityId
{
	struct
	{
		uint32_t Id : 28;
		uint32_t Type : 4;
	};
	uint32_t Mask;
};

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

enum CreatureTypes : uint16_t
{
	Human = 0,

	MaxSize
};

struct CreatureType
{
	SString Name;	// Monster internal name
	SString Desc;	// Description
	SString Lore;	// Learnable lore

	Sprite Sprite;	// Src Rect on sprite sheet		

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
	TileDirection LookDir;
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
};

struct Character
{
	SString FirstName;
	SString LastName;
	SString Title;
};

constexpr global_var EntityId PLAYER_ENTITY = { 0 };

struct Player : public WorldEntity
{
	Creature Creature;
	Character Character;
	EntityId Uid;
};

struct Monster : public WorldEntity
{
	Creature Creature;
	EntityId Uid;
};

struct TileEntity : public WorldEntity
{
};

struct EntityManager
{
	constexpr static size_t MONSTER_BLOCK_SZ = AlignPowTwo64Ceil((sizeof(Monster) * 256));

	Player Player;
	
	uint32_t NextUid = 1; // 0 Is always player
	SHashMap<uint32_t, void*> Entities;
	SHashMap<Vector2i, TileEntity, Vector2iHasher> TileEntities;

	MemoryPool<Monster, MONSTER_BLOCK_SZ> Monsters;

	StaticArray<CreatureType, CreatureTypes::MaxSize> CreatureDB;
};

void EntityMgrInitialize();
EntityManager* GetEntityMgr();

void UpdateEntities(Game* game);
void DrawEntities(Game* game);

Monster* SpawnMonster();
void DeleteMonster(Monster* monster);

void* GetEntity(EntityId ent);
bool DoesEntityExist(EntityId ent);

TileEntity* SpawnTileEntity(Vector2i pos);
