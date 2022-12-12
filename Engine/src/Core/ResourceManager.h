#pragma once

#include "Engine.h"
#include "Core/TileMap.h"

struct SDFFont
{
	Font Font;
	Shader Shader;
};

// TODO better solution for this
#define PLAYER_SPRITE { 0.0f, 0.0f, 16.0f, 16.0f }

struct Resources
{
	Texture2D EntitySpriteSheet;
	Texture2D TileSheet;
	Font MainFontM;
	//Font MainFontS;
	Font FontSilver;
	Shader TileShader;
	Shader LightRayShader;
	Shader LightShader;
	bool IsInitialized;
};

bool InitializeResources(Resources* outData);
