#pragma once

#include "Core.h"

#include "Structures/SList.h"
#include "Structures/ComponentArray.h"
#include "Structures/SHoodTable.h"

union UID
{
	struct
	{
		uint32_t Id : 24;
		uint32_t Gen : 8;
	};
	uint32_t Value;
};

struct Player
{

};

struct Monster
{

};

struct TileEntity
{

};

struct Trigger
{

};

struct EntityManager
{
	Player Player;
	
	ComponentArray Monsters;
	SHoodTable<Vector2i, TileEntity> TileEntities;
};
