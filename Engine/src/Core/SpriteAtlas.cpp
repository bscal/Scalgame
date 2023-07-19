#include "SpriteAtlas.h"

#include "SMemory.h"

SpriteAtlas SpriteAtlasLoad(const char* dirPath, const char* atlasFile)
{
	SASSERT(dirPath);
	SASSERT(atlasFile);

	const char* atlasFiles[2] = { dirPath, atlasFile };
	const char* filepath = TextJoin(atlasFiles, 2, 0);

	SpriteAtlas atlas = {};

	if (!FileExists(filepath))
	{
		SLOG_ERR("[ SpriteAtlas ] File does not exist. Path: %s", filepath);
		return atlas;
	}

	char* atlasData = LoadFileText(filepath);
	SASSERT(atlasData);

	int count = 0;
	int bufferSize = Kilobytes(10);
	char* buffer = (char*)SMemTempAlloc(bufferSize);
	int splitBufferSize = Kilobytes(1);
	char** split = (char**)SMemTempAlloc(splitBufferSize * sizeof(char*));
	TextSplitBuffered(atlasData, '\n', &count, buffer, bufferSize, split, splitBufferSize);
	
	// 1st line empty
	int line = 1;
	const char* imgName = split[line++];
	const char* size = split[line++];
	const char* format = split[line++];
	const char* filter = split[line++];
	const char* repear = split[line++];

	const char* imgFiles[2] = { dirPath, imgName };
	const char* imgPath = TextJoin(imgFiles, 2, 0);

	atlas.Length = (count / 7) - 1;
	atlas.Rects = (Rect16*)SAlloc(SAllocator::Game, atlas.Length * sizeof(Rect16), MemoryTag::Game);
	atlas.NameToIdx.Reserve(atlas.Length);
	atlas.TextureName = RawStringNew(imgPath, SAllocator::Game);
	atlas.Texture = LoadTexture(atlas.TextureName.Data);

	char s0[16];
	char s1[16];

	for (uint16_t entryCounter = 0; entryCounter < atlas.Length; ++entryCounter)
	{
		if (line >= count)
			break;
		// Weird hack to make sure we don't process a space at the end if the atlas format happens to do so.
		//if (entryCounter == atlas.Length - 1 && (split[line][0] == ' ' || split[line][0] == '\0'))
			//break;

		const char* name	= split[line];
		const char* rotate	= split[line + 1];
		const char* xy		= split[line + 2];
		const char* size	= split[line + 3];
		const char* orig	= split[line + 4];
		const char* offset	= split[line + 5];
		const char* index	= split[line + 6];

		line += 7;

		SRawString nameStr = RawStringNew(name, SAllocator::Game);
		atlas.NameToIdx.Insert(&nameStr, &entryCounter);

		int err;

		int x;
		int y;
		{
			const char* find = xy + 5;
			int comma = TextFindIndex(find, ",");

			SMemClear(s0, 16);
			SMemClear(s1, 16);
			SMemCopy(s0, find, comma);
			SMemCopy(s1, find + comma + 1, strlen(find + comma + 1));

			
			RemoveWhitespace(s0);
			err = Str2Int(&x, s0, 10);
			SASSERT(err == STR2INT_SUCCESS);
			RemoveWhitespace(s1);
			err = Str2Int(&y, s1, 10);
			SASSERT(err == STR2INT_SUCCESS);

		}

		int w;
		int h;
		{
			const char* find = size + 7;
			int comma = TextFindIndex(find, ",");

			SMemClear(s0, 16);
			SMemClear(s1, 16);
			SMemCopy(s0, find, comma);
			SMemCopy(s1, find + comma + 1, strlen(find + comma + 1));
			
			RemoveWhitespace(s0);
			err = Str2Int(&w, s0, 10);
			SASSERT(err == STR2INT_SUCCESS);
			RemoveWhitespace(s1);
			err = Str2Int(&h, s1, 10);
			SASSERT(err == STR2INT_SUCCESS);
		}

		Rect16 r;
		r.x = (uint16_t)x;
		r.y = (uint16_t)y;
		r.w = (uint16_t)w;
		r.h = (uint16_t)h;

		atlas.Rects[entryCounter] = r;

		SLOG_DEBUG("[ SpriteAtlas ] Loaded rect, %s, x:%u, y:%u, w:%u, h:%u", name, x, y, w, h);
	}

	UnloadFileText(atlasData);

	SLOG_INFO("[ SpriteAtlas ] Successfully loaded sprite atlas, %s, with %i sprites", filepath, atlas.Length);

	return atlas;
}


void SpriteAtlasUnload(SpriteAtlas* atlas)
{
	SFree(SAllocator::Game, atlas->Rects, atlas->Length * sizeof(Rect16), MemoryTag::Game);
	atlas->NameToIdx.Free();
	UnloadTexture(atlas->Texture);
}

Rectangle SpriteAtlasRect(SpriteAtlas* atlas, const SRawString name)
{
	uint16_t* idxPtr = atlas->NameToIdx.Get(&name);
	if (!idxPtr)
	{
		SLOG_WARN("[ SpriteAtlas ] Cannot find sprite %s", name.Data);
		return {};
	}

	uint16_t idx = *idxPtr;

	Rectangle res;
	res.x = atlas->Rects[idx].x;
	res.y = atlas->Rects[idx].y;
	res.width = atlas->Rects[idx].w;
	res.height = atlas->Rects[idx].h;
	return res;
}
