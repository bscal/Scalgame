#pragma once

#include "GameHeaders.h"
#include "grid/Grid.h"

namespace TheGame
{
	class World
	{
	public:
		Grid<unsigned int> WorldGrid; // TODO this should be in a new class GameMode ?

	public:
		World(const uint32_t& width, const uint32_t& height);
		void Update();
	};
}