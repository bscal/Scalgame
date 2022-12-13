#pragma once

#include "Core.h"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

struct SubSprite
{
	float Speed;
    float TotalTime;
    bool32 ShouldLoop;
	std::vector<Rectangle> Frames;
};

struct Sprite
{
    std::unordered_map<uint32_t, SubSprite*> SubSprites;
	uint32_t CurrentSubSprite;
	uint32_t CurrentFrame;
	float FrameProgress;

    void SetAnimation(uint32_t anim)
    {
        CurrentSubSprite = anim;
        CurrentFrame = 0;
        FrameProgress = 0.0f;
    }

    const Rectangle& GetCurrentFrame() const
    {
        return SubSprites.at(CurrentSubSprite)->Frames[CurrentFrame];
    }

    void Update(float dt)
    {
        const auto frame = SubSprites.at(CurrentSubSprite);
        FrameProgress += frame->Speed * dt;
        if (FrameProgress > frame->TotalTime)
        {
            FrameProgress = 0.0f;
            ++CurrentFrame;
            if (CurrentFrame == frame->Frames.size())
            {
                CurrentFrame = 0;
                if (frame->ShouldLoop == 0) CurrentSubSprite = 0;
            }
        }
    }
};
