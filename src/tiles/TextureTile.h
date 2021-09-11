#pragma once

struct TextureTile
{
	int x, y, width, height;

	bool operator==(const TextureTile& rhs) const;
	bool operator!=(const TextureTile& rhs) const;
};

static const TextureTile BLANK_TILE({ 0, 0, 16, 16 });