#pragma once

#include "Engine.h"
#include "Core/TileMap.h"

struct Resources
{
	TextureTileSet MainTileSet;
	Font MainFont;
	bool IsInitialized;
};

bool InitializeResources(Resources* outData);
