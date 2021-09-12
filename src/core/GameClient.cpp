#include "GameClient.h"

#include <stdio.h>
#include <math.h>
#include <vector>

#include "tiles/GridTile.h"
#include "core/ResourceManager.h"

namespace TheGame
{
	Rectangle TextureTileToRect(const TextureTile& tile)
	{
		return Rectangle({ (float)tile.x * 16, (float)tile.y * 16, (float)tile.width, (float)tile.height
			});
	}

	// TODO need to figure out World? Maybe play button creating a GameMode instance?
	GameClient::GameClient()
		: ScreenWidth(800), ScreenHeight(400)
	{
		// Initialization
		//--------------------------------------------------------------------------------------
		InitWindow(ScreenWidth, ScreenHeight, "raylib [text] example - sprite font loading");

		g_ResourceManager.Load();
		Tilemap = LoadTexture("assets/textures/tiles/16x16.png");

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
		SetTargetFPS(60);
	}

	void GameClient::SetupGame()
	{
		GameWorld = std::make_shared<World>(World(48, 32));
	}

	void GameClient::Loop()
	{
		// ONLY HERE TO WORK
		// TODO combine new and old grids
		const size_t xTiles = GetScreenWidth() / 64;
		const size_t yTiles = GetScreenHeight() / 64;
		std::vector<GridTile> tiles(xTiles * yTiles);

		for (size_t y = 0; y < yTiles; y++)
		{
			for (size_t x = 0; x < xTiles; x++)
			{
				tiles[x + y * xTiles] = GridTile({ 7, 0, 16, 16 });
			}
		}

		//--------------------------------------------------------------------------------------
		// Main game loop
		while (!WindowShouldClose())
		{
			// Update
			//----------------------------------------------------------------------------------

			// Draw
			//----------------------------------------------------------------------------------
			BeginDrawing();

			ClearBackground(RAYWHITE);


			// TODO REMOVE
			for (size_t y = 0; y < yTiles; y++)
			{
				for (size_t x = 0; x < xTiles; x++)
				{
					GridTile tile = tiles[x + y * xTiles];

					float xx = x * 64.0f;
					float yy = y * 64.0f;
					Rectangle dest({ xx, yy, xx + 64, yy + 64 });
					Vector2 origin({ 0, 0 });

					TextureTile background = tile.GetBackground();
					DrawTextureTiled(Tilemap, TextureTileToRect(background), dest, origin, 0.0f, 4.0f, WHITE);

					TextureTile foreground = tile.GetForeground();
					if (foreground != BLANK_TILE)
					{
						DrawTextureTiled(Tilemap, TextureTileToRect(foreground), dest, origin, 0.0f, 4.0f, WHITE);
					}
				}
			}

			Render();
			RenderUI();

			Update();

			EndDrawing();
			//----------------------------------------------------------------------------------
		}
	}

	void GameClient::Cleanup()
	{
		// De-Initialization
		//--------------------------------------------------------------------------------------
		UnloadTexture(Tilemap);

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