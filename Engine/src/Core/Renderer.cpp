#include "Renderer.h"

#include "Game.h"
#include "ResourceManager.h"

#include "raylib/src/rlgl.h"
#include "raylib/src/raymath.h"

void Renderer::Initialize()
{
	int sW = GetScreenWidth();
	int sH = GetScreenHeight();
	int screenW = CULL_WIDTH;
	int screenH = CULL_HEIGHT;
	int blurWidth = sW / 4;
	int blurHeight = sH / 4;
	BlurShader.Initialize(blurWidth, blurHeight);

	WorldTexture = SLoadRenderTexture(screenW, screenH, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	EffectTextureOne = SLoadRenderTexture(sW, sH, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	EffectTextureTwo = SLoadRenderTexture(sW, sH, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);

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
	Rectangle srcRect = { 0.0f, 0.0f, screenW, -screenH };
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
	ClearBackground({});
	DrawTextureProF(lightingTexture, srcRect, blurRectDest, { 0 }, 0.0f, colorWhite);
	EndTextureMode();

	isHorizontal = 0;
	SetShaderValue(BlurShader, UniformIsHorizontalLocation, &isHorizontal, SHADER_UNIFORM_INT);

	BeginTextureMode(TextureVert);
	ClearBackground({});
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

	UniformTilesLoc = GetShaderLocation(TileMapShader, "mapData");
	UniformSpriteLoc = GetShaderLocation(TileMapShader, "textureAtlas");

	int width = CULL_WIDTH;
	int height = CULL_HEIGHT;
	TileMapTexture = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	TileDataTexture = SLoadRenderTexture(width / TILE_SIZE, height / TILE_SIZE, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
}

void TileMapRenderer::Free()
{
	UnloadShader(TileMapShader);
	UnloadRenderTexture(TileMapTexture);
	UnloadRenderTexture(TileDataTexture);
}

void TileMapRenderer::Draw()
{
	UpdateTexture(TileDataTexture.texture, Tiles.data());

	SMemSet(Tiles.data(), 0, sizeof(Tiles[0]) * Tiles.size());

	Tiles.fill({ 0 });

	BeginTextureMode(TileMapTexture);
	BeginShaderMode(TileMapShader);

	SetShaderValueTexture(TileMapShader, UniformSpriteLoc, GetGame()->Resources.TileSheet);
	SetShaderValueTexture(TileMapShader, UniformTilesLoc, TileDataTexture.texture);
	const Texture2D& tileSprite = GetGame()->Resources.TileSprite;
	Rectangle src = { 0, 0, (float)tileSprite.width, (float)tileSprite.height };
	Rectangle dst = { 0, 0, (float)CULL_WIDTH,  (float)CULL_HEIGHT };
	DrawTexturePro(tileSprite, src, dst, { 0 }, 0.0f, WHITE);

	EndShaderMode();
	EndTextureMode();
}

void LightingRenderer::Initialize(Game* game)
{
	LightingShader = LoadShader(
		SHADERS_PATH "lighting_v2.vert",
		SHADERS_PATH "lighting_v2.frag");

	UniformSunlight = GetShaderLocation(LightingShader, "sunlightColor");
	UniformLOSColor = GetShaderLocation(LightingShader, "losColor");
	UniformWorldMap = GetShaderLocation(LightingShader, "tileDataMap");

	AmbientLightColor = { 25.0f / 255, 20.0 / 255, 45.0f / 255 };
	SunlightColor = { 0.0f, 0.0f, 0.0f };
	LOSColor = { 0.0f, 0.0f, 0.0f };
	LightIntensity = 1.0f;

	int width = CULL_WIDTH;
	int height = CULL_HEIGHT;
	LightingTexture = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
	ColorsTexture = SLoadRenderTexture(width / TILE_SIZE, height / TILE_SIZE, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
}

void LightingRenderer::Free()
{
	UnloadShader(LightingShader);
	UnloadRenderTexture(ColorsTexture);
	UnloadRenderTexture(LightingTexture);
}

void LightingRenderer::Draw()
{
	Rectangle src;
	src.x = 0;
	src.y = 0;
	src.width = ColorsTexture.texture.width;
	src.height = ColorsTexture.texture.height;

	Rectangle dst;
	dst.x = GetGameApp()->CullXY.x - HALF_TILE_SIZE;
	dst.y = GetGameApp()->CullXY.y - HALF_TILE_SIZE;
	dst.width = LightingTexture.texture.width;
	dst.height = LightingTexture.texture.height;

	SASSERT(sizeof(Tiles[0]) == (sizeof(float) * 4));
	SASSERT(Tiles.size() == src.width * src.height);

	UpdateTexture(ColorsTexture.texture, Tiles.data());

	Tiles.fill({ 0 });

	BeginTextureMode(LightingTexture);
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
	SetShaderValueTexture(LightingShader, UniformWorldMap, GetGame()->TileMapRenderer.TileDataTexture.texture);
	DrawTextureProF(ColorsTexture.texture, src, dst, { 0 }, 0.0f, color);
	EndMode2D();

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

void SDrawTextureProF(const Texture2D* texture, Rectangle source, Rectangle dest, Vector4 tint)
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
