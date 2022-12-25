#include "Sprite.h"

void AnimatedSprite::Update()
{
    ++CurrentAnimation.FrameProgress;
    if (CurrentAnimation.FrameProgress == CurrentAnimation.FrameSpeed)
    {
        CurrentAnimation.FrameProgress = 0;
        ++CurrentAnimation.FrameIndex;
        if (CurrentAnimation.FrameIndex >= CurrentAnimation.FrameCount)
        {
            if (Loop)
                CurrentAnimation.FrameIndex = 0;
            else
                CurrentAnimation.FrameIndex = CurrentAnimation.FrameCount;
        }
    }
}
