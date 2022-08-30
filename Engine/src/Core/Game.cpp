#include "Game.h"

ENGINE_API bool Game::Start()
{
    InitWindow(800, 450, "Some roguelike game");

    InitializeResources(&Engine.Resources);

    InitializeTileMap(
        &Engine.Resources.MainTileMapTexture,
        64,
        64,
        16,
        &MainTileMap
    );
    LoadTileMap(&MainTileMapTexture);
   
    return true;
}

ENGINE_API void Game::Shutdown()
{
    CloseWindow();
}

ENGINE_API void Game::Run()
{
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        RenderTileMap(this, &MainTileMap);

        EndDrawing();
    }
}

Game* CreateGame()
{
    return (Game*)MemAlloc(sizeof(Game));
}