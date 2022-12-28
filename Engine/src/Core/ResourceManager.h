#pragma once

#include "Engine.h"
#include "Core/TileMap.h"

struct SDFFont
{
	Font Font;
	Shader Shader;
};

struct Resources
{
	Texture2D EntitySpriteSheet;
	Texture2D TileSheet;
	Font MainFontM;
	//Font MainFontS;
	Font FontSilver;
	Shader UnlitShader;
	Shader LitShader;
	Shader LightSamplerShader;
	Shader LightRayShader;
	bool IsInitialized;
};

bool InitializeResources(Resources* outData);
