#pragma once

#include "Core.h"
#include "Creature.h"

#include <vector>
#include <bitset>

struct Movement
{
	int MaxPoints;
	int Points;
	int PointsPerTurn;
};

struct MoveAction
{
	std::bitset<16> set;
	Vector3 ToPosition;
	struct Creature* Creature;
	int Cost;
};

struct MoveMgr
{
	std::vector<MoveAction> MoveActions;

	void Insert(const MoveAction* action)
	{
		bool added = false;
		for (int i = 0; i < MoveActions.size(); ++i)
		{
			const auto& at = MoveActions[i];
			if (at.Cost > action->Cost)
			{
				MoveActions.push_back(at);
				MoveActions[i] = *action;
				added = true;
				break;
			}
		}

		if (!added)
		{
			MoveActions.push_back(*action);
		}
	}

	void Process(World* world)
	{
		for (int i = 0; i < MoveActions.size(); ++i)
		{
			//const auto& at = MoveActions[i];
		}
		MoveActions.clear();
	}
};

