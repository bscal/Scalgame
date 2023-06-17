#pragma once

#include "Core.h"
#include "Tile.h"
#include "Sprite.h"

#include "Structures/StaticArray.h"

struct Game;
struct Resources;
struct WorldEntity;

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

struct TileMapRenderer
{
	Shader TileMapShader;
	RenderTexture2D TileMapTexture;
	RenderTexture2D TileDataTexture;

	int UniformViewOffsetLoc;
	int UniformViewPortSizeLoc;
	int UniformInverseTileTextureSizeLoc;
	int UniformInverseTileSizeLoc;
	int UniformTilesLoc;
	int UniformSpriteLoc;
	int UniformInverseSpriteTextureSizeLoc;
	int UniformTileSizeLoc;

	StaticArray<TileTexValues, CULL_TOTAL_TILES> Tiles;

	void Initialize(Game* game);
	void Free();
	void Draw();
};

struct LightingRenderer
{
	Shader LightingShader;

	RenderTexture2D ColorsTexture;
	RenderTexture2D LightingTexture;
	
	StaticArray<Vector4, CULL_TOTAL_TILES> Tiles;

	Vector3 AmbientLightColor;
	Vector3 SunlightColor;
	Vector3 LOSColor;
	float LightIntensity;

	int UniformSunlight;
	int UniformLOSColor;
	int UniformWorldMap;

	void Initialize(Game* game);
	void Free();
	void Draw();
};

struct Renderer
{
    BlurShader BlurShader;
    RenderTexture2D WorldTexture;
    RenderTexture2D EffectTextureOne;
    RenderTexture2D EffectTextureTwo;

	Shader UnlitShader;
	Shader LitShader;
	Shader BrightnessShader;
	Shader BloomShader;

    int UniformAmbientLightLoc;
	int UniformLightIntensityLoc;
	int UniformSunLightColorLoc;
	int UniformLightMapLoc;
	int UniformLightTexLoc;
	int UniformBloomIntensityLoc;

	void Initialize();
	void Free();

	void PostProcess(Game* game, const RenderTexture2D& worldTexture,
		const RenderTexture2D& lightingTexture) const;

	void DrawBloom(Rectangle dest);
};

void
SDrawTextureProF(const Texture2D& texture, Rectangle source, const Rectangle& dest,
	Vector2 origin, float rotation, const Vector4& tint);

void 
SDrawTextureF(const Texture2D& texture, const Rectangle& source,
	const Rectangle& dest, const Vector4& tint);

void
SDrawSprite(Texture2D* texture, Rectangle source, Rectangle dest, Color color, bool flipX);

void SDrawSprite(Texture2D* texture, WorldEntity* entity, Vector2 pos, Sprite sprite);

void 
SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color);

RenderTexture2D
SLoadRenderTexture(int width, int height, PixelFormat format);

RenderTexture2D
SLoadRenderTextureEx(int width, int height, PixelFormat format, bool useDepth);

void DrawTileMap(Texture2D texture, Rectangle dest);
