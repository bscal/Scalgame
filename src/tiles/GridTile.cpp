#include "GridTile.h"

#include "core/ResourceManager.h"
#include "core/GameClient.h"

namespace TheGame
{
	GridTile::GridTile(uint32_t x, uint32_t y)
		:X(x), Y(y), Background(std::make_unique<TextureTile>(g_BlankTile)), Foreground(std::make_unique<TextureTile>(g_BlankTile)),
		TemperatureTile(0, 10, 0, SKYBLUE, RED)
	{
	}

	GridTile::GridTile(uint32_t x, uint32_t y, const TextureTile& background)
		: X(x), Y(y), Background(std::make_unique<TextureTile>(background)), Foreground(std::make_unique<TextureTile>(g_BlankTile)),
		TemperatureTile(0, 10, 0, SKYBLUE, RED)
	{
	}

	GridTile::GridTile(uint32_t x, uint32_t y, const TextureTile& background, const TextureTile& foreground)
		: X(x), Y(y), Background(std::make_unique<TextureTile>(background)), Foreground(std::make_unique<TextureTile>(foreground)),
		TemperatureTile(0, 10, 0, SKYBLUE, RED)
	{
	}

	void GridTile::Render(const Rectangle& destination, const GameClient& client)
	{
		Vector2 origin{ 0.0f, 0.0f };
		DrawTextureTiled(*g_ResourceManager.TileMap, TextureTileToRect(*Background), destination, origin, 0.0f, 4.0f, WHITE);

		if (*Foreground != g_BlankTile)
		{
			DrawTextureTiled(*g_ResourceManager.TileMap, TextureTileToRect(*Foreground), destination, origin, 0.0f, 4.0f, WHITE);
		}

		if (ShowTemperature)
			TemperatureTile.Render(destination, client);
	}

	void GridTile::Update()
	{
	}
}