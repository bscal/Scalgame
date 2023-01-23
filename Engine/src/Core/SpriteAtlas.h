#pragma once

#include "SString.h"
#include "Structures/SList.h"
#include "Structures/STable.h"

#include <stdint.h>
#include <raylib/src/raylib.h>

constexpr const char* Stone = "Tile1";

struct SpriteAtlas
{
	struct AtlasInfo
	{
		int X;
		int Y;
		int TileW;
		int TileH;
	};

	Texture2D Texture;
	SString TextureName;
	SString AtlasFilePath;
	SList<Rectangle> SpritesArray;
	STable<SString, uint32_t> SpritesByName = STable<SString, uint32_t>(STableDefaultKeyEquals);
	bool IsLoaded;

	bool Load(const char* atlasDataPath, uint64_t estimatedSprites);
	void Unload();

	inline uint64_t Size() const { return SpritesArray.Count; }
	inline const Rectangle& GetRect(uint64_t index) const
	{
		return SpritesArray[index];
	}
	Rectangle GetRectByName(std::string_view name) const;
};
