#pragma once

#include "Core.h"
#include "TileMap.h"
#include "Player.h"
#include "World.h"

struct Resources;
struct UIState;

struct Game
{
    Camera2D Camera;
    World World;
    uint32_t Time;
};

struct GameApplication
{
    Game* Game;
    Resources* Resources;
    UIState* UIState;

    float DeltaTime;
    bool IsInitialized;
    bool IsRunning;
    bool IsSuspended;

    SAPI bool Start();
    SAPI void Shutdown();
    SAPI void Run();
};

GameApplication* const GetGameApp();
inline float GetDeltaTime();

void UpdateTime(GameApplication* gameApp, int timeChange);

internal bool InitializeGame(GameApplication* gameApp);
