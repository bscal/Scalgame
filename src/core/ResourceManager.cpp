#include "ResourceManager.h"

namespace TheGame
{
	ResourceManager g_ResourceManager;


	void ResourceManager::Load()
	{
		MainFont = std::make_unique<Font>(LoadFont("assets/textures/fonts/Silver.ttf"));
	}

	void ResourceManager::Cleanup()
	{
		UnloadFont(*MainFont);
		MainFont.release();
	}
}