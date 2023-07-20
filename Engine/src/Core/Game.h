#pragma once

#include "Core.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "CommandMgr.h"
#include "Universe.h"
#include "SMemory.h"
#include "SRandom.h"
#include "Entity.h"
#include "Inventory.h"
#include "Lighting.h"

#include "MapGeneration/MapGeneration.h"

#include "rmem/rmem.h"

struct UIState;

struct Game
{
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    
    Resources Resources;
    Renderer Renderer;
    TileMapRenderer TileMapRenderer;
    LightingRenderer LightingRenderer;
    
    CommandMgr CommandMgr;

    InventoryMgr InventoryMgr;
    ItemDB ItemDB;

    LightingState LightingState;

    MapGenerator MapGen;
    
    PlayerClient Client;

    Universe Universe;

    uint64_t GameTick;
    float GameTimer;
    uint32_t CurrentDayProgress;

    float CameraLerpTime;

    bool IsPlayersTurn;
    bool IsFreeCam;
    bool IsInventoryOpen;

    bool DebugDisableDarkess;
    bool DebugDisableFOV;
    bool DebugTileView;
};

struct View
{
    Vector2i Resolution;
    Vector2i ResolutionInTiles;
    Vector2 ScreenCenter;
    Vector2 ScreenXY;
    Vector2i ScreenXYInTiles;
    int TotalTilesOnScreen;
};

struct GameApplication
{
    Game* Game;
    UIState* UIState;

    MemPool GameMemory;
    BiStack TemporaryMemory;
    
    SRandom GlobalRandom;

    View View;
    
    double RenderTime;
    double UpdateWorldTime;
    double DrawTime;
    double DebugLightTime;
    int NumOfLoadedChunks;
    int NumOfChunksUpdated;
    int NumOfLightsUpdated;

    float Scale;
    
    bool IsGUIInputDisabled;
    bool IsGameInputDisabled;
    bool IsHoveringGUIWindow;
    
    bool IsInitialized;
    bool IsRunning;
    
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
Vector2i GetTileFromMouse(Game* game);
bool TileInsideCullRect(Vector2i coord);

Vector2i WorldTileToCullTile(Vector2i coord);
Vector2i CullTileToWorldTile(Vector2i coord);