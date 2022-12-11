#pragma once

#include "Core.h"
#include "TileMap.h"
#include "Player.h"
#include "World.h"
#include "SpriteAtlas.h"

struct Resources;
struct UIState;

struct Game
{
    #if Mode3D
    Camera3D Camera;
    #else
    Camera2D Camera;
    #endif
    bool IsFreeCam;
    World World;
    SpriteAtlas Atlas;
    Rectangle CurScreenRect;
    Rectangle EndScreenRect;
    uint32_t Time;
};

struct GameApplication
{
    Game* Game;
    Resources* Resources;
    UIState* UIState;

    double RenderTime;
    double LOSTime;
    int NumOfLoadedChunks;
    int NumOfChunksUpdated;

    float DeltaTime;
    bool IsInitialized;
    bool IsRunning;
    bool IsSuspended;

    SAPI bool Start();
    SAPI void Shutdown();
    SAPI void Run();
};

GameApplication* const GetGameApp();
void SetCameraPosition(Game* game, Vector3 pos);
void SetCameraDistance(Game* game, float zoom);
Scal::Creature::Player* GetClientPlayer();
float GetDeltaTime();

internal void UpdateGame(Game* game, GameApplication* gameApp);
internal bool InitializeGame(Game* game, GameApplication* gameApp);


void UpdateTime(GameApplication* gameApp, int timeChange);

