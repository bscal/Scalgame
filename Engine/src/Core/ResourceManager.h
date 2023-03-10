#pragma once

#include "Core.h"
#include "SpriteAtlas.h"

struct SDFFont
{
	Font Font;
	Shader Shader;
};

struct Resources
{
	Texture2D EntitySpriteSheet;
	Texture2D TileSheet;
	Texture2D TileSprite;
	SpriteAtlas Atlas;
	Font MainFontM;
	Font MainFontS;
	Font FontSilver;
};

bool InitializeResources(Resources* outData);
void FreeResouces(Resources* resources);
