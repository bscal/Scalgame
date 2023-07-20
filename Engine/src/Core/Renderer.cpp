#include "Renderer.h"

#include "Game.h"
#include "Entity.h"
#include "ResourceManager.h"

#include <string.h>

#include "raylib/src/rlgl.h"
#include "raylib/src/raymath.h"

void Renderer::Initialize()
{
	int w = GetGameApp()->View.Resolution.x;
	int h = GetGameApp()->View.Resolution.y;

	int blurWidth = w / 4;
	int blurHeight = h / 4;
	BlurShader.Initialize(blurWidth, blurHeight);

	WorldTexture = SLoadRenderTexture(w, h, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	EffectTextureOne = SLoadRenderTexture(w, h, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	EffectTextureTwo = SLoadRenderTexture(w, h, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);

	UnlitShader = LoadShader(
		"assets/shaders/tile_shader.vert",
		"assets/shaders/tile_shader.frag");

	LitShader = LoadShader(
		"assets/shaders/tile_lit.vert",
		"assets/shaders/tile_lit.frag");

	BrightnessShader = LoadShader(
		"assets/shaders/tile_shader.vert",
		"assets/shaders/brightness_filter.frag");

	BloomShader = LoadShader(
		"assets/shaders/bloom.vert",
		"assets/shaders/bloom.frag");

	UniformLightMapLoc = GetShaderLocation(LitShader, "texture1");
	UniformLightTexLoc = GetShaderLocation(BloomShader, "texture1");
	UniformBloomIntensityLoc = GetShaderLocation(BloomShader, "bloomIntensity");

	SLOG_INFO("[ Renderer ] Initialized!");
}

void Renderer::Free()
{
	BlurShader.Free();
	UnloadRenderTexture(WorldTexture);
	UnloadRenderTexture(EffectTextureOne);
	UnloadRenderTexture(EffectTextureTwo);
	UnloadShader(UnlitShader);
	UnloadShader(LitShader);
	UnloadShader(BrightnessShader);
	UnloadShader(BloomShader);
}

void Renderer::PostProcess(Game* game, const RenderTexture2D& worldTexture,
	const RenderTexture2D& lightingTexture) const
{
	float screenW = (float)GetScreenWidth();
	float screenH = (float)GetScreenHeight();
	Rectangle srcRect = { 0.0f, 0.0f, (float)lightingTexture.texture.width, -(float)lightingTexture.texture.height};
	Rectangle screenRect = { 0.0f, 0.0f, screenW, screenH };

	// Brightness pass
	BeginTextureMode(EffectTextureTwo);

	BeginShaderMode(BrightnessShader);
	ClearBackground(BLACK);
	DrawTexturePro(lightingTexture.texture, srcRect, screenRect, { 0 }, 0.f, WHITE);
	EndShaderMode();
	
	EndTextureMode();

	// Blur pass
	BlurShader.Draw(EffectTextureTwo.texture);
}

void Renderer::DrawBloom(Rectangle dest)
{
	BeginBlendMode(BLEND_ADDITIVE);
	Rectangle rect;
	rect.x = 0;
	rect.y = 0;
	rect.width = (float)BlurShader.TextureVert.texture.width;
	rect.height = -(float)BlurShader.TextureVert.texture.height;
	DrawTexturePro(BlurShader.TextureVert.texture, rect, dest, {}, 0.0f, WHITE);
	EndBlendMode();
}

void BlurShader::Initialize(int width, int height)
{
	BlurShader = LoadShader(
		"assets/shaders/blur.vert",
		"assets/shaders/blur.frag");

	TextureHorizontal = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	TextureVert = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);

	UniformIsHorizontalLocation = GetShaderLocation(BlurShader, "isHorizontal");

	int targetWidthLoc = GetShaderLocation(BlurShader, "targetWidth");
	float targetWidth = (float)width;
	SetShaderValue(BlurShader, targetWidthLoc, &targetWidth, SHADER_UNIFORM_FLOAT);
}

void BlurShader::Free()
{
	UnloadRenderTexture(TextureHorizontal);
	UnloadRenderTexture(TextureVert);
	UnloadShader(BlurShader);
}

void BlurShader::Draw(const Texture2D& lightingTexture) const
{
	Rectangle srcRect =
	{
		0.0f,
		0.0f,
		(float)lightingTexture.width,
		-(float)lightingTexture.height,
	};

	Rectangle blurRectSrc =
	{
		0.0f,
		0.0f,
		(float)TextureHorizontal.texture.width,
		-(float)TextureHorizontal.texture.height,
	};

	Rectangle blurRectDest =
	{
		0.0f,
		0.0f,
		(float)TextureHorizontal.texture.width,
		(float)TextureHorizontal.texture.height,
	};

	Vector4 colorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };

	BeginShaderMode(BlurShader);

	int isHorizontal = 1;
	SetShaderValue(BlurShader, UniformIsHorizontalLocation, &isHorizontal, SHADER_UNIFORM_INT);

	BeginTextureMode(TextureHorizontal);
	ClearBackground({});
	SDrawTextureProF(lightingTexture, srcRect, blurRectDest, { 0 }, 0.0f, colorWhite);
	EndTextureMode();

	isHorizontal = 0;
	SetShaderValue(BlurShader, UniformIsHorizontalLocation, &isHorizontal, SHADER_UNIFORM_INT);

	BeginTextureMode(TextureVert);
	ClearBackground({});
	SDrawTextureProF(TextureHorizontal.texture, blurRectSrc, blurRectDest, { 0 }, 0.0f, colorWhite);
	EndTextureMode();

	EndShaderMode();
}

