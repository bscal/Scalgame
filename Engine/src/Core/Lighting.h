#pragma once

#include "Core.h"
#include "Vector2i.h"

#include "Structures/StaticArray.h"
#include "Structures/SList.h"
#include "Structures/IndexArray.h"

struct GameApplication;
struct Game;
struct ChunkedTileMap;
struct Light;
struct UpdatingLight;
struct UpdatingLightSource;

struct Slope
{
    int y;
    int x;

    inline bool Greater(int y, int x) { return this->y * x > this->x * y; }; // this > y/x
    inline bool GreaterOrEqual(int y, int x) { return this->y * x >= this->x * y; } // this >= y/x
    inline bool Less(int y, int x) { return this->y * x < this->x * y; } // this < y/x
};

// Used to translate tile coordinates 
constexpr global_var int TranslationTable[8][4] =
{
    {  1,  0,  0, -1 },
    {  0,  1, -1,  0 },
    {  0, -1, -1,  0 },
    { -1,  0,  0, -1 },
    { -1,  0,  0,  1 },
    {  0, -1,  1,  0 },
    {  0,  1,  1,  0 },
    {  1,  0,  0,  1 },
};

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

enum class StaticLightTypes : uint8_t
{
    Basic = 0,
    Lava,
    MaxTypes
};

struct StaticLight : public Light
{
    StaticLightTypes StaticLightType;
};

struct LightingState
{
    SList<UpdatingLight> UpdatingLights;
    SList<StaticLight> StaticLights;
    SList<StaticLightType> StaticLightTypes;

    IndexArray<UpdatingLight> Lights;

    Vector2 PlayerLookVector; // TODO maybe move this
    uint32_t NumOfLightsUpdatedThisFrame;

    StaticArray<bool, CULL_TOTAL_TILES> CheckedTiles;
};

void DrawStaticLights(ChunkedTileMap* tilemap, const StaticLight* light);
void DrawStaticTileLight(Vector2i tilePos, Color color, StaticLightTypes type);
void DrawStaticLavaLight(Vector2i tilePos, Color color);

void LightsInitialize(LightingState* lightingState);
void LightsAddUpdating(const UpdatingLight& light);
void LightsAddStatic(const StaticLight& light);

void LightsUpdate(LightingState* lightingState, Game* game);
uint32_t GetNumOfLights();

void LightsUpdateTileColor(int index, float distance, const Light* light);
void LightsUpdateTileColorTile(Vector2i tileCoord, float distance, const Light* light);
//void DrawLightWithShadows(Vector2 pos, const UpdatingLightSource& light);
bool FloodFillLighting(ChunkedTileMap* tilemap, Light* light);
void FloodFillScanline(const Light* light, int x, int y, int width, int height, bool diagonal);//, bool (*test)(int, int)), void (*paint)(int, int))