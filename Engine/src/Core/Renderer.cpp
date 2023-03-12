#include "Renderer.h"

#include "Game.h"
#include "ResourceManager.h"

#include "raylib/src/rlgl.h"
#include "raylib/src/raymath.h"

void Renderer::Initialize()
{
	int screenW = GetScreenWidth();
	int screenH = GetScreenHeight();
	int blurWidth = screenW / 4;
	int blurHeight = screenH / 4;
	BlurShader.Initialize(blurWidth, blurHeight);

	WorldTexture = SLoadRenderTexture(screenW, screenH, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	EffectTextureOne = SLoadRenderTexture(screenW, screenH, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	EffectTextureTwo = SLoadRenderTexture(screenW, screenH, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);

	UnlitShader = LoadShader(
		"assets/shaders/tile_shader.vert",
		"assets/shaders/tile_shader.frag");

	LitShader = LoadShader(
		"assets/shaders/tile_lit.vert",
		"assets/shaders/tile_lit.frag");

	LightingShader = LoadShader(
		"assets/shaders/lighting.vert",
		"assets/shaders/lighting.frag");

	BrightnessShader = LoadShader(
		"assets/shaders/tile_shader.vert",
		"assets/shaders/brightness_filter.frag");

	BloomShader = LoadShader(
		"assets/shaders/bloom.vert",
		"assets/shaders/bloom.frag");

	UniformAmbientLightLoc = GetShaderLocation(LitShader, "ambientLightColor");
	UniformLightMapLoc = GetShaderLocation(LitShader, "texture1");
	UniformLightIntensityLoc = GetShaderLocation(LightingShader, "lightIntensity");
	UniformSunLightColorLoc = GetShaderLocation(LightingShader, "sunLightColor");
	UniformLightTexLoc = GetShaderLocation(BloomShader, "texture1");
	UniformBloomIntensityLoc = GetShaderLocation(BloomShader, "bloomIntensity");

	SetValueAndUniformAmbientLight({ 0.1f, 0.1f, 0.2f, 1.0f });
	SetValueAndUniformSunLight({ 186.f / 255.f,  186.f / 255.f,  169.f / 255.f, 1.0f });
	SetValueAndUniformLightIntensity(1.0f);
	SetValueAndUniformBloomIntensity(1.0f);

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
	UnloadShader(LightingShader);
	UnloadShader(BrightnessShader);
	UnloadShader(BloomShader);
}

void Renderer::PostProcess(Game* game, const RenderTexture2D& worldTexture,
	const RenderTexture2D& lightingTexture) const
{
	float screenW = (float)GetScreenWidth();
	float screenH = (float)GetScreenHeight();
	Rectangle srcRect = { 0.0f, 0.0f, screenW, -screenH };
	Rectangle screenRect = { 0.0f, 0.0f, screenW, screenH };

	// Brightness pass
	BeginShaderMode(BrightnessShader);
	BeginTextureMode(EffectTextureTwo);
	ClearBackground(BLACK);
	DrawTexturePro(lightingTexture.texture, srcRect, screenRect, { 0 }, 0.f, WHITE);
	EndTextureMode();
	EndShaderMode();

	// Blur pass
	BlurShader.Draw(EffectTextureTwo.texture);

	BeginShaderMode(BloomShader);
	BeginTextureMode(EffectTextureTwo);
	ClearBackground(BLACK);
	SetShaderValueTexture(BloomShader, UniformLightTexLoc, BlurShader.TextureVert.texture);
	DrawTexturePro(lightingTexture.texture, srcRect, screenRect, { 0 }, 0.f, WHITE);
	EndTextureMode();
	EndShaderMode();
}

void Renderer::SetValueAndUniformAmbientLight(Vector4 ambientLight)
{
	AmbientLight = ambientLight;
	SetShaderValue(LitShader, UniformAmbientLightLoc, &AmbientLight, SHADER_UNIFORM_VEC4);
}

void Renderer::SetValueAndUniformSunLight(Vector4 sunLight)
{
	SunLight = sunLight;
	SetShaderValue(LightingShader, UniformSunLightColorLoc, &SunLight, SHADER_UNIFORM_VEC4);
}

void Renderer::SetValueAndUniformLightIntensity(float intensity)
{
	LightIntensity = intensity;
	SetShaderValue(LightingShader, UniformLightIntensityLoc, &LightIntensity, SHADER_UNIFORM_FLOAT);
}

void Renderer::SetValueAndUniformBloomIntensity(float intensity)
{
	BloomIntensity = intensity;
	SetShaderValue(LightingShader, UniformLightIntensityLoc, &BloomIntensity, SHADER_UNIFORM_FLOAT);
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
	ClearBackground(BLACK);
	DrawTextureProF(lightingTexture, srcRect, blurRectDest, { 0 }, 0.0f, colorWhite);
	EndTextureMode();

	isHorizontal = 0;
	SetShaderValue(BlurShader, UniformIsHorizontalLocation, &isHorizontal, SHADER_UNIFORM_INT);

	BeginTextureMode(TextureVert);
	ClearBackground(BLACK);
	DrawTextureProF(TextureHorizontal.texture, blurRectSrc, blurRectDest, { 0 }, 0.0f, colorWhite);
	EndTextureMode();

	EndShaderMode();
}

void BlurShader::Free()
{
	UnloadRenderTexture(TextureHorizontal);
	UnloadRenderTexture(TextureVert);
	UnloadShader(BlurShader);
}

void TileMapRenderer::Initialize(Game* game)
{
	TileMapShader = LoadShader(
		"assets/shaders/tilemap.vert",
		"assets/shaders/tilemap.frag");

	UniformViewOffsetLoc = GetShaderLocation(TileMapShader, "viewOffset");
	UniformViewPortSizeLoc = GetShaderLocation(TileMapShader, "viewportSize");
	UniformInverseTileTextureSizeLoc = GetShaderLocation(TileMapShader, "inverseTileTextureSize");
	UniformInverseTileSizeLoc = GetShaderLocation(TileMapShader, "inverseTileSize");
	UniformTilesLoc = GetShaderLocation(TileMapShader, "mapData");
	UniformSpriteLoc = GetShaderLocation(TileMapShader, "textureAtlas");
	UniformInverseSpriteTextureSizeLoc = GetShaderLocation(TileMapShader, "inverseSpriteTextureSize");
	UniformTileSizeLoc = GetShaderLocation(TileMapShader, "tileSize");

	//Vector2 viewPortSize = { (float)GetScreenWidth(), (float)GetScreenHeight() };
	//SetShaderValue(TileMapShader, UniformViewPortSizeLoc, &viewPortSize, RL_SHADER_UNIFORM_VEC2);

	//Vector2 inverseTileTextureSize = { 1. / (float)8, 1. / (float)8 };
	//SetShaderValue(TileMapShader, UniformInverseTileTextureSizeLoc, &inverseTileTextureSize, RL_SHADER_UNIFORM_VEC2);

	//float inverseTileSize = 1.f / 16.0f;
	//SetShaderValue(TileMapShader, UniformInverseTileSizeLoc, &inverseTileSize, RL_SHADER_UNIFORM_FLOAT);

	//Vector2 inverseSpriteTextureSize = { 1. / (512.f), 1. / (512.f)};
	//SetShaderValue(TileMapShader, UniformInverseSpriteTextureSizeLoc, &inverseSpriteTextureSize, RL_SHADER_UNIFORM_VEC2);

	//float tileSize = 16.0f;
	//SetShaderValue(TileMapShader, UniformTileSizeLoc, &tileSize, RL_SHADER_UNIFORM_FLOAT);

	TileMapTexture = LoadRenderTexture(SCREEN_WIDTH_TILES, SCREEN_HEIGHT_TILES);
	SetTextureFilter(TileMapTexture.texture, TEXTURE_FILTER_POINT);

	TileMapInfo info = {};
	info.x = 1;
	info.y = 27;
	Tiles.fill(info);
	UpdateTexture(TileMapTexture.texture, Tiles.data());
}

void TileMapRenderer::Free()
{
	UnloadShader(TileMapShader);
	UnloadRenderTexture(TileMapTexture);
}


void TileMapRenderer::Draw() const
{
	BeginShaderMode(TileMapShader);

	SetShaderValueTexture(TileMapShader, UniformSpriteLoc, GetGame()->Resources.TileSheet);

	SetShaderValueTexture(TileMapShader, UniformTilesLoc, TileMapTexture.texture);

	// Rectangle r = GetGame()->CullingRect;

	 //Vector2 viewOffset = { 0.0f, 0.0f };
	 //SetShaderValue(TileMapShader, UniformViewOffsetLoc, &viewOffset, RL_SHADER_UNIFORM_VEC2);

	DrawTexturePro(GetGame()->Resources.TileSprite, { 0, 0, 16.f, 16.f }, { 0, 0, (float)SCREEN_WIDTH_TILES * 16,  (float)SCREEN_HEIGHT_TILES * 16 }, { 0 }, 0.0f, WHITE);

	EndShaderMode();
}

void LightingRenderer::Initialize(Game* game)
{
	LightingShader = LoadShader(
		"assets/shaders/lighting_v2.vert",
		"assets/shaders/lighting_v2.frag");

	UniformLightIntensity = GetShaderLocation(LightingShader, "lightIntensity");
	UniformSunlight = GetShaderLocation(LightingShader, "sunLightColor");

	int width = SCREEN_WIDTH_PADDING;
	int height = SCREEN_HEIGHT_PADDING;
	LightingTexture = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	ColorsTexture = SLoadRenderTexture(width / TILE_SIZE, height / TILE_SIZE, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
}

void LightingRenderer::Free()
{
	UnloadShader(LightingShader);
	UnloadRenderTexture(ColorsTexture);
	UnloadRenderTexture(LightingTexture);
}

void LightingRenderer::Draw() const
{
	Rectangle src;
	src.x = 0;
	src.y = 0;
	src.width = ColorsTexture.texture.width;
	src.height = ColorsTexture.texture.height;

	Rectangle dst;
	dst.x = 0;
	dst.y = 0;
	dst.width = LightingTexture.texture.width;
	dst.height = LightingTexture.texture.height;

	auto colors = &GetGame()->LightMap.LightColors;
	SASSERT(sizeof(LightInfo) == (sizeof(float) * 4));
	SASSERT(colors->size() == src.width * src.height);

	UpdateTexture(ColorsTexture.texture, colors);

	colors->fill(LightInfo{ 0 });

	BeginTextureMode(LightingTexture);

	BeginShaderMode(LightingShader);

	DrawTextureProF(ColorsTexture.texture, src, dst, { 0 }, 0.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

	EndShaderMode();

	EndTextureMode();
}

void DrawTextureProF(Texture2D texture, Rectangle source, Rectangle dest,
	Vector2 origin, float rotation, Vector4 tint)
{
	// Check if texture is valid
	if (texture.id > 0)
	{
		float width = (float)texture.width;
		float height = (float)texture.height;

		bool flipX = false;

		if (source.width < 0) { flipX = true; source.width *= -1; }
		if (source.height < 0) source.y -= source.height;

		Vector2 topLeft = { 0 };
		Vector2 topRight = { 0 };
		Vector2 bottomLeft = { 0 };
		Vector2 bottomRight = { 0 };

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

		rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

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

void ScalDrawTextureProF(const Texture2D* texture,
	Rectangle source, Rectangle dest, Vector4 tint)
{
	SASSERT(texture->id > 0);

	float width = (float)texture->width;
	float height = (float)texture->height;

	Vector2 topLeft = Vector2{ dest.x, dest.y };
	Vector2 topRight = Vector2{ dest.x + dest.width, dest.y };
	Vector2 bottomLeft = Vector2{ dest.x, dest.y + dest.height };
	Vector2 bottomRight = Vector2{ dest.x + dest.width, dest.y + dest.height };

	rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

	rlSetTexture(texture->id);
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

// Draw a color-filled rectangle with pro parameters
void SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color)
{
	Vector2 topLeft = { 0 };
	Vector2 topRight = { 0 };
	Vector2 bottomLeft = { 0 };
	Vector2 bottomRight = { 0 };

	float x = rec.x - origin.x;
	float y = rec.y - origin.y;
	topLeft = (Vector2){ x, y };
	topRight = (Vector2){ x + rec.width, y };
	bottomLeft = (Vector2){ x, y + rec.height };
	bottomRight = (Vector2){ x + rec.width, y + rec.height };

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
		target.texture.id = rlLoadTexture(NULL, width, height, format, 1);
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


void DrawTileMap(Texture2D texture, Rectangle dest)
{
	//SASSERT(texture.id > 0);

	//rlBatch
	//float width = (float)texture.width;
	//float height = (float)texture.height;

	//Vector2 topLeft = Vector2{ dest.x, dest.y };
	//Vector2 topRight = Vector2{ dest.x + dest.width, dest.y };
	//Vector2 bottomLeft = Vector2{ dest.x, dest.y + dest.height };
	//Vector2 bottomRight = Vector2{ dest.x + dest.width, dest.y + dest.height };

	//rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

	//rlSetTexture(texture->id);
	//rlBegin(RL_QUADS);

	//rlColor4f(tint.x, tint.y, tint.z, tint.w);
	//rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

	//rlTexCoord2f(source.x / width, source.y / height);
	//rlVertex2f(topLeft.x, topLeft.y);

	//rlTexCoord2f(source.x / width, (source.y + source.height) / height);
	//rlVertex2f(bottomLeft.x, bottomLeft.y);

	//rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
	//rlVertex2f(bottomRight.x, bottomRight.y);

	//rlTexCoord2f((source.x + source.width) / width, source.y / height);
	//rlVertex2f(topRight.x, topRight.y);

	//rlEnd();
	//rlSetTexture(0);

}

// Draw render batch
// NOTE: We require a pointer to reset batch and increase current buffer (multi-buffer)
//
