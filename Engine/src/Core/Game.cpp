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
#include "SpriteAtlas.h"

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

    
    Game->Atlas.Load("assets/textures/atlas/tiles.atlas");

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
            if (IsKeyDown(KEY_L))
                SetCameraPosition(&Game->Camera3D,
                    { 512.0f * DeltaTime, 0, 0 });
            else if (IsKeyDown(KEY_J))
                SetCameraPosition(&Game->Camera3D,
                    { -512.0f * DeltaTime, 0, 0 });
            if (IsKeyDown(KEY_K))
                SetCameraPosition(&Game->Camera3D,
                    { 0, -512.0f * DeltaTime, 0 });
            else if (IsKeyDown(KEY_I))
                SetCameraPosition(&Game->Camera3D,
                    { 0, 512.0f * DeltaTime, 0 });

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
            float mouseWheel = GetMouseWheelMove();
            if (mouseWheel != 0)
            {
                SetCameraDistance(&Game->Camera3D, mouseWheel);
            }

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

        //BeginMode2D(Game->Camera);

        BeginMode3D(Game->Camera3D);

        WorldUpdate(&Game->World, this);

        EndMode3D();

        //EndMode2D();

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

void SetCameraPosition(Camera3D* camera, Vector3 pos)
{
    camera->target.x = floorf(camera->target.x + pos.x);
    camera->target.y = floorf(camera->target.y + pos.y);
    camera->target.z = floorf(camera->target.z + pos.z);
    camera->position.x = floorf(camera->position.x + pos.x);
    camera->position.y = floorf(camera->position.y + pos.y);
}

void SetCameraDistance(Camera3D* camera, float zoom)
{
    zoom = camera->position.z + zoom * 15.0f;
    if (zoom > 256.0f) zoom = 250.0f;
    if (zoom < 16.0f) zoom = 15.0f;

    camera->position.z = floorf(zoom);

    //Game->Camera.zoom += ((float)GetMouseWheelMove() * 0.2f);
    //if (Game->Camera.zoom > 3.0f) Game->Camera.zoom = 3.0f;
    //else if (Game->Camera.zoom < 0.2f) Game->Camera.zoom = 0.2f;
}

internal void HandleInput(GameApplication* gameApp) {}

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

    Camera3D camera = {};
    camera.position = { 0.0f, 0.0f, 100.0f };   // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };        // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };            // Camera up vector (rotation towards target)
    camera.fovy = 90.0f;                         // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;      // Camera mode type
    gameApp->Game->Camera3D = camera;
    SetCameraMode(gameApp->Game->Camera3D,
        gameApp->Game->Camera3D.projection);

    gameApp->Game->Camera.zoom = 2.0f;

    S_LOG_INFO("Game Initialized!");
    return true;
}
