#pragma once

#include "Core.h"
#include "ResourceManager.h"
#include "EntityMgr.h"
#include "CommandMgr.h"
#include "World.h"
#include "SMemory.h"
#include "SRandom.h"
#include "LightMap.h"

struct UIState;

struct Game
{
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    
    Resources Resources;
    Renderer Renderer;

    RenderTexture2D WorldTexture;
    RenderTexture2D EffectTextureOne;
    RenderTexture2D EffectTextureTwo;
    
    TileMgr TileMgr;
    CommandMgr CommandMgr;
    struct EntityMgr EntityMgr;
    LightData LightMap;
    
    World World;
    
    float CameraLerpTime;
    uint32_t Time;
    bool IsFreeCam;
    
    bool DebugDisableDarkess;
    bool DebugDisableFOV;
    bool DebugTileView;
    bool DebugTest;
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
    double UpdateWorldTime;
    size_t LastFrameTempMemoryUsage;
    int NumOfLoadedChunks;
    int NumOfChunksUpdated;

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
