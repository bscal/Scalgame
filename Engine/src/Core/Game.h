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
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    #endif

    SpriteAtlas Atlas;
    RenderTexture2D ScreenTexture;
    RenderTexture2D ScreenLightMapTexture;
    RenderTexture2D LightTexture;
    
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
Player* GetClientPlayer();
float GetDeltaTime();
float GetScale();

Vector2 ScaleWorldVec2(Vector2 vec);
Vector2i ScaleWorldVec2i(Vector2i vec);

internal void GameLoadScreen(GameApplication* gameApp, int width, int height);
internal bool GameInitialize(Game* game, GameApplication* gameApp);
internal void GameFree(Game* game);
internal void GameUpdate(Game* game, GameApplication* gameApp);
