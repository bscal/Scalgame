#include "GridTile.h"

GridTile::GridTile(const TextureTile& background)
{
	m_tiles[0] = background;
	m_tiles[1] = BLANK_TILE;
}

GridTile::GridTile(const TextureTile& background, const TextureTile& foreground)
{
	m_tiles[0] = background;
	m_tiles[1] = foreground;
}
