#include "ResourceManager.h"

#define TILE_MAP_TEXTURE_PATH "assets/textures/tiles/16x16.png"

#define FONT_PATH "assets/textures/fonts/Silver.ttf"

bool InitializeResources(Resources* resources)
{
	LoadTileSet(TILE_MAP_TEXTURE_PATH, 16, 16,
		&resources->MainTileSet);

	resources->MainFont = LoadFont(TILE_MAP_TEXTURE_PATH);

	TraceLog(LOG_INFO, "Initialized Resources");

	resources->IsInitialized = true;
	return resources->IsInitialized;
}
