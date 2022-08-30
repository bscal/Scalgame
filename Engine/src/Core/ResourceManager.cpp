#include "ResourceManager.h"

#define TILE_MAP_TEXTURE_FILEPATH "assets/textures/tiles/16x16.png"

bool InitializeResources(Resources* resources)
{
	resources->MainTileMapTexture = LoadTexture(TILE_MAP_TEXTURE_FILEPATH);
	resources->IsInitialized = true;
	return resources->IsInitialized;
}