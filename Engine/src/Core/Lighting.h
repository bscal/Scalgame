#pragma once

#include "Core.h"
#include "Structures/SList.h"
#include "Structures/SSet.h"

struct GameApplication;
struct Game;
namespace CTileMap { struct ChunkedTileMap; }
struct Light;
struct UpdatingLight;

struct LightingState
{
    SList<Light> Lights;
    SList<UpdatingLight> UpdatingLights;
    SSet<Vector2i> VisitedTilePerLight;
};

struct Light
{
    Vector2 Pos;
    Color Color;
    float Intensity;
    float Radius;
};

struct UpdatingLight : public Light
{
    static constexpr float UPDATE_RATE = 0.2f;

    struct Color Colors[4];
    float MinIntensity;
    float MaxIntensity;
    float LastUpdate;

    void Update(Game* game);
};

void LightsInitialized(GameApplication* gameApp);
void LightsAdd(const Light& light);
void LightsAddUpdating(const UpdatingLight& light);

void LightsUpdate(Game* game);
size_t GetNumOfLights();

void
LightsUpdateTileColor(CTileMap::ChunkedTileMap* tilemap,
    TileCoord tileCoord, float distance, const Light& light);
