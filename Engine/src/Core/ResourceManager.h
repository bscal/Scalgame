#pragma once

#include "Engine.h"
#include "Core/TileMap.h"

struct SDFFont
{
	Font Font;
	Shader Shader;
};

struct Resources
{
	Texture2D TileSheet;
	TileSet MainTileSet;
	SDFFont SDFFont;
	Font MainFont;
	bool IsInitialized;
};

bool InitializeResources(Resources* outData);
