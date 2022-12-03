#include "Game.h"

#include "ResourceManager.h"
#include "SMemory.h"
#include "raymath.h"
#include "SUI.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "SUtil.h"
#include "Creature.h"

global_var GameApplication* GameAppPtr;

SAPI bool GameApplication::Start()
{
    if (IsInitialized)
    {
        TraceLog(LOG_ERROR, "GameApplication already initialized!");
        return false;
    }

    const int screenWidth = 1600;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "Some roguelike game");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_ALL);

    Resources = (struct Resources*)Scal::MemAllocZero(sizeof(struct Resources));
    InitializeResources(Resources);

    UIState = (struct UIState*)Scal::MemAllocZero(sizeof(struct UIState));
    InitializeUI(&Resources->MainFontS, 16.0f, UIState);
    InitiailizeDebugWindow(&Resources->MainFontM, 10, 30, DARKGREEN);

    Game = (struct Game*)Scal::MemAllocZero(sizeof(struct Game));
    if (!InitializeGame(this))
    {
        S_CRASH("Game failed to initialized!");
        return false;
    }


    GameAppPtr = this;

    Scal::Creature::TestCreature(Game);
    Test();
    TestSTable();
    TestHashes();
    return IsInitialized = true;
}

SAPI void GameApplication::Shutdown()
{
    CloseWindow();
}

internal void TestActionFunc(World* world, Action* action)
{
    TraceLog(LOG_INFO, "Testing!");
}

SAPI void GameApplication::Run()
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

        if (!UIState->IsMouseHoveringUI)
        {   
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

            // Camera reset (zoom and rotation)
            if (IsKeyPressed(KEY_R))
            {
                Game->Camera.zoom = 1.0f;
                Game->Camera.rotation = 0.0f;
            }

            // Camera zoom controls
            Game->Camera.zoom += ((float)GetMouseWheelMove() * 0.2f);

            if (Game->Camera.zoom > 3.0f) Game->Camera.zoom = 3.0f;
            else if (Game->Camera.zoom < 0.2f) Game->Camera.zoom = 0.2f;

            // Debug place tiles
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                // LocalToWorld
                Matrix invMatCamera =
                    MatrixInvert(GetCameraMatrix2D(Game->Camera));
                Vector3 transform = Vector3Transform(
                    { (float)GetMouseX(), (float)GetMouseY(), 0.0f }, invMatCamera);

                int x = transform.x / 16;
                int y = transform.y / 16;
                if (IsInBounds(x, y, 64, 64))
                {
                    //Tile tile = CreateTile(&Game->World.MainTileMap, 4);
                    //SetTile(&Game->World.MainTileMap, x, y, &tile);
                }
            }

            if (IsKeyPressed(KEY_ONE))
            {
                Action testAction = {};
                testAction.Cost = 100;
                testAction.ActionFunction = TestActionFunc;
                AddAction(&Game->World, &testAction);
            }
        }

        UpdateUI(UIState);
        Scal::ShowMemoryUsage(UIState);

        // ***************
        // Render
        // ***************
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(Game->Camera);

        WorldUpdate(&Game->World, this);

        EndMode2D();

        // ***************
        // UI
        // ***************
        UpdateDebugWindow();
        //BeginShaderMode(Resources.SDFFont.Shader);

        DisplayDebugText("Zoom = %.2f", Game->Camera.zoom);
        DisplayDebugText("cX = %.1f, cY = %.1f",
            Game->Camera.target.x, Game->Camera.target.y);
        const Scal::Creature::Player* p = Game->World.EntityMgr.GetPlayer(0);
        DisplayDebugText("pX = %d, pY = %d",
            p->Transform.TilePos.x, p->Transform.TilePos.y);
        //DisplayDebugText("Time = %d", Game->Time);
        //DisplayDebugText("Energy = %d/%d", Game->Player.Energy, Game->Player.MaxEnergy);
        //EndShaderMode();

        RenderUI(UIState);

        EndDrawing();


    }
    IsRunning = false;
}


GameApplication* const GetGameApp()
{
    assert(GameAppPtr);
    return GameAppPtr;
}

inline float GetDeltaTime()
{
    return GetFrameTime();
}

//void UpdateTime(GameApplication* gameApp, int timeChange)
//{
//    gameApp->Game->Time += timeChange;
//    gameApp->Game->Player.Energy = gameApp->Game->Player.MaxEnergy;
//
//    ProcessActions(&gameApp->Game->World);
//    gameApp->Game->World.EntityActionsList.Clear();
//}

internal bool InitializeGame(GameApplication* gameApp)
{
    CALL_CONSTRUCTOR(gameApp->Game) Game();

    WorldInitialize(&gameApp->Game->World, &gameApp->Resources->MainTileSet);

    gameApp->Game->Camera.zoom = 2.0f;

    S_LOG_INFO("Game Initialized!");
    return true;
}
