#pragma once

#include "Core.h"
#include "Structures/SList.h"
#include "Structures/SSet.h"
#include "Structures/SHoodSet.h"

struct GameApplication;
struct Game;
struct Light;
struct UpdatingLight;

struct LightingState
{
    SList<Light> Lights;
    SList<UpdatingLight> UpdatingLights;
    SHoodSet<Vector2i, DefaultHasher<Vector2i>, Vector2iEquals> VisitedTilePerLight;
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
LightsUpdateTileColor(TileCoord tileCoord, float distance, const Light& light);
