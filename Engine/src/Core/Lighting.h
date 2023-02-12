#pragma once

#include "Core.h"
#include "ChunkedTileMap.h"

struct GameApplication;
struct Game;

struct Light
{
    Vector2 Pos;
    Color Color;
    float Intensity;
    float Radius;
    int Id;
};

void LightsInitialized(GameApplication* gameApp);
void LightsAdd(const Light& light);

void LightsUpdate(Game* game);

void
LightsUpdateTileColor(CTileMap::ChunkedTileMap* tilemap,
    TileCoord tileCoord, float distance, const Light& light);
