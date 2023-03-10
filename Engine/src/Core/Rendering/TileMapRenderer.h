#pragma once

#include "Core/Core.h"
#include "Core/Globals.h"

struct TileMapRenderer
{
	Shader TileMapShader;
	RenderTexture2D TileMapTexture;
	int PixelWidth;
	int PixelHeight;
	int TileWidth;
	int TileHeight;

	int UniformTileAtlasLoc;
	int UniformTileMapLoc;

	uint32_t Tiles[];
};
