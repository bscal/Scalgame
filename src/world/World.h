#pragma once

#include <memory>

#include "GameHeaders.h"
#include "grid/Grid.h"
#include "tiles/GridTile.h"

namespace TheGame
{
	class World
	{
	public:
		Grid<std::unique_ptr<GridTile>> WorldGrid; // TODO this should be in a new class GameMode ?
		GameClient& Client; // TODO a ServerWorld shouldnt have a client reference

	public:
		World(uint32_t width, uint32_t height, GameClient& client);
		void Update();
	};
}