#pragma once

#include "Core.h"

#include "Structures/BitArray.h"
#include "Structures/SList.h"

struct CreatureEntity;
struct World;


struct MoveAction
{
	Vector3 ToPosition;
	struct CreatureEntity* Creature;
	int Cost;
};

struct MoveList
{
	SList<MoveAction> Actions;
};

void QueueMove(MoveList* moves, const MoveAction* move);
void MovesProcess(MoveList* moves, World* world);
