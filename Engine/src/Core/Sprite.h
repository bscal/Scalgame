#pragma once

#include "Core.h"

#include "Structures/StaticArray.h"

struct Game;

union Sprite
{
    struct
    {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
    };
    uint16_t Region[4];
};

struct AnimatedSprite
{
    DynamicArray<uint16_t> Frames;
    uint8_t CurrentIdx;
    uint8_t CurrentTick;
    uint8_t TickSpeed;
    uint8_t StartIdx;
};

struct SpriteDB
{
#define MAX_SPRITES 512

    uint16_t NextId;
    uint16_t DefaultId;
    Sprite Sprites[MAX_SPRITES];
};

inline struct SpriteDB SpriteDB;

inline internal uint16_t SpriteRegister(Sprite rec)
{
    uint16_t id = SpriteDB.NextId++;
    SASSERT(id < MAX_SPRITES);

    if (id < MAX_SPRITES)
    {
        SpriteDB.Sprites[id] = rec;
        return id;
    }
    else
    {
        SLOG_ERR("[ SpriteDB ] ERROR: Registering to many sprites! Max is %d", MAX_SPRITES);
        return SpriteDB.DefaultId;
    }
}

void SpriteDBSetDefaultId(uint16_t id);
Sprite SpriteGet(uint16_t id);
Rectangle SpriteGetRec(uint16_t id);

AnimatedSprite AnimatedSpriteCreate(uint8_t startIdx, uint8_t tickSpeed, uint8_t frameCount, uint16_t frameIds...);
void AnimedSpriteTick(AnimatedSprite* sprite, Game* game);
Rectangle AnimatedSpriteRec(AnimatedSprite* sprite);

#define SPRITE_DEF(v, rec) const inline uint16_t v = SpriteRegister(rec)

constexpr internal Sprite 
SpriteRec(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    Sprite res = { x, y, w, h };
    return res;
}

namespace Sprites
{
    SPRITE_DEF(PLAYER, SpriteRec(0, 0, 16, 16));
    SPRITE_DEF(RAT, SpriteRec(16, 16, 16, 16));
    SPRITE_DEF(ROGUE, SpriteRec(16, 0, 16, 16));
    SPRITE_DEF(FIRE_STAFF, SpriteRec(4, 32, 4, 16));
    SPRITE_DEF(TORCH, SpriteRec(0, 32, 4, 4));
}

//struct SpriteAtlasT
//{
//    RenderTexture2D RenderTexture;
//    Sprite* Sprites;
//    uint32_t Length;
//    uint16_t NextIdx;
//    SHashMap<SRawString, uint16_t, SRawStringHasher> NameToSpriteIdx;
//};
//void SpriteAtlasLoadT(SpriteAtlasT* atlas, const char* directoryPath);
//void SpriteAtlasFreeT(SpriteAtlasT* atlas);
