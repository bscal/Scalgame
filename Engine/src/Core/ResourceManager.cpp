#include "ResourceManager.h"

#include "RenderExtensions.h"

#define NUM_OF_FONT_GLYPHS 95

#define ENTITY_SHEET_PATH "assets/textures/SpriteSheet.png"
#define TILE_MAP_TEXTURE_PATH "assets/textures/tiles/16x16.png"

#define FONT_PATH "assets/textures/fonts/Pixuf.ttf"
#define SDF_FONT_PATH "assets/textures/fonts/UbuntuMono/UbuntuMono-Regular.ttf"

internal SDFFont LoadSDFFont();

bool InitializeResources(Resources* resources)
{
    resources->Atlas.Load("assets/textures/atlas/tiles.atlas", 32);

    resources->EntitySpriteSheet = LoadTexture(ENTITY_SHEET_PATH);

	resources->MainFontM = LoadFont(FONT_PATH);
    resources->MainFontS = LoadFontEx(FONT_PATH, 16, 0, 0);
    resources->FontSilver = LoadFontEx(SDF_FONT_PATH, 16, 0, 0);

    resources->UnlitShader = LoadShader(
        "assets/shaders/tile_shader.vert",
        "assets/shaders/tile_shader.frag");

    resources->BrightnessShader = LoadShader(
        "assets/shaders/tile_shader.vert",
        "assets/shaders/brightness_filter.frag");

    resources->Blur.Initialize(GetScreenWidth() / 4, GetScreenHeight() / 4);

	SLOG_INFO("[ RESOURCES ] Successfully initialized resources!");
	return resources->IsAllocated = true;
}

void FreeResouces(Resources* resources)
{
    UnloadFont(resources->FontSilver);
    UnloadFont(resources->MainFontM);
    UnloadTexture(resources->EntitySpriteSheet);
    UnloadShader(resources->UnlitShader);
    resources->Atlas.Unload();
    resources->Blur.Free();
}

void BlurShader::Initialize(int width, int height)
{
    BlurShader = LoadShader(
        "assets/shaders/blur.vert",
        "assets/shaders/blur.frag");

    TextureHorizontal = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    TextureVert = SLoadRenderTexture(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);

    UniformIsHorizontalLocation = GetShaderLocation(BlurShader, "IsHorizontal");

    int targetWidthLoc = GetShaderLocation(BlurShader, "TargetWidth");
    SetShaderValue(BlurShader, targetWidthLoc, &width, SHADER_UNIFORM_FLOAT);
}

void BlurShader::Draw(const Texture2D& worldTexture) const
{
    Rectangle srcRect =
    {
        0.0f,
        0.0f,
        (float)worldTexture.width,
        -(float)worldTexture.height,
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

    int isHorizontalTrue = 1;
    SetShaderValue(BlurShader, UniformIsHorizontalLocation, &isHorizontalTrue, SHADER_UNIFORM_INT);

    BeginTextureMode(TextureHorizontal);
    ClearBackground(BLACK);
    DrawTextureProF(worldTexture, srcRect, blurRectDest, { 0 }, 0.0f, colorWhite);
    EndTextureMode();

    int isHorizontalFalse = 0;
    SetShaderValue(BlurShader, UniformIsHorizontalLocation, &isHorizontalFalse, SHADER_UNIFORM_INT);

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

internal SDFFont LoadSDFFont()
{
    // Loading file to memory
    unsigned int fileSize = 0;
    unsigned char* fileData = LoadFileData(FONT_PATH, &fileSize);

    // Default font generation from TTF font
    Font fontDefault = { 0 };
    fontDefault.baseSize = 16;
    fontDefault.glyphCount = 95;

    // Loading font data from memory data
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 95 (autogenerate chars array)
    fontDefault.glyphs = LoadFontData(fileData, fileSize, 16, 0, 95, FONT_DEFAULT);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 4 px, pack method: 0 (default)
    Image atlas = GenImageFontAtlas(fontDefault.glyphs, &fontDefault.recs, 95, 16, 4, 0);
    fontDefault.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);

    // SDF font generation from TTF font
    Font fontSDF = { 0 };
    fontSDF.baseSize = 16;
    fontSDF.glyphCount = 95;
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 0 (defaults to 95)
    fontSDF.glyphs = LoadFontData(fileData, fileSize, 16, 0, 0, FONT_SDF);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 0 px, pack method: 1 (Skyline algorythm)
    atlas = GenImageFontAtlas(fontSDF.glyphs, &fontSDF.recs, 95, 16, 0, 1);
    fontSDF.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);

    UnloadFileData(fileData);      // Free memory from loaded file

    // Load SDF required shader (we use default vertex shader)
    Shader shader = LoadShader(0, "assets/shaders/sdf.fs");
    SetTextureFilter(fontSDF.texture, TEXTURE_FILTER_BILINEAR);    // Required for SDF font

    SDFFont sdfFont;
    sdfFont.Font = fontSDF;
    sdfFont.Shader = shader;
    return sdfFont;
}