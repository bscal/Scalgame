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
    uint32_t Time;
};

struct GameApplication
{
    Game* Game;
    Resources* Resources;
    UIState* UIState;

    
    double RenderTime;
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
inline float GetDeltaTime();

internal void HandleInput(GameApplication* gameApp);
internal bool InitializeGame(Game* game, GameApplication* gameApp);


void UpdateTime(GameApplication* gameApp, int timeChange);

