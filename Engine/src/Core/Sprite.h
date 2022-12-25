#pragma once

#include "Core.h"
#include "SHash.hpp"

#include <string>
#include <string_view>
#include <unordered_map>

struct Sprite;
struct Animation;

global_var std::unordered_map<uint32_t, Animation> Animations;

struct Sprite
{
    Rectangle TexCoord;
};

struct EntitySpriteSheet
{
    Texture2D Texture;
};

struct Animation
{
    const Rectangle* const Frames;
    uint8_t FrameCount;
    uint8_t FrameSpeed;
    uint8_t FrameProgress;
    uint8_t FrameIndex;
};

struct AnimatedSprite
{
    Animation CurrentAnimation;
    bool Loop;

    inline const Rectangle& CurrentTextCoord() const
    {
        return CurrentAnimation.Frames[CurrentAnimation.FrameIndex];
    }

    void Update();
};

inline constexpr Sprite AsSprite(const Rectangle& rect)
{
    Sprite sprite = {};
    sprite.TexCoord = rect;
    return sprite;
}

inline constexpr uint32_t HashName(std::string_view Name)
{
    return CrcHashString(Name);
}

//
// Sprites
//

constexpr global_var Sprite PLAYER_SPRITE = AsSprite({ 0, 0, 16, 16 });
constexpr global_var Sprite RAT_SPRITE = AsSprite({ 16, 16, 16, 16 });
constexpr global_var Sprite ROGUE_SPRITE = AsSprite({ 16, 0, 16, 16 });
