#pragma once

#include "Core.h"
#include "Tile.h"
#include "Sprite.h"

#include "Structures/StaticArray.h"

struct Game;
struct Resources;
struct WorldEntity;

#define RICHTEXT_COLOR_ID 0
#define RICHTEXT_IMG_ID 1
#define RICHTEXT_TOOLTIP_ID 2

enum RichTextTypes
{
	RICHTEXT_COLOR = RICHTEXT_COLOR_ID,
	RICHTEXT_IMG = RICHTEXT_IMG_ID,
	RICHTEXT_TOOLTIP = RICHTEXT_TOOLTIP_ID,
};

#define RichTextColorExpand(type, hex) "${" type "," #hex "}"
#define RichTextColor(hex) RichTextColorExpand(Expand(RICHTEXT_COLOR_ID), hex)

#define RichTextImgExpand(type, id, w, h) "${" type "," #id "," #w "," #h "}"
#define RichTextImg(id, w, h) RichTextImgExpand(Expand(RICHTEXT_IMG_ID), id, w, h)

#define RichTextTooltipExpand(type, str)  "${" type "," str "}"
#define RichTextTooltip(str) RichTextTooltipExpand(Expand(RICHTEXT_TOOLTIP_ID), str)

#define RichTextTooltipExExpand(type, str, w, h)  "${" type "," str "," #w "," #h "}"
#define RichTextTooltipEx(str, w, h) RichTextTooltipWHExpand(Expand(RICHTEXT_TOOLTIP_ID), str, w, h)

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

	int UniformTilesLoc;
	int UniformSpriteLoc;
	int UniformMapTilesCountX;
	int UniformMapTilesCountY;

	DynamicArray<TileTexValues> Tiles;

	void Initialize(Game* game);
	void Free();
	void Draw();
};

// NOTE: I originally ran into problem using bitfield or 
// 2 u8's with bit shifts to store 4 bit per channel.
// It seemed the rgba format from color was reverse to abgr?
// I don't know? And I can't find anything related to it, so
// I am either dumb or something weird happens. But because,
// I don't want to deal with a possible memory layout per system
// bug I just will use 2 u8's for now.
struct TileLightData
{
	uint8_t r; // LOS - Keep between 0-15
	uint8_t g; // Cieling - Keep betwen 0-15
};

struct LightingRenderer
{
	Shader LightingShader;

	RenderTexture2D LightMapTexture;

	//RenderTexture2D ColorsTexture;
	//RenderTexture2D LightingTexture;

	RenderTexture2D TileColorsTexture;
	RenderTexture2D TileLightDataTexture;
	
	//DynamicArray<Vector3> Tiles;
	DynamicArray<Color> TileColors;
	DynamicArray<TileLightData> TileData;

	Vector3 AmbientLightColor;
	Vector3 SunlightColor;
	Vector3 LOSColor;
	float LightIntensity;

	int UniformSunlight;
	int UniformLOSColor;
	int UniformTileColorTexture;
	int UniformTileDataTexture;

	void Initialize(Game* game);
	void Free();
	void Draw(Rectangle dstRect);
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

void SDrawSubSprite(const Texture2D* texture, Sprite sprite, Vector2 pos, Vector2 offset, Color color, bool flipX);
void SDrawSprite(const Texture2D* texture, Sprite sprite, Vector2 pos, Color color, bool flipX);
void SDrawSprite(const Texture2D* texture, Rectangle source, Rectangle dest, Color color, bool flipX);

void SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color);

RenderTexture2D
SLoadRenderTexture(int width, int height, PixelFormat format);

RenderTexture2D
SLoadRenderTextureEx(int width, int height, PixelFormat format, bool useDepth);

Font Scal_LoadBMFont(const char* fileName, Texture2D fontTexture, Vector2 offset);

// Draws text that can handle special keywords in the text.
// Keywords and their parameters follow this format. ${0,param1,paaram2}
// Keywords are a character following the ${ . I might switch to using a string
// for them. There characters are found in RichTextTypes enum
void DrawRichText(Font font, const char* text, Vector2 position, float fontSize, float spacing, Color tint);
