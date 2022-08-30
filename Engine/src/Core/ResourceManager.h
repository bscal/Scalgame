#pragma once

#include "Engine.h"

struct Resources
{
	Texture2D MainTileMapTexture;
	bool IsInitialized;
};

bool InitializeResources(Resources* outData);
