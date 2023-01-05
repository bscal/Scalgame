#pragma once

#include "Structures/SList.h"

#include <stdint.h>
#include <raylib/src/raylib.h>

#include <string>
#include <unordered_map>

constexpr std::string_view Stone = "Tile1";

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
	std::string TextureName;
	std::string AtlasFilePath;
	SList<Rectangle> SpritesArray;
	std::unordered_map<std::string, uint32_t> SpritesByName;
	bool IsLoaded;

	bool Load(const char* atlasDataPath, uint64_t estimatedSprites);
	void Unload();

	inline uint64_t Size() const { return SpritesArray.Count; }
	inline const Rectangle& SpriteAtlas::GetRect(uint64_t index) const
	{
		return SpritesArray[index];
	}
};
