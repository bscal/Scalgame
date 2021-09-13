#pragma once

#include <memory>
#include <cstdint>

#include "GameHeaders.h"
#include "world/World.h"

namespace TheGame
{
	class GameClient
	{
	public:
		uint32_t ScreenWidth, ScreenHeight;
		std::shared_ptr<World> GameWorld;
		//Camera2D MainCamera;
	public:
		GameClient();
		int Start();

	private:
		void Init();
		void SetupGame();
		void Loop();
		void Cleanup();

		void Render();
		void RenderUI();
		void Update();
		void SetupCamera();
	};
}


