#pragma once

#include "Core.h"
#include "TileMap.h"
#include "Player.h"
#include "World.h"
#include "SpriteAtlas.h"
#include "ResourceManager.h"

struct UIState;

struct Game
{
    #if Mode3D
    Camera3D Camera;
    #else
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    #endif

    Resources Resources;
    SpriteAtlas Atlas;
    RenderTexture2D WorldTexture;
    
    World World;
    TileMgr TileMgr;
    EntityMgr EntityMgr;
    
    Vector2i ChunkViewDistance;
    float CameraLerpTime;
    uint32_t Time;
    bool IsFreeCam;

    bool DebugDisableDarkess;
    bool DebugDisableFOV;
};

struct GameApplication
{
    Game* Game;
    UIState* UIState;

    Vector2 ScreenXY; // Camera top left corner
    Vector2 ScaledScreenXY;
    Vector2 HalfWidthHeight;

    double RenderTime;
    int NumOfLoadedChunks;
    int NumOfChunksUpdated;

    float DeltaTime;
    float Scale;

    bool IsGameInputDisabled;
    bool IsHoveringGUIWindow;

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
EntityMgr* GetEntityMgr();
Game* GetGame();
float GetDeltaTime();
float GetScale();
Rectangle GetScaledScreenRect();

Vector2 VecToTileCenter(Vector2 vec);

Vector2 ScaleWorldVec2(Vector2 vec);
Vector2i ScaleWorldVec2i(Vector2i vec);

internal void GameLoadScreen(GameApplication* gameApp, int width, int height);
internal bool GameInitialize(Game* game, GameApplication* gameApp);
internal void GameStart(Game* game, GameApplication* gameApp);
internal void GameFree(Game* game);
internal void GameUpdate(Game* game, GameApplication* gameApp);

internal void GameInputUpdate(Game* game, GameApplication* gameApp);
