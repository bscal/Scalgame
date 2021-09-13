#pragma once

#include "GameHeaders.h";

struct TextureTile
{
	int x, y, width, height;

	bool operator==(const TextureTile& rhs) const;
	bool operator!=(const TextureTile& rhs) const;
};

Rectangle TextureTileToRect(const TextureTile& tile);

extern TextureTile g_BlankTile;