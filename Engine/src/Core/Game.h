#pragma once

#include "Core.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "EntityMgr.h"
#include "CommandMgr.h"
#include "World.h"
#include "SMemory.h"
#include "SRandom.h"
#include "LightMap.h"

#include "MapGeneration/MapGeneration.h"

struct UIState;

struct Game
{
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    
    Resources Resources;
    Renderer Renderer;
    TileMapRenderer TileMapRenderer;
    
    TileMgr TileMgr;
    CommandMgr CommandMgr;
    struct EntityMgr EntityMgr;
    LightData LightMap;

    MapGenerator MapGen;
    World World;

    Rectangle CullingRect;
    Rectangle UpdateRect;

    uint64_t Time;
    uint64_t CurrentDayProgress;

    float CameraLerpTime;

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

GameApplication* GetGameApp();
Game* GetGame();
Player* GetClientPlayer();

void SetCameraPosition(Game* game, Vector3 pos);
void SetCameraDistance(GameApplication* gameApp, float zoom);

SRandom* GetGlobalRandom();
float GetDeltaTime();
float GetScale();
Vector2 VecToTileCenter(Vector2 vec);
Vector2 GetZoomedMousePos(const Camera2D& camera);
Vector2i GetTileFromMouse(Game* game);
bool TileInsideCullRect(Vector2i tileCoord);
Vector2i TranslateTileWorldToCull(Vector2i pos);
Vector2i TranslateTileCullToWorld(Vector2i pos);
