#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "SMemory.h"
#include "SUtil.h"

#include "Structures/StaticArray.h"
#include "Structures/SList.h"
#include "Structures/IndexArray.h"

struct GameApplication;
struct Game;
struct ChunkedTileMap;
struct TileMapChunk;
struct Light;
struct UpdatingLight;
struct UpdatingLightSource;

typedef void(*LightUpdate)(Light* light, Game* game, float dt);

enum class LightType : uint8_t
{
    Updating = 0,
    Static
};

struct Light
{
    LightUpdate UpdateFunc;
    Vector2i Pos;
    float Radius;
    Color Color;
    LightType LightType;
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
    constexpr static size_t UpdatingLightSize = AlignPowTwo64Ceil(sizeof(UpdatingLight) * 64);

    IndexArray<Light*> LightPtrs;
    MemoryPool<UpdatingLight, UpdatingLightSize> UpdatingLightPool;

    StaticArray<StaticLightType, (size_t)StaticLightTypes::MaxTypes> StaticLightTypes;

    uint32_t NumOfUpdatingLights;
};

void LightsInitialize(LightingState* lightingState);
uint32_t LightAddUpdating(LightingState* lightState, UpdatingLight* light);
uint32_t GetNumOfLights();
void LightRemove(LightingState* lightState, uint32_t lightId);
void StaticLightDrawToChunk(StaticLight* light, TileMapChunk* chunkDst, ChunkedTileMap* tilemap);
void LightsUpdate(LightingState* lightingState, Game* game);

// Types
struct Slope
{
    int y;
    int x;

    _FORCE_INLINE_ bool Greater(int y, int x) { return this->y * x > this->x * y; } // this > y/x
    _FORCE_INLINE_ bool GreaterOrEqual(int y, int x) { return this->y * x >= this->x * y; } // this >= y/x
    _FORCE_INLINE_ bool Less(int y, int x) { return this->y * x < this->x * y; } // this < y/x
};

constexpr global_var Vector2i LavaLightOffsets[9] =
{
    {-1, -1}, {0, -1}, {1, -1},
    {-1,  0}, {0,  0}, {1,  0},
    {-1,  1}, {0,  1}, {1,  1},
};

constexpr global_var float Inverse = 1.0f / 255.0f;

constexpr global_var float LavaLightWeightsInverse[9] =
{
        0.05f * Inverse, 0.15f * Inverse, 0.05f * Inverse,
        0.15f * Inverse, 0.25f * Inverse, 0.15f * Inverse,
        0.05f * Inverse, 0.15f * Inverse, 0.05f * Inverse,
};

constexpr global_var float LavaLightWeights[9] =
{
        0.05f, 0.15f, 0.05f,
        0.15f, 0.25f, 0.15f,
        0.05f, 0.15f, 0.05f,
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
