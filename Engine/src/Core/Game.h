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
    Camera2D ViewCamera;
    #endif

    SpriteAtlas Atlas;
    RenderTexture2D ScreenTexture;
    RenderTexture2D ScreenMapTexture;
    RenderTexture2D ScreenLightMapTexture;
    
    World World;
    
    Rectangle CurScreenRect;
    Vector2 HalfWidthHeight;
    Vector2i ChunkViewDistance;
    float CameraLerpTime;
    uint32_t Time;
    bool IsFreeCam;
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
    float Scale;
    bool IsInitialized;
    bool IsRunning;
    bool IsSuspended;

    SAPI bool Start();
    SAPI void Shutdown();
    SAPI void Run();
};

GameApplication* const GetGameApp();
void SetCameraPosition(Game* game, Vector3 pos);
void SetCameraDistance(GameApplication* gameApp, float zoom);
Scal::Creature::Player* GetClientPlayer();
float GetDeltaTime();
float GetScale();

Vector2 ScaleWorldVec2(Vector2 vec);
Vector2i ScaleWorldVec2i(Vector2i vec);

internal void LoadScreen(GameApplication* gameApp, int width, int height);
internal void UpdateGame(Game* game, GameApplication* gameApp);
internal bool InitializeGame(Game* game, GameApplication* gameApp);
internal void FreeGame(Game* game);
