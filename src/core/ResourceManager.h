#pragma once

#include <memory>

#include "GameHeaders.h"

namespace TheGame
{
	class ResourceManager
	{
	public:
		std::unique_ptr<Font> MainFont;
	public:
		void Load();
		void Cleanup();
	};

	extern ResourceManager g_ResourceManager;
}