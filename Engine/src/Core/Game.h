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
    Player Player;
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

void UpdateTime(GameApplication* gameApp, int timeChange);

