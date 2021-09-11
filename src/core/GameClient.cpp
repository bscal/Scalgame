#include "GameClient.h"

#include <stdio.h>
#include <math.h>
#include <vector>

#include "src/tiles/GridTile.h"
#include "src/grid/Grid.h"


Rectangle TextureTileToRect(const TextureTile& tile)
{
	return Rectangle({ (float)tile.x * 16, (float)tile.y * 16, (float)tile.width, (float)tile.height
		});
}

GameClient::GameClient()
	: ScreenWidth(800), ScreenHeight(400)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	InitWindow(ScreenWidth, ScreenHeight, "raylib [text] example - sprite font loading");

	MainGameFont = LoadFont("assets/textures/fonts/Silver.ttf");
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
	const size_t xTiles = GetScreenWidth() / 64;
	const size_t yTiles = GetScreenHeight() / 64;
	std::vector<GridTile> tiles(xTiles * yTiles);

	for (size_t y = 0; y < yTiles; y++)
	{
		for (size_t x = 0; x < xTiles; x++)
		{
			tiles[x * y] = GridTile({ 7, 0, 16, 16 });
		}
	}

	SetTargetFPS(60);
}

void GameClient::SetupGame()
{
}

void GameClient::Loop()
{
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

		Render();
		RenderUI();

		Update();

		// TODO combine new and old grids
		const size_t xTiles = GetScreenWidth() / 64;
		const size_t yTiles = GetScreenHeight() / 64;
		for (size_t y = 0; y < yTiles; y++)
		{
			for (size_t x = 0; x < xTiles; x++)
			{
				GridTile tile = tiles[x * y];

				float xx = x * 64;
				float yy = y * 64;
				Rectangle dest({ xx, yy, xx + 64, yy + 64 });
				Vector2 origin({ 0, 0 });

				TextureTile background = tile.GetBackground();
				DrawTextureTiled(tilemap, TextureTileToRect(background), dest, origin, 0.0f, 4.0f, WHITE);

				TextureTile foreground = tile.GetForeground();
				if (foreground != BLANK_TILE)
				{
					DrawTextureTiled(tilemap, TextureTileToRect(foreground), dest, origin, 0.0f, 4.0f, WHITE);
				}
			}
		}
		EndDrawing();
		//----------------------------------------------------------------------------------
	}
}

void GameClient::Cleanup()
{
	// De-Initialization
	//--------------------------------------------------------------------------------------
	UnloadFont(MainGameFont);
	UnloadTexture(Tilemap);

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
	World.Update();
}

void GameClient::SetupCamera()
{
}
