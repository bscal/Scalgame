#pragma once

#include "Core.h"
#include "Globals.h"

#include "Structures/StaticArray.h"
#include "Structures/SList.h"

struct GameApplication;
struct Game;
struct Light;
struct UpdatingLight;

struct LightingState
{
    SList<Light> Lights;
    SList<UpdatingLight> UpdatingLights;

    size_t Size = CULL_TOTAL_TILES;
    StaticArray<bool, CULL_TOTAL_TILES> CheckedTiles;
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
LightsUpdateTileColor(int index, float distance, const Light& light);

void
LightsUpdateTileColorTile(Vector2i tileCoord, float distance, const Light& light);
