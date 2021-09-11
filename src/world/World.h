#pragma once

#include "src/GameHeaders.h"
#include "src/grid/Grid.h"


class World
{
public:
	TheGame::Grid<unsigned int> Grid; // TODO this should be in a new class GameMode ?

public:
	World(const uint32_t& width, const uint32_t& height);
	void Update();
};

