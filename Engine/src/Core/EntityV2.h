#pragma once

#include "Core.h"
#include "Player.h"
#include "SUtil.h"
#include "SString.h"

#include "Structures/StaticArray.h"
#include "Structures/SHashMap.h"

#include <MemoryPool/MemoryPool.h>

struct GameApplication;

union Entity
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
	uint16_t SpriteIdx;
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

constexpr global_var Entity PLAYER_ENTITY = { 0 };

struct Player : public WorldEntity
{
	Creature Creature;
	Character Character;
	Entity Uid;
};

struct Monster : public WorldEntity
{
	Creature Creature;
	Entity Uid;
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

void EntityMgrInitialize(GameApplication* gameApp);
EntityManager* GetEntityMgr();


Monster* SpawnMonster();
void DeleteMonster(Monster* monster);

void* GetEntity(Entity ent);
bool DoesEntityExist(Entity ent);

TileEntity* SpawnTileEntity(Vector2i pos);
