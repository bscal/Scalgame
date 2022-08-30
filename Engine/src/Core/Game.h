#pragma once

#include "Core.h"
#include "TileMap.h"
#include "ResourceManager.h"

struct Resources;

struct GameEngine
{
    Resources Resources;
};

struct Game
{
    GameEngine Engine;
    TileMap MainTileMap;

    ENGINE_API bool Start();
    ENGINE_API void Shutdown();
    ENGINE_API void Run();
};

ENGINE_API Game* CreateGame();