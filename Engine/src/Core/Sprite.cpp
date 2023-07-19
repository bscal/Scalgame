#include "Sprite.h"

#include "Game.h"
#include "SMemory.h"
#include "SString.h"

#include <rectpack2D/src/finders_interface.h>

using namespace rectpack2D;
/*
void SpriteAtlasLoad(SpriteAtlas* atlas, const char* spriteDirPath)
{
	SASSERT(atlas);
	SASSERT(spriteDirPath);

	constexpr bool allow_flip = false;

	using spaces_type = rectpack2D::empty_spaces<allow_flip, default_empty_spaces>;
	using rect_type = output_rect_t<spaces_type>;

	finder_input input = make_finder_input(
		1000,
		-4,
		[](rect_type&)
		{
			return callback_result::CONTINUE_PACKING;
		},
		[](rect_type&)
		{
			SFATAL("Cannot Pack!");
			return callback_result::ABORT_PACKING;
		},
		flipping_option::DISABLED
	);

	if (!DirectoryExists(spriteDirPath))
	{
		SFATAL("[ SpriteAtlas ] spriteDirPath does not exist: %s", spriteDirPath);
	}

	FilePathList dirFiles = LoadDirectoryFiles(spriteDirPath);

	SList<Texture2D> textures = {};
	textures.Allocator = SAllocator::Temp;
	textures.Reserve(dirFiles.count);

	std::vector<rect_xywh> rects = {};
	rects.reserve(dirFiles.count);

	for (uint32_t i = 0; i < dirFiles.count; ++i)
	{
		char* path = dirFiles.paths[i];

		const char* extensionMatch = ".png";
		const char* extension = GetFileExtension(path);
		if (strcmp(extensionMatch, extension))
		{
			Texture2D texture = LoadTexture(path);
			if (IsTextureReady(texture))
			{
				textures.Push(&texture);
				rects.emplace_back(rect_xywh{ 0, 0, texture.width, texture.height });

				const char* filename = GetFileNameWithoutExt(path);
				SRawString str = RawStringNew(filename, SAllocator::Game);
				uint16_t id = atlas->NextIdx++;
				atlas->NameToSpriteIdx.Insert(&str, &id);
			}
		}
	}

	rect_wh result_size = find_best_packing<spaces_type>(rects, input);

	atlas->RenderTexture = LoadRenderTexture(result_size.w, result_size.h);

	atlas->Length = textures.Count;
	size_t spritesSize = sizeof(Sprite) * atlas->Length;
	atlas->Sprites = (Sprite*)SAlloc(SAllocator::Game, spritesSize, MemoryTag::Arrays);

	BeginTextureMode(atlas->RenderTexture);

	for (uint32_t i = 0; i < textures.Count; ++i)
	{
		Texture2D text = textures[i];
		rect_xywh rect = rects[i];
		Sprite sprite = { (uint16_t)rect.x, (uint16_t)rect.y, (uint16_t)rect.w, (uint16_t)rect.h };
		
		SMemCopy(atlas->Sprites + i, &sprite, sizeof(Sprite));

		Rectangle src = { (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h };
		DrawTextureRec(text, src, { src.x, src.y }, WHITE);
	}

	EndTextureMode();

	for (uint32_t i = 0; i < textures.Count; ++i)
	{
		UnloadTexture(textures[i]);
	}

	UnloadDirectoryFiles(dirFiles);

	SLOG_INFO("[ SpriteAtlas ] Created atlas size w: %d, h: %d", result_size.w, result_size.h);
	SLOG_INFO("[ SpriteAtlas ] Succesfully Initialized!");
}

void SpriteAtlasFree(SpriteAtlas* atlas)
{
	UnloadRenderTexture(atlas->RenderTexture);
	SFree(SAllocator::Game, atlas->Sprites, atlas->Length * sizeof(Sprite), MemoryTag::Game);
	atlas->NameToSpriteIdx.Free();
}

void SpritesUpdate()
{
	for (uint16_t i = 0; i < SpriteMgr.NextId; ++i)
	{
		TileSprite& sprite = SpriteMgr.TileSprites[i];
		sprite.Time += 1;
		if (sprite.Time > sprite.Anim.TickSpeed)
		{
			sprite.Time = 0;
			sprite.Anim.CurrentIdx = (sprite.Anim.CurrentIdx + 1) % sprite.Anim.Frames.Count;
		}
	}
}

Sprite GetTileSprite(uint16_t spriteId)
{
	TileSprite& tileSprite = SpriteMgr.TileSprites[spriteId];
	uint16_t curSpriteId = tileSprite.Anim.Frames[tileSprite.Anim.CurrentIdx];
	return SpriteMgr.Sprites[curSpriteId];
}

uint16_t GetAnimationsByName(SRawString name)
{
	uint16_t* res = SpriteMgr.SpriteNameToIndex.Get(&name);
	if (res)
		return *res;
	else
		return UINT16_MAX;
}

uint16_t RegisterTileSprite(const TileSprite* tileSprite)
{
	SASSERT(tileSprite);
	uint16_t id = SpriteMgr.NextId++;
	void* dst = &SpriteMgr.TileSprites[id];
	memcpy(dst, tileSprite, sizeof(TileSprite));
	return id;
}

void Animator::Update(Game* game)
{
	if (CurrentAnimation)
	{
		if (CurrentAnimation->Frames.Count == 1)
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

				CurrentAnimation->CurrentIdx = (CurrentAnimation->CurrentIdx + 1) % CurrentAnimation->Frames.Count;
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

void Animator::SetAnimation(uint16_t animId)
{
	Animation* animation = Animations.Get(&animId);
	SASSERT(animation);

	CurrentAnimation = Animations.Get(&animId);
	CurrentCycleTick = 0;
}

Sprite Animator::GetAnimation() const
{
	SASSERT(CurrentAnimation);
	uint16_t curSpriteId = CurrentAnimation->Frames[CurrentAnimation->CurrentIdx];
	return SpriteMgr.Sprites[curSpriteId];
}


Animation CreateAnimation(uint16_t tickSpeed, uint16_t framesCount, uint16_t frames...)
{
	SASSERT(framesCount > 0);

	Animation animation = {};
	animation.Frames.FromVarArgs(framesCount, frames);
	animation.TickSpeed = tickSpeed;

	return animation;
}
*/