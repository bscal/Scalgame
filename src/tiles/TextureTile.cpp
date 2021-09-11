#include "TextureTile.h"

bool TextureTile::operator==(const TextureTile& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool TextureTile::operator!=(const TextureTile& rhs) const
{
	return x != rhs.x || y != rhs.y;
}
