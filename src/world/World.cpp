#include "World.h"

namespace TheGame
{
	World::World(const uint32_t& width, const uint32_t& height)
		: WorldGrid(width, height, 64)
	{
		int i = 0;
		for (uint32_t y = 0; y < WorldGrid.Height; y++)
		{
			for (uint32_t x = 0; x < WorldGrid.Width; x++)
			{
				i++;
				WorldGrid.Set(x, y, i);
			}
		}
	}

	void World::Update()
	{
		for (uint32_t y = 0; y < WorldGrid.Height; y++)
		{
			for (uint32_t x = 0; x < WorldGrid.Width; x++)
			{
				WorldGrid.DrawDebugText(x, y);
				WorldGrid.DrawDebugRect(x, y);
			}
		}
	}
}