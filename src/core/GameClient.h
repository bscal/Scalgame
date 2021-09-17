#pragma once

#include <memory>
#include <cstdint>

#include "GameHeaders.h"
#include "input/InputHandler.h"
#include "world/World.h"

namespace TheGame
{
	class GameClient
	{
	public:
		static GameClient& Instance()
		{
			static GameClient client;
			return client;
		}

		uint32_t ScreenWidth, ScreenHeight;
		std::shared_ptr<World> GameWorld;

		
	private:
		std::unique_ptr<InputHandler> m_InputHandler;
		std::unique_ptr<Camera2D> m_MainCamera;
	public:
		GameClient();
		int Start();

		inline InputHandler& GetInputHandler() { return *m_InputHandler; }
		inline Camera2D& GetMainCamera() { return *m_MainCamera; }
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


