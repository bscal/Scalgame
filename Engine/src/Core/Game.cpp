#include "Game.h"

ENGINE_API bool GameApplication::Start()
{
    if (IsInitialized)
    {
        TraceLog(LOG_ERROR, "GameApplication already initialized!");
        return false;
    }

    const int screenWidth = 1600;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Some roguelike game");
    //SetTargetFPS(60);

    Resources = (struct Resources*)MemAlloc(sizeof(struct Resources));
    InitializeResources(Resources);

    InitiailizeDebugWindow(&Resources->MainFont, 10, 30, DARKGREEN);

    Game = (struct Game*)MemAlloc(sizeof(struct Game));
    Game->Camera.target = { 0, 0 };
    Game->Camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
    Game->Camera.rotation = 0.0f;
    Game->Camera.zoom = 2.0f;

    InitializeTileMap(&Resources->MainTileSet,
        64,
        64,
        16,
        &Game->World.MainTileMap
    );
    LoadTileMap(&Game->World.MainTileMap);

    InitializePlayer(this, &Game->Player);
   
    return IsInitialized = true;
}

ENGINE_API void GameApplication::Shutdown()
{
    CloseWindow();
}

ENGINE_API void GameApplication::Run()
{
    TraceLog(LOG_INFO, "Game Running...");
    IsRunning = true;

    while (!WindowShouldClose())
    {
        DeltaTime = GetFrameTime();

        if (IsSuspended) continue;

        // ***************
        // Update
        // ***************

        if (IsKeyDown(KEY_L)) Game->Camera.target.x += 512.0f * DeltaTime;
        else if (IsKeyDown(KEY_J)) Game->Camera.target.x -= 512.0f * DeltaTime;
        if (IsKeyDown(KEY_K)) Game->Camera.target.y += 512.0f * DeltaTime;
        else if (IsKeyDown(KEY_I)) Game->Camera.target.y -= 512.0f * DeltaTime;

        // Camera rotation controls
        if (IsKeyDown(KEY_Q)) Game->Camera.rotation--;
        else if (IsKeyDown(KEY_E)) Game->Camera.rotation++;

        // Limit camera rotation to 80 degrees (-40 to 40)
        if (Game->Camera.rotation > 40) Game->Camera.rotation = 40;
        else if (Game->Camera.rotation < -40) Game->Camera.rotation = -40;

        // Camera zoom controls
        Game->Camera.zoom += ((float)GetMouseWheelMove() * 0.2f);

        if (Game->Camera.zoom > 3.0f) Game->Camera.zoom = 3.0f;
        else if (Game->Camera.zoom < 0.2f) Game->Camera.zoom = 0.2f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            Game->Camera.zoom = 1.0f;
            Game->Camera.rotation = 0.0f;
        }

        UpdatePlayer(this, &Game->Player);

        // ***************
        // Render
        // ***************
        BeginDrawing();
        ClearBackground(RAYWHITE);
       
        BeginMode2D(Game->Camera);

        RenderTileMap(Game, &Game->World.MainTileMap);
        RenderPlayer(this, &Game->Player);

        EndMode2D();

        // ***************
        // UI
        // ***************

        UpdateDebugWindow();
        //BeginShaderMode(Resources.SDFFont.Shader);

        DisplayDebugText("Zoom = %.2f", Game->Camera.zoom);
        DisplayDebugText("cX = %.1f, cY = %.1f", Game->Camera.target.x, Game->Camera.target.y);
        DisplayDebugText("pX = %.1f, pY = %.1f", Game->Player.Position.x / 16.0f, Game->Player.Position.y / 16.0f);
        DisplayDebugText("Time = %d", Game->Time);
        DisplayDebugText("Energy = %d/%d", Game->Player.Energy, Game->Player.MaxEnergy);
        //EndShaderMode();

        EndDrawing();
    }
    IsRunning = false;
}

void UpdateTime(GameApplication* gameApp, int timeChange)
{
    gameApp->Game->Time += timeChange;
    gameApp->Game->Player.Energy = gameApp->Game->Player.MaxEnergy;
}
