#pragma once

#include "Core.h"
#include "SString.h"
#include "Structures/SHashMap.h"

struct Rect16
{
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
};

struct SpriteAtlas
{
	Texture2D Texture;
	Rect16* Rects;
	SHashMap<SRawString, uint16_t, SRawStringHasher> NameToIdx;
	uint16_t Length;
	SRawString TextureName;
};

SpriteAtlas SpriteAtlasLoad(const char* dirPath, const char* atlasFile);
void SpriteAtlasUnload(SpriteAtlas* atlas);
Rectangle SpriteAtlasRect(SpriteAtlas* atlas, const SRawString name);
