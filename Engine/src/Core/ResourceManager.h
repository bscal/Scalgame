#pragma once

#include "Core.h"
#include "SpriteAtlas.h"

struct Resources;

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

struct Renderer
{
	int UniformAmbientLightLoc;
	int UniformLightIntensityLoc;
	int UniformSunLightColorLoc;
	int UniformLightMapLoc;

	Vector4 AmbientLight;
	Vector4 SunLight;
	float LightIntensity;

	void Initialize(Resources* gameApp, Texture2D* lightMapTexture);
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
	Shader LitShader;
	Shader LightingShader;
	Shader BrightnessShader;
	bool IsAllocated;
};

bool InitializeResources(Resources* outData);
void FreeResouces(Resources* resources);
