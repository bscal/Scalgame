#include "Game.h"

#include <string>

ENGINE_API bool Game::Start()
{
    const int screenWidth = 1600;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Some roguelike game");
    SetTargetFPS(60);

    Camera.target = { 0, 0 };
    Camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    Camera.rotation = 0.0f;
    Camera.zoom = 2.0f;

    InitializeResources(&Resources);
    InitiailizeDebugWindow(&Resources.MainFont, 10, 30, DARKGREEN);

    InitializeTileMap(&Resources.MainTileSet,
        64,
        64,
        16,
        &MainTileMap
    );
    LoadTileMap(&MainTileMap);


    InitializePlayer(&Player, this);
   
    return true;
}

ENGINE_API void Game::Shutdown()
{
    CloseWindow();
}

ENGINE_API void Game::Run()
{
    TraceLog(LOG_INFO, "Game Running...");

    while (!WindowShouldClose())
    {
        DeltaTime = GetFrameTime();

        // ***************
        // Update
        // ***************

        if (IsKeyDown(KEY_L)) Camera.target.x += (int)(512.0f * DeltaTime);
        else if (IsKeyDown(KEY_J)) Camera.target.x -= (int)(512.0f * DeltaTime);
        if (IsKeyDown(KEY_K)) Camera.target.y += (int)(512.0f * DeltaTime);
        else if (IsKeyDown(KEY_I)) Camera.target.y -= (int)(512.0f * DeltaTime);

        // Camera rotation controls
        if (IsKeyDown(KEY_Q)) Camera.rotation--;
        else if (IsKeyDown(KEY_E)) Camera.rotation++;

        // Limit camera rotation to 80 degrees (-40 to 40)
        if (Camera.rotation > 40) Camera.rotation = 40;
        else if (Camera.rotation < -40) Camera.rotation = -40;

        // Camera zoom controls
        Camera.zoom += ((float)GetMouseWheelMove() * 0.2f);

        if (Camera.zoom > 3.0f) Camera.zoom = 3.0f;
        else if (Camera.zoom < 0.2f) Camera.zoom = 0.2f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            Camera.zoom = 1.0f;
            Camera.rotation = 0.0f;
        }

        UpdatePlayer(&Player, this);

        // ***************
        // Render
        // ***************
        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        UpdateDebugWindow();

        BeginMode2D(Camera);

        RenderTileMap(this, &MainTileMap);
        RenderPlayer(&Player, this);

        EndMode2D();

        // ***************
        // UI
        // ***************

        //BeginShaderMode(Resources.SDFFont.Shader);

        DisplayDebugText("Zoom = %.2f", Camera.zoom);
        DisplayDebugText("cX = %.1f, cY = %.1f", Camera.target.x, Camera.target.y);
        DisplayDebugText("pX = %.1f, pY = %.1f", Player.Position.x / 16.0f, Player.Position.y / 16.0f);

        //EndShaderMode();



        EndDrawing();
    }
}

Game* CreateGame()
{
    return (Game*)MemAlloc(sizeof(Game));
}