#pragma once

#include "Core.h"

#include "Structures/SHashMap.h"

struct Game;

typedef int AnimationId;

#define SPRITE_EMTPY { UINT16_MAX, UINT16_MAX, 0, 0 }

union Sprite
{
    uint16_t Region[4];
    struct
    {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
    };
};

struct SImage
{
    Sprite Src;
    Sprite Dst;
    Vector2 Origin;
};

struct Animation
{
    Sprite* Frames;
    uint16_t FramesCount;
    uint16_t CurrentIdx;
    uint16_t TickSpeed;
};

struct Animator
{
    SHashMap<AnimationId, Animation> Animations;
    Animation* IdleAnimation;
    Animation* CurrentAnimation;
    uint16_t CurrentCycleTick;

    void Update(Game* game);
    void SetAnimation(AnimationId animId);
    Sprite GetAnimation() const;
};

Animation CreateAnimation(uint16_t framesCount, uint16_t tickSpeed, Sprite frames...);

namespace Sprites
{
constexpr global_var Sprite PLAYER_SPRITE = { 0, 0, 16, 16 };
constexpr global_var Sprite RAT_SPRITE = { 16, 16, 16, 16 };
constexpr global_var Sprite ROGUE_SPRITE = { 16, 0, 16, 16 };

constexpr global_var Sprite FIRE_STAFF = { 4, 32, 4, 16 };

}
