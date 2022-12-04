#pragma once

#include "Core.h"
#include "TileMap.h"
#include "Player.h"
#include "World.h"
#include "Renderer.h"
#include "SpriteAtlas.h"

struct Resources;
struct UIState;

struct Game
{
    Camera Camera3D;
    Camera2D Camera;
    World World;
    SpriteAtlas Atlas;
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
void SetCameraPosition(Camera3D* camera, Vector3 pos);
void SetCameraDistance(Camera3D* camera, float zoom);

internal void HandleInput(GameApplication* gameApp);

inline float GetDeltaTime();

void UpdateTime(GameApplication* gameApp, int timeChange);

internal bool InitializeGame(GameApplication* gameApp);
