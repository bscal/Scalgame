#pragma once

#include "Core.h"
#include "SpriteAtlas.h"

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
	SpriteAtlas UIAtlas;
	Texture2D EntitySpriteSheet;
	Texture2D TileSheet;
	Texture2D TileSprite;
	Font MainFontM;
	Font MainFontS;
	Font FontSilver;
};

bool InitializeResources(Resources* outData);
void FreeResouces(Resources* resources);

uint32_t LoadComputeShader(const char* file);