void TileMapRenderer::Initialize(Game* game)
{
	int w = GetGameApp()->View.ResolutionInTiles.x;
	int h = GetGameApp()->View.ResolutionInTiles.y;

	Tiles.Initialize(w * h, SAllocator::Game);

	TileMapShader = LoadShader(
		"assets/shaders/tilemap.vert",
		"assets/shaders/tilemap.frag");

	UniformTilesLoc = GetShaderLocation(TileMapShader, "mapData");
	UniformSpriteLoc = GetShaderLocation(TileMapShader, "textureAtlas");
	UniformMapTilesCountX = GetShaderLocation(TileMapShader, "mapTilesCountX");
	UniformMapTilesCountY = GetShaderLocation(TileMapShader, "mapTilesCountY");

	float mapTileCountX = (float)w;
	float mapTileCountY = (float)h;
	SetShaderValue(TileMapShader, UniformMapTilesCountX, &mapTileCountX, RL_SHADER_UNIFORM_FLOAT);
	SetShaderValue(TileMapShader, UniformMapTilesCountY, &mapTileCountY, RL_SHADER_UNIFORM_FLOAT);

	TileMapTexture = SLoadRenderTexture((float)GetGameApp()->View.Resolution.x, (float)GetGameApp()->View.Resolution.y, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	TileDataTexture = SLoadRenderTexture(w, h, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
}

void TileMapRenderer::Free()
{
	UnloadShader(TileMapShader);
	UnloadRenderTexture(TileMapTexture);
	UnloadRenderTexture(TileDataTexture);
}

void TileMapRenderer::Draw()
{
	UpdateTexture(TileDataTexture.texture, Tiles.Memory);

	SMemClear(Tiles.Memory, Tiles.SizeOf());

	BeginTextureMode(TileMapTexture);
	BeginShaderMode(TileMapShader);

	SetShaderValueTexture(TileMapShader, UniformSpriteLoc, GetGame()->Resources.TileSheet);
	SetShaderValueTexture(TileMapShader, UniformTilesLoc, TileDataTexture.texture);
	const Texture2D& tileSprite = GetGame()->Resources.TileSprite;
	Rectangle src = { 0, 0, (float)tileSprite.width, (float)tileSprite.height };
	Rectangle dst = { 0, 0, (float)GetGameApp()->View.Resolution.x,  (float)GetGameApp()->View.Resolution.y};
	DrawTexturePro(tileSprite, src, dst, { 0 }, 0.0f, WHITE);

	EndShaderMode();
	EndTextureMode();
}

void LightingRenderer::Initialize(Game* game)
{
	int w = (int)GetGameApp()->View.ResolutionInTiles.x;
	int h = (int)GetGameApp()->View.ResolutionInTiles.y;

	TileColors.Initialize(w * h, SAllocator::Game);
	TileData.Initialize(w * h, SAllocator::Game);

	LightingShader = LoadShader(
		SHADERS_PATH "lighting_v2.vert",
		SHADERS_PATH "lighting_v2.frag");

	UniformSunlight = GetShaderLocation(LightingShader, "sunlightColor");
	UniformLOSColor = GetShaderLocation(LightingShader, "losColor");
	UniformTileDataTexture = GetShaderLocation(LightingShader, "tileDataTexture");

	AmbientLightColor = { 25.0f / 255.0f, 20.0f / 255.0f, 45.0f / 255.0f };
	SunlightColor = { 0.0f, 0.0f, 0.0f };
	LOSColor = { 0.0f, 0.0f, 0.0f };
	LightIntensity = 1.0f;

	Vector2i lightMapReso = GetGameApp()->View.Resolution;
	LightMapTexture = SLoadRenderTexture(lightMapReso.x, lightMapReso.y, PIXELFORMAT_UNCOMPRESSED_R32G32B32);

	Vector2i tileReso = GetGameApp()->View.ResolutionInTiles;
	TileColorsTexture = SLoadRenderTexture(tileReso.x, tileReso.y, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	TileLightDataTexture = SLoadRenderTexture(tileReso.x, tileReso.y, PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA);
}

void LightingRenderer::Free()
{
	UnloadShader(LightingShader);
	UnloadRenderTexture(LightMapTexture);
	UnloadRenderTexture(TileColorsTexture);
	UnloadRenderTexture(TileLightDataTexture);
}

void LightingRenderer::Draw(Rectangle dstRect)
{
	Rectangle src;
	src.x = 0;
	src.y = 0;
	src.width = (float)TileColorsTexture.texture.width;
	src.height = (float)TileColorsTexture.texture.height;

	SASSERT(sizeof(TileColors[0]) == 4);
	SASSERT(sizeof(TileData[0]) == 2);

	UpdateTexture(TileColorsTexture.texture, TileColors.Memory);
	SMemClear(TileColors.Memory, TileColors.SizeOf());

	UpdateTexture(TileLightDataTexture.texture, TileData.Memory);
	SMemClear(TileData.Memory, TileData.SizeOf());

	BeginTextureMode(LightMapTexture);
	ClearBackground(BLACK);

	BeginShaderMode(LightingShader);

	Vector4 color;
	color.x = AmbientLightColor.x;
	color.y = AmbientLightColor.y;
	color.z = AmbientLightColor.z;
	color.w = LightIntensity;

	BeginMode2D(GetGame()->WorldCamera);

	SetShaderValue(LightingShader, UniformSunlight, &SunlightColor, SHADER_UNIFORM_VEC3);
	SetShaderValue(LightingShader, UniformLOSColor, &LOSColor, SHADER_UNIFORM_VEC3);
	SetShaderValueTexture(LightingShader, UniformTileDataTexture, TileLightDataTexture.texture);

	// Draw as texture for tex coordinates
	SDrawTextureProF(TileColorsTexture.texture, src, dstRect, { 0 }, 0.0f, color);

	EndMode2D();

	EndShaderMode();

	EndTextureMode();
}

void
SDrawTextureProF(const Texture2D& texture, Rectangle source, const Rectangle& dest,
	Vector2 origin, float rotation, const Vector4& tint)
{
	SASSERT(texture.id > 0);
	// Check if texture is valid
	if (texture.id > 0)
	{
		float width = (float)texture.width;
		float height = (float)texture.height;

		bool flipX = false;

		if (source.width < 0) { flipX = true; source.width *= -1; }
		if (source.height < 0) source.y -= source.height;

		Vector2 topLeft;
		Vector2 topRight;
		Vector2 bottomLeft;
		Vector2 bottomRight;

		// Only calculate rotation if needed
		if (rotation == 0.0f)
		{
			float x = dest.x - origin.x;
			float y = dest.y - origin.y;
			topLeft = Vector2{ x, y };
			topRight = Vector2{ x + dest.width, y };
			bottomLeft = Vector2{ x, y + dest.height };
			bottomRight = Vector2{ x + dest.width, y + dest.height };
		}
		else
		{
			float sinRotation = sinf(rotation * DEG2RAD);
			float cosRotation = cosf(rotation * DEG2RAD);
			float x = dest.x;
			float y = dest.y;
			float dx = -origin.x;
			float dy = -origin.y;

			topLeft.x = x + dx * cosRotation - dy * sinRotation;
			topLeft.y = y + dx * sinRotation + dy * cosRotation;

			topRight.x = x + (dx + dest.width) * cosRotation - dy * sinRotation;
			topRight.y = y + (dx + dest.width) * sinRotation + dy * cosRotation;

			bottomLeft.x = x + dx * cosRotation - (dy + dest.height) * sinRotation;
			bottomLeft.y = y + dx * sinRotation + (dy + dest.height) * cosRotation;

			bottomRight.x = x + (dx + dest.width) * cosRotation - (dy + dest.height) * sinRotation;
			bottomRight.y = y + (dx + dest.width) * sinRotation + (dy + dest.height) * cosRotation;
		}

		rlSetTexture(texture.id);
		rlBegin(RL_QUADS);
			rlColor4f(tint.x, tint.y, tint.z, tint.w);
			rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

			// Top-left corner for texture and quad
			if (flipX) rlTexCoord2f((source.x + source.width) / width, source.y / height);
			else rlTexCoord2f(source.x / width, source.y / height);
			rlVertex2f(topLeft.x, topLeft.y);

			// Bottom-left corner for texture and quad
			if (flipX) rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
			else rlTexCoord2f(source.x / width, (source.y + source.height) / height);
			rlVertex2f(bottomLeft.x, bottomLeft.y);

			// Bottom-right corner for texture and quad
			if (flipX) rlTexCoord2f(source.x / width, (source.y + source.height) / height);
			else rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
			rlVertex2f(bottomRight.x, bottomRight.y);

			// Top-right corner for texture and quad
			if (flipX) rlTexCoord2f(source.x / width, source.y / height);
			else rlTexCoord2f((source.x + source.width) / width, source.y / height);
			rlVertex2f(topRight.x, topRight.y);
		rlEnd();
		rlSetTexture(0);
	}
}


void
SDrawTextureF(const Texture2D& texture, const Rectangle& source,
	const Rectangle& dest, const Vector4& tint)
{
	SASSERT(texture.id > 0);

	float width = (float)texture.width;
	float height = (float)texture.height;

	Vector2 topLeft = Vector2{ dest.x, dest.y };
	Vector2 topRight = Vector2{ dest.x + dest.width, dest.y };
	Vector2 bottomLeft = Vector2{ dest.x, dest.y + dest.height };
	Vector2 bottomRight = Vector2{ dest.x + dest.width, dest.y + dest.height };

	rlSetTexture(texture.id);
	rlBegin(RL_QUADS);
		rlColor4f(tint.x, tint.y, tint.z, tint.w);
		rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

		rlTexCoord2f(source.x / width, source.y / height);
		rlVertex2f(topLeft.x, topLeft.y);

		rlTexCoord2f(source.x / width, (source.y + source.height) / height);
		rlVertex2f(bottomLeft.x, bottomLeft.y);

		rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
		rlVertex2f(bottomRight.x, bottomRight.y);

		rlTexCoord2f((source.x + source.width) / width, source.y / height);
		rlVertex2f(topRight.x, topRight.y);
	rlEnd();
	rlSetTexture(0);
}

void SDrawSprite(Texture2D* texture, WorldEntity* entity, Vector2 pos, Sprite sprite)
{
	SASSERT(texture);
	SASSERT(entity);
	Rectangle src = { (float)sprite.x, (float)sprite.y, (float)sprite.w, (float)sprite.h };
	Rectangle dst = { pos.x, pos.y, (float)sprite.w, (float)sprite.h };
	bool flip = (entity->LookDir == TileDirection::South || entity->LookDir == TileDirection::West) ? true : false;
	SDrawSprite(texture, src, dst, entity->Color, flip);
}

void SDrawSubSprite(Texture2D* texture, WorldEntity* entity, Vector2 pos, Vector2 offset, Sprite sprite)
{
	SASSERT(texture);
	SASSERT(entity);
	Rectangle src = { (float)sprite.x, (float)sprite.y, (float)sprite.w, (float)sprite.h };
	Rectangle dst = { pos.x, pos.y + offset.y, (float)sprite.w, (float)sprite.h };
	bool flip = (entity->LookDir == TileDirection::South || entity->LookDir == TileDirection::West) ? true : false;
	dst.x += (flip) ? -offset.x + -sprite.w : offset.x;
	SDrawSprite(texture, src, dst, entity->Color, flip);
}

void SDrawSprite(Texture2D* texture, Rectangle source, Rectangle dest, Color color, bool flipX)
{
	SASSERT(texture);
	SASSERT(texture->id);

	float rotation = 0.0f;
	float width = (float)texture->width;
	float height = (float)texture->height;

	if (source.height < 0) source.y -= source.height;

	Vector2 topLeft = { 0 };
	Vector2 topRight = { 0 };
	Vector2 bottomLeft = { 0 };
	Vector2 bottomRight = { 0 };

	// Only calculate rotation if needed
	if (rotation == 0.0f)
	{
		float x = dest.x - 0.0f;
		float y = dest.y - 0.0f;
		topLeft = Vector2{ x, y };
		topRight = Vector2{ x + dest.width, y };
		bottomLeft = Vector2{ x, y + dest.height };
		bottomRight = Vector2{ x + dest.width, y + dest.height };
	}
	else
	{
		float sinRotation = sinf(rotation * DEG2RAD);
		float cosRotation = cosf(rotation * DEG2RAD);
		float x = dest.x;
		float y = dest.y;
		float dx = -0.0f;
		float dy = -0.0f;

		topLeft.x = x + dx * cosRotation - dy * sinRotation;
		topLeft.y = y + dx * sinRotation + dy * cosRotation;

		topRight.x = x + (dx + dest.width) * cosRotation - dy * sinRotation;
		topRight.y = y + (dx + dest.width) * sinRotation + dy * cosRotation;

		bottomLeft.x = x + dx * cosRotation - (dy + dest.height) * sinRotation;
		bottomLeft.y = y + dx * sinRotation + (dy + dest.height) * cosRotation;

		bottomRight.x = x + (dx + dest.width) * cosRotation - (dy + dest.height) * sinRotation;
		bottomRight.y = y + (dx + dest.width) * sinRotation + (dy + dest.height) * cosRotation;
	}

	rlSetTexture(texture->id);
	rlBegin(RL_QUADS);

	rlColor4ub(color.r, color.g, color.b, color.a);
	rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

	// Top-left corner for texture and quad
	if (flipX) rlTexCoord2f((source.x + source.width) / width, source.y / height);
	else rlTexCoord2f(source.x / width, source.y / height);
	rlVertex2f(topLeft.x, topLeft.y);

	// Bottom-left corner for texture and quad
	if (flipX) rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
	else rlTexCoord2f(source.x / width, (source.y + source.height) / height);
	rlVertex2f(bottomLeft.x, bottomLeft.y);

	// Bottom-right corner for texture and quad
	if (flipX) rlTexCoord2f(source.x / width, (source.y + source.height) / height);
	else rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
	rlVertex2f(bottomRight.x, bottomRight.y);

	// Top-right corner for texture and quad
	if (flipX) rlTexCoord2f(source.x / width, source.y / height);
	else rlTexCoord2f((source.x + source.width) / width, source.y / height);
	rlVertex2f(topRight.x, topRight.y);

	rlEnd();
	rlSetTexture(0);

}

// Draw a color-filled rectangle with pro parameters
void
SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color)
{
	float x = rec.x - origin.x;
	float y = rec.y - origin.y;
	Vector2 topLeft = Vector2{ x, y };
	Vector2 topRight = Vector2{ x + rec.width, y };
	Vector2 bottomLeft = Vector2{ x, y + rec.height };
	Vector2 bottomRight = Vector2{ x + rec.width, y + rec.height };

	rlBegin(RL_TRIANGLES);
		rlColor4f(color.x, color.y, color.z, color.w);

		rlVertex2f(topLeft.x, topLeft.y);
		rlVertex2f(bottomLeft.x, bottomLeft.y);
		rlVertex2f(topRight.x, topRight.y);

		rlVertex2f(topRight.x, topRight.y);
		rlVertex2f(bottomLeft.x, bottomLeft.y);
		rlVertex2f(bottomRight.x, bottomRight.y);
	rlEnd();
}

RenderTexture2D SLoadRenderTexture(int width, int height, PixelFormat format)
{
	return SLoadRenderTextureEx(width, height, format, false);
}

RenderTexture2D
SLoadRenderTextureEx(int width, int height, PixelFormat format, bool useDepth)
{
	RenderTexture2D target = { 0 };

	target.id = rlLoadFramebuffer(width, height);   // Load an empty framebuffer

	if (target.id > 0)
	{
		rlEnableFramebuffer(target.id);

		// Create color texture (default to RGBA)
		target.texture.id = rlLoadTexture(nullptr, width, height, format, 1);
		target.texture.width = width;
		target.texture.height = height;
		target.texture.format = format;
		target.texture.mipmaps = 1;

		if (useDepth)
		{
			// Create depth renderbuffer/texture
			target.depth.id = rlLoadTextureDepth(width, height, true);
			target.depth.width = width;
			target.depth.height = height;
			target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
			target.depth.mipmaps = 1;
		}

		// Attach color texture and depth renderbuffer/texture to FBO
		rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

		if (useDepth)
		{
			rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
		}

		// Check if fbo is complete with attachments (valid)
		if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

		rlDisableFramebuffer();
	}
	else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

	return target;
}

// Read a line from memory
// REQUIRES: memcpy()
// NOTE: Returns the number of bytes read
static int GetLine(const char* origin, char* buffer, int maxLength)
{
	int count = 0;
	for (; count < maxLength; count++) if (origin[count] == '\n') break;
	memcpy(buffer, origin, count);
	return count;
}

// Load a BMFont file (AngelCode font file)
// REQUIRES: strstr(), sscanf(), strrchr(), memcpy()
Font Scal_LoadBMFont(const char* fileName, Texture2D fontTexture, Vector2 offset)
{
#define MAX_BUFFER_SIZE     256

	Font font = { 0 };

	char buffer[MAX_BUFFER_SIZE] = { 0 };
	char* searchPoint = NULL;

	int fontSize = 0;
	int glyphCount = 0;

	int imWidth = 0;
	int imHeight = 0;
	char imFileName[129] = { 0 };

	int base = 0;   // Useless data

	char* fileText = LoadFileText(fileName);

	if (fileText == NULL) return font;

	char* fileTextPtr = fileText;

	// NOTE: We skip first line, it contains no useful information
	int lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
	fileTextPtr += (lineBytes + 1);

	// Read line data
	lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
	searchPoint = strstr(buffer, "lineHeight");
	sscanf(searchPoint, "lineHeight=%i base=%i scaleW=%i scaleH=%i", &fontSize, &base, &imWidth, &imHeight);
	fileTextPtr += (lineBytes + 1);

	TRACELOGD("FONT: [%s] Loaded font info:", fileName);
	TRACELOGD("    > Base size: %i", fontSize);
	TRACELOGD("    > Texture scale: %ix%i", imWidth, imHeight);

	lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
	searchPoint = strstr(buffer, "file");
	sscanf(searchPoint, "file=\"%128[^\"]\"", imFileName);
	fileTextPtr += (lineBytes + 1);

	TRACELOGD("    > Texture filename: %s", imFileName);

	lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
	searchPoint = strstr(buffer, "count");
	sscanf(searchPoint, "count=%i", &glyphCount);
	fileTextPtr += (lineBytes + 1);

	TRACELOGD("    > Chars count: %i", glyphCount);

	// Compose correct path using route of .fnt file (fileName) and imFileName
	char* imPath = NULL;
	char* lastSlash = NULL;

	lastSlash = (char*)strrchr(fileName, '/');
	if (lastSlash == NULL) lastSlash = (char*)strrchr(fileName, '\\');

	if (lastSlash != NULL)
	{
		// NOTE: We need some extra space to avoid memory corruption on next allocations!
		imPath = (char*)RL_CALLOC(TextLength(fileName) - TextLength(lastSlash) + TextLength(imFileName) + 4, 1);
		memcpy(imPath, fileName, TextLength(fileName) - TextLength(lastSlash) + 1);
		memcpy(imPath + TextLength(fileName) - TextLength(lastSlash) + 1, imFileName, TextLength(imFileName));
	}
	else imPath = imFileName;

	TRACELOGD("    > Image loading path: %s", imPath);

	font.texture = fontTexture;

	if (lastSlash != NULL) RL_FREE(imPath);

	Image imFont = LoadImageFromTexture(font.texture);

	// Fill font characters info data
	font.baseSize = fontSize;
	font.glyphCount = glyphCount;
	font.glyphPadding = 0;
	font.glyphs = (GlyphInfo*)RL_MALLOC(glyphCount * sizeof(GlyphInfo));
	font.recs = (Rectangle*)RL_MALLOC(glyphCount * sizeof(Rectangle));

	int charId, charX, charY, charWidth, charHeight, charOffsetX, charOffsetY, charAdvanceX;

	for (int i = 0; i < glyphCount; i++)
	{
		lineBytes = GetLine(fileTextPtr, buffer, MAX_BUFFER_SIZE);
		sscanf(buffer, "char id=%i x=%i y=%i width=%i height=%i xoffset=%i yoffset=%i xadvance=%i",
			&charId, &charX, &charY, &charWidth, &charHeight, &charOffsetX, &charOffsetY, &charAdvanceX);
		fileTextPtr += (lineBytes + 1);

		// Get character rectangle in the font atlas texture
		font.recs[i] = (Rectangle){ (float)charX + offset.x, (float)charY + offset.y, (float)charWidth, (float)charHeight };

		// Save data properly in sprite font
		font.glyphs[i].value = charId;
		font.glyphs[i].offsetX = charOffsetX;
		font.glyphs[i].offsetY = charOffsetY;
		font.glyphs[i].advanceX = charAdvanceX;

		// Fill character image data from imFont data
		font.glyphs[i].image = ImageFromImage(imFont, font.recs[i]);
	}

	UnloadImage(imFont);
	UnloadFileText(fileText);

	if (font.texture.id == 0)
	{
		UnloadFont(font);
		font = GetFontDefault();
		TRACELOG(LOG_WARNING, "FONT: [%s] Failed to load texture, reverted to default font", fileName);
	}
	else TRACELOG(LOG_INFO, "FONT: [%s] Font loaded successfully (%i glyphs)", fileName, font.glyphCount);

	return font;
}

void DrawRichText(Font font, const char* text, Vector2 position, float fontSize, float spacing, Color tint)
{
	PROFILE_BEGIN();
	if (font.texture.id == 0) font = GetFontDefault();  // Security check in case of not valid font

	int size = TextLength(text);    // Total size in bytes of the text, scanned by codepoints in loop

	int textOffsetY = 0;            // Offset between lines (on linebreak '\n')
	float textOffsetX = 0.0f;       // Offset X to next character to draw

	float scaleFactor = fontSize / font.baseSize;         // Character quad scaling factor

	for (int i = 0; i < size;)
	{
		// Get next codepoint from byte string and glyph index in font
		int codepointByteCount = 1;
		int codepoint = text[i];

		if (codepoint == '$' && text[i + 1] == '{')
		{
			const char* keywordStart = &text[i + 2];

			// Copies over the input ignoreing ${} chars.
			// If no } is found in length, then abort.
			short end = -1;
			char input[32] = {};
			for (short c = 0; c < ArrayLength(input); ++c)
			{
				char val = *(keywordStart + c);
				if (val == '}')
				{
					end = c;
					break;
				}
				else
					input[c] = val;
			}

			if (end == -1)
			{
				i += codepointByteCount;
				continue;
			}

			int params = 0;
			const char** split = TextSplit(input, ',', &params);
			SASSERT(params > 0);
			SASSERT(strlen(split[0]) == 1);

			int type = FastAtoi(split[0]);
			switch (type)
			{
			case(RICHTEXT_COLOR):
			{
				SASSERT(params == 2);
				uint32_t colorInt;
				Str2UInt(&colorInt, split[1], 16);
				tint = IntToColor(colorInt);
			}
			break;

			case(RICHTEXT_IMG):
			{
				SASSERT(params == 4);
				int id = FastAtoi(split[1]);
				int w = FastAtoi(split[2]);
				int h = FastAtoi(split[3]);

				Rectangle src = { 2, 52, 16, 16 };
				Rectangle dst = { position.x + textOffsetX, position.y + textOffsetY, (float)w, (float)h };
				DrawTexturePro(font.texture, src, dst, {}, 0, WHITE);

				textOffsetX += w * scaleFactor + spacing;
			}
			break;

			case(RICHTEXT_TOOLTIP):
			{
				SASSERT(params >= 2);
				// Seach spaces until we find space
				Rectangle wordRect;
				wordRect.x = position.x + textOffsetX;
				wordRect.y = position.y + textOffsetY;
				if (params > 2)
				{
					wordRect.width = (float)FastAtoi(split[2]);
					wordRect.height = (float)FastAtoi(split[3]);
				}
				else
				{
					wordRect.width = 32 * 10;
					wordRect.height = fontSize;

					for (short c = 0; c < 32; ++c)
					{
						char val = keywordStart[end + c];
						if (val == ' ')
						{
							wordRect.width = (float)c * 10;
							break;
						}
					}
				}

				Vector2 mouse = GetMousePosition();
				if (CheckCollisionPointRec(mouse, wordRect))
				{
					const char* tooltip = split[1];
					//Vector2 textSize = MeasureTextEx(font, tooltip, fontSize, spacing);
					Vector2 dst;
					dst.x = mouse.x + wordRect.width + 2;
					dst.y = mouse.y + wordRect.height + 2;
					DrawTextEx(font, tooltip, dst, fontSize, spacing, WHITE);
				}
			}
			break;

			default:
				SLOG_WARN("Using invalid type for RichType. Type: %d", type);
				break;
			}
			// +3 for ${ and } chars
			i += end + 3;
		}
		else
		{
			int index = GetGlyphIndex(font, codepoint);
			// NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
			// but we need to draw all the bad bytes using the '?' symbol moving one byte
			if (codepoint == 0x3f) codepointByteCount = 1;

			if (codepoint == '\n')
			{
				// NOTE: Fixed line spacing of 1.5 line-height
				// TODO: Support custom line spacing defined by user
				textOffsetY += (int)((font.baseSize + font.baseSize / 2.0f) * scaleFactor);
				textOffsetX = 0.0f;
			}
			else
			{
				if ((codepoint != ' ') && (codepoint != '\t'))
				{
					DrawTextCodepoint(font, codepoint, (Vector2) { position.x + textOffsetX, position.y + textOffsetY }, fontSize, tint);
				}

				if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing);
				else textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing);
			}

			i += codepointByteCount;   // Move text bytes counter to next codepoint
		}
	}
	PROFILE_END();
}
