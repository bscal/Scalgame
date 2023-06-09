#pragma once

#include "Core.h"

#include "Player.h"

#include "Structures/StaticArray.h"
#include "Structures/SList.h"
#include "Structures/SLinkedList.h"
#include "Structures/ComponentArray.h"
#include "Structures/SHoodTable.h"

enum class EntityTypes : uint8_t
{
	Npc = 0,
	Monster,
	TileEntity
};

union UID
{
	struct
	{
		uint32_t Id : 20;
		uint32_t Generation : 8;
		uint32_t Type : 4;
	};
	uint32_t Packed;
};

struct Player
{

};

struct NPC
{
	UID Uid;
};

enum MonsterType : uint8_t
{
	Human = 0,

	MaxTypes
};

struct Monster
{
	UID Uid;
};

struct TileEntity
{
	UID Uid;
};

struct EntityManager
{
	Player Player;
	
	ComponentArray Npcs;
	uint32_t NextNpcId;
	SList<UID> UnusedNpcId;
	ComponentArray Monsters;
	uint32_t NextMonsterId;
	SList<UID> UnusedMonsterId;
	SHoodTable<Vector2i, TileEntity> TileEntities;

	StaticArray<MonsterType, MonsterType::MaxTypes> MonsterTypes;
};

EntityManager* GetEntityMgr();

NPC* SpawnNPC();
Monster* SpawnMonster();
TileEntity* SpawnTileEntity();
