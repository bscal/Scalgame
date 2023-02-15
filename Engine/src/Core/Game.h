#pragma once

#include "Core.h"
#include "Tile.h"
#include "Player.h"
#include "World.h"
#include "SpriteAtlas.h"
#include "ResourceManager.h"
#include "CommandMgr.h"
#include "SMemory.h"
#include "SRandom.h"

struct UIState;

struct Game
{
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    
    Resources Resources;
    RenderTexture2D WorldTexture;
    
    TileMgr TileMgr;
    CommandMgr CommandMgr;
    struct EntityMgr EntityMgr;
    
    World World;
    
    float CameraLerpTime;
    uint32_t Time;
    bool IsFreeCam;
    
    bool DebugDisableDarkess;
    bool DebugDisableFOV;
    bool DebugTileView;
};

struct GameApplication
{
    Game* Game;
    UIState* UIState;
    
    MemPool GameMemory;
    BiStack TemporaryMemory;
    
    SRandom GlobalRandom;

    Vector2 ScreenXY; // Camera top left corner
    Vector2 ScaledScreenXY;
    Vector2 HalfWidthHeight;
    
    double RenderTime;
    size_t LastFrameTempMemoryUsage;
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
Player* const GetClientPlayer();
Game* const GetGame();

SRandom* GetGlobalRandom();
float GetDeltaTime();
float GetScale();
Rectangle GetScaledScreenRect();
Vector2 VecToTileCenter(Vector2 vec);
Vector2 GetZoomedMousePos(const Camera2D& camera);
Vector2i GetTileFromMouse(Game* game);

internal void GameLoadScreen(GameApplication* gameApp, int width, int height);
internal bool GameInitialize(Game* game, GameApplication* gameApp);
internal void GameStart(Game* game, GameApplication* gameApp);
internal void GameFree(Game* game);

internal void GameUpdate(Game* game, GameApplication* gameApp);
internal void GameLateUpdate(Game* game);

internal void GameInputUpdate(Game* game, GameApplication* gameApp);

