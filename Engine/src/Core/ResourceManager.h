#pragma once

#include "Core.h"
#include "SpriteAtlas.h"

struct SDFFont
{
	Font Font;
	Shader Shader;
};

struct BlurShader
{
	Shader BlurShader;
	RenderTexture2D TextureHorizontal;
	RenderTexture2D TextureVert;
	int UniformIsHorizontalLocation;

	void Initialize(int width, int height);
	void Draw(const Texture2D& worldTexture) const;
	void Free();
};

struct Resources
{
	Texture2D EntitySpriteSheet;
	Texture2D TileSheet;
	BlurShader Blur;
	SpriteAtlas Atlas;
	Font MainFontM;
	Font MainFontS;
	Font FontSilver;
	Shader UnlitShader;
	Shader BrightnessShader;
	bool IsAllocated;
};

bool InitializeResources(Resources* outData);
void FreeResouces(Resources* resources);
