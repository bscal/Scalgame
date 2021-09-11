#include "World.h"

World::World(const uint32_t& width, const uint32_t& height)
	: Grid(width, height, 64)
{
	int i = 0;
	for (uint32_t y = 0; y < Grid.Height; y++)
	{
		for (uint32_t x = 0; x < Grid.Width; x++)
		{
			i++;
			Grid.Set(x, y, i);
			printf("%d\n", Grid.Get(x, y));
		}
	}
}

void World::Update()
{
	for (uint32_t y = 0; y < Grid.Height; y++)
	{
		for (uint32_t x = 0; x < Grid.Width; x++)
		{
			Grid.DrawDebugText(x, y);
			Grid.DrawDebugRect(x, y);
		}
	}
}
