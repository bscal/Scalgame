#pragma once

#include "Core.h"

#define ASSET_PATH "assets/"
#define SHADERS_PATH ASSET_PATH "shaders/"
#define TEXTURES_PATH ASSET_PATH "textures/"

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
	//SpriteAtlas Atlas;
	Font MainFontM;
	Font MainFontS;
	Font FontSilver;
};

bool InitializeResources(Resources* outData);
void FreeResouces(Resources* resources);
