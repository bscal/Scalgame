#include "TextureTile.h"

TextureTile g_BlankTile{ 0, 0, 16, 16 };

Rectangle TextureTileToRect(const TextureTile& tile)
{
	return Rectangle{ (float)tile.x * 16, (float)tile.y * 16, (float)tile.width, (float)tile.height
	};
}

bool TextureTile::operator==(const TextureTile& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool TextureTile::operator!=(const TextureTile& rhs) const
{
	return x != rhs.x || y != rhs.y;
}
