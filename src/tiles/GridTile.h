#pragma once

#include <memory>

#include "Tile.h"
#include "TextureTile.h"
#include "HeatMapTile.h"

namespace TheGame
{
	class GridTile : Tile
	{
	public:
		const uint32_t X, Y;
		std::unique_ptr<TextureTile> Background;
		std::unique_ptr<TextureTile> Foreground;
		HeatMapTile TemperatureTile;

	public:
		GridTile(uint32_t x, uint32_t y);
		GridTile(uint32_t x, uint32_t y, const TextureTile& background);
		GridTile(uint32_t x, uint32_t y, const TextureTile& background, const TextureTile& foreground);

		virtual void Render(const GameClient& client) override;
		virtual void Update() override;
	};
}

