#pragma once

#include "Core.h"

struct Game;
struct Resources;

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
    BlurShader BlurShader;
    RenderTexture2D WorldTexture;
    RenderTexture2D EffectTextureOne;
    RenderTexture2D EffectTextureTwo;

	Shader UnlitShader;
	Shader LitShader;
	Shader LightingShader;
	Shader BrightnessShader;
	Shader BloomShader;
        
    int UniformAmbientLightLoc;
	int UniformLightIntensityLoc;
	int UniformSunLightColorLoc;
	int UniformLightMapLoc;
	int UniformLightTexLoc;
	int UniformBloomIntensityLoc;

	Vector4 AmbientLight;
	Vector4 SunLight;
	float LightIntensity;
	float BloomIntensity;

	void Initialize();
	void Free();

	void PostProcess(Game* game, const RenderTexture2D& worldTexture,
		const RenderTexture2D& lightingTexture) const;

	void SetValueAndUniformAmbientLight(Vector4 ambientLight);
	void SetValueAndUniformSunLight(Vector4 sunLight);
	void SetValueAndUniformLightIntensity(float intensity);
	void SetValueAndUniformBloomIntensity(float intensity);
};

void 
DrawTextureProF(Texture2D texture, Rectangle source, Rectangle dest,
	Vector2 origin, float rotation, Vector4 tint);

void 
ScalDrawTextureProF(const Texture2D* texture, Rectangle source,
	Rectangle dest, Vector4 tint);

void 
SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color);

RenderTexture2D
SLoadRenderTexture(int width, int height, PixelFormat format);

RenderTexture2D
SLoadRenderTextureEx(int width, int height, PixelFormat format, bool useDepth);
