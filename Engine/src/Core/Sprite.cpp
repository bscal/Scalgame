#include "Sprite.h"

#include "Game.h"
#include "SMemory.h"

#include <stdarg.h>

void Animator::Update(Game* game)
{
	if (CurrentAnimation)
	{
		if (CurrentAnimation->FramesCount == 1)
		{
			CurrentAnimation->CurrentIdx = 1;
			return;
		}
		else
		{
			++CurrentCycleTick;
			if (CurrentCycleTick == CurrentAnimation->TickSpeed)
			{
				CurrentCycleTick = 0;
				++CurrentAnimation->CurrentIdx;

				SASSERT(CurrentAnimation->FramesCount > 0);
				if (CurrentAnimation->CurrentIdx >= CurrentAnimation->FramesCount - 1)
					CurrentAnimation->CurrentIdx = 0;
			}
		}
	}
	else if (IdleAnimation)
	{
		CurrentAnimation = IdleAnimation;
	}
	else
	{
		SASSERT_MSG(false, "CurrentAnimation and IdleAnimation are both NULL!");
	}
}

void Animator::SetAnimation(AnimationId animId)
{
	Animation* animation = Animations.Get(&animId);
	SASSERT(animation);

	CurrentAnimation = animation;
	CurrentCycleTick = 0;
}

Sprite Animator::GetAnimation() const
{
	SASSERT(CurrentAnimation);
	return CurrentAnimation->Frames[CurrentAnimation->CurrentIdx];
}


Animation CreateAnimation(uint16_t framesCount, uint16_t tickSpeed, Sprite frames...)
{
	SASSERT(framesCount > 0);

	Animation animation;

	size_t size = sizeof(Sprite) * framesCount;
	animation.FramesCount = framesCount;
	animation.TickSpeed = tickSpeed;
	animation.Frames = (Sprite*)SAlloc(SAllocator::Game, size, MemoryTag::Arrays);

	va_list ap;
	va_start(ap, frames);

	for (uint16_t i = 0; i < framesCount; ++i)
	{
		animation.Frames[i] = va_arg(ap, Sprite);
	}
	
	va_end(ap);

	return animation;
}
