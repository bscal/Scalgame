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
	SpriteAtlas Atlas;
	Font MainFontM;
	Font MainFontS;
	Font FontSilver;
	Shader UnlitShader;
	bool IsInitialized;
};

bool InitializeResources(Resources* outData);
