#pragma once

#include "Core.h"

#include "SString.h"
#include "Inventory.h"

#include "Structures/BitArray.h"

struct Game;
struct SEntity;
struct CreatureTypeInfo;

#define ENTITY_NOT_FOUND UINT32_MAX
#define ENTITY_ID_NOT_FOUND 0xffffff
#define UID_PLAYER 0

enum EntityType : uint8_t
{
	ENTITY_HUMANOID,
	ENTITY_BEAST,
	ENTITY_UNDEAD,
	ENTITY_DEMON,

	ENTITY_MAX_TYPES
};

enum EntitySize : uint8_t
{
	SIZE_TINY,		// Rat
	SIZE_SMALL,		// Goblin
	SIZE_HUMAN,
	SIZE_LARGE,		// Cow/Bear
	SIZE_HUGE,		// Gryphon/Elephant
	SIZE_GIANT,		// Dragon/Ent

	ESIZE_MAX
};

enum EntityGroup : uint8_t
{
	GROUP_PLAYER,
	GROUP_PASSIVE,
	GROUP_NEUTRAL,
	GROUP_HOSTILE,
	GROUP_HOSTILE_TO_PLAYER,
};

union EntitySkeleton
{
#define SkeletonMaxParts 4
	struct
	{
		Vector2 Head;
		Vector2 Body;
		Vector2 LHand;
		Vector2 RHand;
	};
	Vector2 Positions[SkeletonMaxParts];
};

enum EquipmentSlot : uint8_t
{
	EQUIPMENT_MAIN_HAND,
	EQUIPMENT_OFF_HAND,
	EQUIPMENT_HEAD,
	EQUIPMENT_CHEST,
	EQUIPMENT_LEG,
	EQUIPMENT_BOOTS,

	EQUIPMENT_MAX_SLOTS
};

union Equipment
{
	struct
	{
		ItemStack MainHand;
		ItemStack OffHand;
		ItemStack Head;
		ItemStack Body;
		ItemStack Legs;
		ItemStack Feet;
	};
	ItemStack Stacks[EQUIPMENT_MAX_SLOTS];
};

enum EntityFlags
{
	EFLAG_PLAYER = Bit(0),
	EFLAG_DEAD = Bit(1),

	EFLAG_SNEAKING = Bit(8),
	EFLAG_RUNNING = Bit(9),
};

enum StatusFlags
{
	STATUS_STUN		= Bit(0),
	STATUS_SLOW		= Bit(2),
	STATUS_IN_WATER	= Bit(3),
	STATUS_FIRE		= Bit(4),
	STATUS_HELLFIRE = Bit(5),

	STATUS_POISON_1 = Bit(6),
	STATUS_POISON_2 = Bit(7),
	STATUS_POISON_3 = Bit(8),

	STATUS_WEBBED = Bit(9),
	STATUS_ROOTED = Bit(10),
};


struct Character
{
	SString FirstName;
	SString LastName;
	SString Title;
	int Age;
};

enum CreatureType : uint16_t
{
	CREATURE_HUMAN,
	CREATURE_RAT,

	CREATURE_MAX
};

typedef void(*EntityEnable)(SEntity*, CreatureTypeInfo*);
typedef void(*EntityDisable)(SEntity*, CreatureTypeInfo*);
typedef void(*EntityUpdate)(SEntity*, CreatureTypeInfo*);

struct CreatureTypeInfo
{
	EntityEnable OnEnable;
	EntityDisable OnDisable;
	EntityUpdate OnUpdate;
	
	EntitySkeleton Skeleton;	// Body part points on creature

	SString Name;				// Monster internal name
	SString DefaultDisplayName;	// Display name given to initilized creature, can be overriden
	SString Desc;				// Description
	SString Lore;				// Learnable lore

	short MaxEnergy;
	short MaxHealth;
	uint16_t OldAge;
	EntityGroup GroupId;	// Groups define Friendly/Neutral/Enemy relations
};

struct CreatureData
{
	union
	{
		struct Human
		{

		};
	};
};

union UID
{
	struct
	{
		uint32_t Id : 24;
		uint32_t Gen : 8;
	};
	uint32_t Number;
};

struct SEntity
{
	Vector2i TilePos;
	Vector2 Origin;
	Color Color;
	uint16_t SpriteId;
	TileDirection LookDir;
	EntityType Type;
	CreatureType CreatureType;
	UID Uid;
	BitFlags64 Flags;
	BitFlags64 Statuses;
	UID OwnerUid;

	EntityGroup Group;
	EntitySize Size;

	Equipment Equipment;

	Inventory* Inventory;
	Character* Char;

	short Energy;
	short MaxEnergy;
	short Health;
	short MaxHealth;

	CreatureData Data;

	_FORCE_INLINE_ Vector2 TileToWorld() const
	{
		return { (float)TilePos.x * TILE_SIZE_F, (float)TilePos.y * TILE_SIZE_F };
	}
};

SEntity* EntitySpawn(CreatureType creatureType);
bool EntityAlive(UID uid);
SEntity* EntityGet(UID uid);
void EntityDestroy(UID uid);

const CreatureTypeInfo* CreatureGet(CreatureType type);

SEntity* GetPlayer();

void EntitiesInitialize();

void EntitiesUpdate(Game* game);
void EntitiesDraw(Game* game);

bool EquipItem(SEntity* entity, const ItemStack* stack, uint8_t slot);
bool UnquipItem(SEntity* entity, uint8_t slot);
