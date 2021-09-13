#pragma once

#include <memory>

#include "GameHeaders.h"

namespace TheGame
{
	class ResourceManager
	{
	public:
		std::unique_ptr<Font> MainFont;
		std::unique_ptr<Texture2D> TileMap;
	public:
		void Load();
		void Cleanup();
	};

	extern ResourceManager g_ResourceManager;
}