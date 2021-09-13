#include "GridTile.h"

#include "core/ResourceManager.h"
#include "core/GameClient.h"

namespace TheGame
{
	GridTile::GridTile(uint32_t x, uint32_t y)
		:X(x), Y(y), Background(std::make_unique<TextureTile>(g_BlankTile)), Foreground(std::make_unique<TextureTile>(g_BlankTile)),
		TemperatureTile(0, 10, 0, SKYBLUE, RED, Rectangle{ (float)x, (float)y, 64, 64 })
	{
	}

	GridTile::GridTile(uint32_t x, uint32_t y, const TextureTile& background)
		: X(x), Y(y), Background(std::make_unique<TextureTile>(background)), Foreground(std::make_unique<TextureTile>(g_BlankTile)),
		TemperatureTile(0, 10, 0, SKYBLUE, RED, Rectangle{ (float)x, (float)y, 64, 64 })
	{
	}

	GridTile::GridTile(uint32_t x, uint32_t y, const TextureTile& background, const TextureTile& foreground)
		: X(x), Y(y), Background(std::make_unique<TextureTile>(background)), Foreground(std::make_unique<TextureTile>(foreground)),
		TemperatureTile(0, 10, 0, SKYBLUE, RED, Rectangle{ (float)x, (float)y, 64, 64 })
	{
	}

	void GridTile::Render(const GameClient& client)
	{
		float tileSize = client.GameWorld->WorldGrid.TileSize;
		Rectangle destination{ X, Y, tileSize, tileSize };
		Vector2 origin{ 0.0f, 0.0f };
		DrawTextureTiled(*g_ResourceManager.TileMap, TextureTileToRect(*Background), destination, origin, 0.0f, 4.0f, WHITE);

		if (*Foreground != g_BlankTile)
		{
			DrawTextureTiled(*g_ResourceManager.TileMap, TextureTileToRect(*Foreground), destination, origin, 0.0f, 4.0f, WHITE);
		}
	}

	void GridTile::Update()
	{
	}
}