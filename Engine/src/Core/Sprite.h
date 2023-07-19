#pragma once

#include "Core.h"

#include "Structures/SHashMap.h"
#include "Structures/StaticArray.h"

struct Game;

#define SPRITE_EMTPY { UINT16_MAX, UINT16_MAX, 0, 0 }

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

struct Animation
{
    DynamicArray<uint16_t> Frames;
    uint16_t CurrentIdx;
    uint16_t TickSpeed;
};

struct TileSprite
{
    Animation Anim;
    uint16_t Time;
};

struct Animator
{
    SHashMap<uint16_t, Animation> Animations;
    Animation* IdleAnimation;
    Animation* CurrentAnimation;
    uint16_t CurrentCycleTick;

    void Update(Game* game);
    void SetAnimation(uint16_t animId);
    Sprite GetAnimation() const;
};

Animation CreateAnimation(uint16_t tickSpeed, uint16_t framesCount, Sprite frames...);

void SpritesInitialize(GameApplication* gameApp, const char* spriteDirPath);
void SpritesFree();

Sprite GetTileSprite(uint16_t spriteId);
uint16_t GetAnimationsByName(SRawString name);

namespace Sprites
{
constexpr global_var Sprite PLAYER_SPRITE = { 0, 0, 16, 16 };
constexpr global_var Sprite RAT_SPRITE = { 16, 16, 16, 16 };
constexpr global_var Sprite ROGUE_SPRITE = { 16, 0, 16, 16 };

constexpr global_var Sprite FIRE_STAFF = { 4, 32, 4, 16 };

}

struct SpriteAtlasT
{
    RenderTexture2D RenderTexture;
    Sprite* Sprites;
    uint32_t Length;
    uint16_t NextIdx;
    SHashMap<SRawString, uint16_t, SRawStringHasher> NameToSpriteIdx;
};

void SpriteAtlasLoadT(SpriteAtlasT* atlas, const char* directoryPath);
void SpriteAtlasFreeT(SpriteAtlasT* atlas);
