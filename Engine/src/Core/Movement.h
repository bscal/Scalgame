#pragma once

#include "Core.h"

#include "Structures/BitArray.h"
#include "Structures/SList.h"

struct CreatureEntity;
struct World;
struct Action;

enum class ActionType : int
{
	REST = 0,
	MOVE,

	MAX
};

typedef bool(*ActionFunction)(Action*, int);

struct Action
{
	CreatureEntity* Creature;
	ActionFunction Func;
	void* ActionData;
	ActionType Type;
	int Cost;
};

struct MoveList
{
	SList<Action> Actions;
};

