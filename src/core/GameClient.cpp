#include "GameClient.h"

#include <stdio.h>
#include <math.h>
#include <vector>

#include "tiles/GridTile.h"
#include "core/ResourceManager.h"
#include "input/Keybinds.h"

const Vector2 VEC2_ZERO = Vector2{ 0.0f, 0.0f };

namespace TheGame
{
	// TODO need to figure out World? Maybe play button creating a GameMode instance?
	GameClient::GameClient()
		: ScreenWidth(800), ScreenHeight(400)
	{
		// Initialization
		//--------------------------------------------------------------------------------------
		InitWindow(ScreenWidth, ScreenHeight, "raylib [text] example - sprite font loading");

		g_ResourceManager.Load();

		Init();
	}

	int GameClient::Start()
	{
		SetupGame();
		SetupCamera();

		Loop();

		Cleanup();
		return 0;
	}

	void GameClient::Init()
	{
		m_MainCamera = std::make_unique<Camera2D>(Camera2D{ 0 });
		m_MainCamera->target = { 0.0f, 0.0f };
		m_MainCamera->offset = { ScreenWidth / 2.0f, ScreenHeight / 2.0f };
		m_MainCamera->rotation = 0.0f;
		m_MainCamera->zoom = 1.0f;

		m_InputHandler = std::make_unique<InputHandler>(InputHandler());
		KeyBinds::Defaults();

		SetTargetFPS(60);
	}

	void GameClient::SetupGame()
	{
		GameWorld = std::make_shared<World>(World(48, 32, *this));
	}

	void GameClient::Loop()
	{
		//--------------------------------------------------------------------------------------
		// Main game loop
		while (!WindowShouldClose())
		{
			// Update
			//----------------------------------------------------------------------------------

			KeyBinds::Process();
			m_InputHandler->ProcessInputs();
		

			if (KeyBinds::GetBind("MoveUp").IsActive())
				TraceLog(LOG_INFO, "Moved up!");

			if (KeyBinds::GetBind("MoveDown").IsActive())
				TraceLog(LOG_INFO, "Moved down!");

			if (KeyBinds::GetBind("MoveLeft").IsActive())
				TraceLog(LOG_INFO, "Moved left!");


			if (KeyBinds::GetBind("MoveRight").IsActive())
				TraceLog(LOG_INFO, "Moved right!");

			// Draw
			//----------------------------------------------------------------------------------
			BeginDrawing();
				ClearBackground(RAYWHITE);

				BeginMode2D(*m_MainCamera);
					Update();
					Render();
				EndMode2D();

				RenderUI();
			EndDrawing();
			//----------------------------------------------------------------------------------
		}
	}

	void GameClient::Cleanup()
	{
		// De-Initialization
		//--------------------------------------------------------------------------------------
		g_ResourceManager.Cleanup();

		CloseWindow();
		//--------------------------------------------------------------------------------------
	}

	void GameClient::Render()
	{
	}

	void GameClient::RenderUI()
	{
	}

	void GameClient::Update()
	{
		

		GameWorld->Update();
	}

	void GameClient::SetupCamera()
	{
	}
}