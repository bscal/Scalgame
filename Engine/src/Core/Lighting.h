#pragma once

#include "Core.h"
#include "Globals.h"

#include "Structures/StaticArray.h"
#include "Structures/SList.h"

struct GameApplication;
struct Game;
struct ChunkedTileMap;
struct Light;
struct UpdatingLight;

struct Light
{
    Vector2 Pos;
    float Radius;
    Color Color;
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

struct StaticLightType
{
    SList<float> LightModifers;
    int8_t x;
    int8_t y;
    uint8_t Width;
    uint8_t Height;
};

global_var int STATIC_LIGHT;
global_var int STATIC_LIGHT_LAVA;

struct StaticLight : public Light
{
    uint8_t StaticLightTypeId;
};

uint32_t RegisterStaticLightType(const StaticLightType* type);
void DrawStaticLights(ChunkedTileMap* tilemap, const StaticLight* light);
void DrawStaticTileLight(Vector2i tilePos, int staticLightType, Color color);

void LightsInitialize(GameApplication* gameApp);
void LightsAddUpdating(const UpdatingLight& light);
void LightsAddStatic(const StaticLight& light);

void LightsUpdate(Game* game);
uint32_t GetNumOfLights();

void
LightsUpdateTileColor(int index, float distance, const Light* light);

void
LightsUpdateTileColorTile(Vector2i tileCoord, float distance, const Light* light);

bool FloodFillLighting(ChunkedTileMap* tilemap, Light* light);
void FloodFillScanline(const Light* light, int x, int y, int width, int height, bool diagonal);//, bool (*test)(int, int)), void (*paint)(int, int))