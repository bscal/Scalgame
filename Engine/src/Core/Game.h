#pragma once

#include "Core.h"
#include "TileMap.h"
#include "ResourceManager.h"
#include "Player.h"

struct Game
{
    Resources Resources;
    TileMap MainTileMap;
    Camera2D Camera;

    Player Player;

    float DeltaTime;

    ENGINE_API bool Start();
    ENGINE_API void Shutdown();
    ENGINE_API void Run();
};

ENGINE_API Game* CreateGame();