#pragma once

#include "TextureTile.h"

class GridTile
{
public:

	GridTile() { m_tiles[0] = BLANK_TILE; m_tiles[1] = BLANK_TILE; }
	GridTile(const TextureTile& background);
	GridTile(const TextureTile& background, const TextureTile& foreground);

	inline TextureTile GetBackground() const { return m_tiles[0]; }
	inline TextureTile GetForeground() const { return m_tiles[1]; }
	inline void SetBackground(const TextureTile& tile) { m_tiles[0] = tile; }
	inline void SetForeground(const TextureTile& tile) { m_tiles[1] = tile; }
private:
	TextureTile m_tiles[2];
};