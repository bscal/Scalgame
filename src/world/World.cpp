#include "World.h"

#include "core/GameClient.h"

namespace TheGame
{
	World::World(uint32_t width, uint32_t height, GameClient& client)
		: WorldGrid(width, height, 64), Client(client)
	{
		for (uint32_t y = 0; y < WorldGrid.Height; y++)
		{
			for (uint32_t x = 0; x < WorldGrid.Width; x++)
			{
				Vector2 pos = WorldGrid.ToWorldPos(x, y);
				Rectangle rect = { pos.x, pos.y, WorldGrid.TileSize, WorldGrid.TileSize };
				WorldGrid.Move(x, y, std::make_unique<GridTile>(x, y, TextureTile{ 3, 3, 16, 16 }));
				//WorldGrid.Move(x, y, std::move(std::make_unique<HeatMapTile>(0, 10, 0, SKYBLUE, RED, rect)));
				//WorldGrid.GetArray()[x + y * WorldGrid.Width] =;
			}
		}
	}

	void World::Update()
	{
		for (uint32_t y = 0; y < WorldGrid.Height; y++)
		{
			for (uint32_t x = 0; x < WorldGrid.Width; x++)
			{
				WorldGrid.Get(x, y)->Render(Client);
				WorldGrid.DrawDebugText(x, y);
				WorldGrid.DrawDebugRect(x, y);
			}
		}
	}
}