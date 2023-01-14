#pragma once

#include "Core.h"
#include "Tile.h"
#include "Player.h"
#include "World.h"
#include "SpriteAtlas.h"
#include "ResourceManager.h"
#include "CommandMgr.h"
#include "SMemory.h"

struct UIState;

struct Game
{
    Camera2D WorldCamera; // Camera world to rendered at
    Camera2D ViewCamera;  // Camera for viewport and scaled
    
    Resources Resources;
    RenderTexture2D WorldTexture;
    
    TileMgr TileMgr;
    CommandMgr CommandMgr;
    EntityMgr EntityMgr;
    
    World World;
    
    Vector2i ChunkViewDistance;
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

inline GameApplication* const GetGameApp();
inline void SetCameraPosition(Game* game, Vector3 pos);
inline void SetCameraDistance(GameApplication* gameApp, float zoom);
inline Player* const GetClientPlayer();
Game* const GetGame();
inline float GetDeltaTime();
inline float GetScale();
Rectangle GetScaledScreenRect();
inline Vector2 VecToTileCenter(Vector2 vec);
inline Vector2 ScaleWorldVec2(Vector2 vec);
inline Vector2i ScaleWorldVec2i(Vector2i vec);

internal void GameLoadScreen(GameApplication* gameApp, int width, int height);
internal bool GameInitialize(Game* game, GameApplication* gameApp);
internal void GameStart(Game* game, GameApplication* gameApp);
internal void GameFree(Game* game);
internal void GameUpdate(Game* game, GameApplication* gameApp);

internal void GameInputUpdate(Game* game, GameApplication* gameApp);

