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

typedef void(*LightUpdate)(uint8_t type, void* light);

struct Light
{
    LightUpdate UpdateFunc;
    Vector2 Pos;
    float Radius;
    Color Color;
    uint8_t LightType;
};

struct UpdatingLight : public Light
{
    constexpr static float UPDATE_RATE = 0.2f;

    struct Color Colors[4]; // Colors chosen for light;
    float MinIntensity;     // Min Radius 
    float MaxIntensity;     // Max Radius - Set to 0 to always use Min Radius
    float LastUpdate;       // Keeps track of internal update
    uint32_t EntityId;      // Entity to sync positions with, ENTITY_NO_POS to disable
    bool UseMultiColor;     // If false uses Color[0] only

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
    constexpr static size_t LIGHTS_CHUNK_SIZE = AlignPowTwo64Ceil(sizeof(StaticLight) * 256);

    IndexArray<Light*> LightPtrs;

    MemoryPool<UpdatingLight, 8192> UpdatingLightPool;
    MemoryPool<StaticLight, 8192> StaticLightPool;

    SList<UpdatingLight> UpdatingLights;
    SList<StaticLight> StaticLights;
    SList<StaticLightType> StaticLightTypes;

    IndexArray<UpdatingLight> Lights;

    Vector2 PlayerLookVector; // TODO maybe move this
    uint32_t NumOfLightsUpdatedThisFrame;

    StaticArray<bool, CULL_TOTAL_TILES> CheckedTiles;
};

void ProcessLights(LightingState* lightState);

uint32_t LightAddUpdating(LightingState* lightState, UpdatingLight* light);
void LightRemove(LightingState* lightState, uint32_t lightId);



uint32_t LightAddEntityLight();

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