#include "ResourceManager.h"

namespace TheGame
{
	ResourceManager g_ResourceManager;


	void ResourceManager::Load()
	{
		MainFont = std::make_unique<Font>(LoadFont("assets/textures/fonts/Silver.ttf"));
		TileMap = std::make_unique<Texture2D>(LoadTexture("assets/textures/tiles/16x16.png"));
	}

	void ResourceManager::Cleanup()
	{
		UnloadFont(*MainFont);
		UnloadTexture(*TileMap);
	}
}