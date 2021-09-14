#include "GameClient.h"

#include <stdio.h>
#include <math.h>
#include <vector>

#include "tiles/GridTile.h"
#include "core/ResourceManager.h"

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
		m_InputHandler = std::make_unique<InputHandler>(InputHandler());

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

			m_InputHandler->ProcessInputs();


			// Draw
			//----------------------------------------------------------------------------------
			BeginDrawing();

			ClearBackground(RAYWHITE);

			Update();
			Render();
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