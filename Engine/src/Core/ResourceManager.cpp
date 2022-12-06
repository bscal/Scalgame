#include "ResourceManager.h"

#define NUM_OF_FONT_GLYPHS 95

#define ENTITY_SHEET_PATH "assets/textures/SpriteSheet.png"
#define TILE_MAP_TEXTURE_PATH "assets/textures/tiles/16x16.png"

#define FONT_PATH "assets/textures/fonts/Pixuf.ttf"
#define SDF_FONT_PATH "assets/textures/fonts/UbuntuMono/UbuntuMono-Regular.ttf"

internal SDFFont LoadSDFFont();

bool InitializeResources(Resources* resources)
{
    resources->EntitySpriteSheet = LoadTexture(ENTITY_SHEET_PATH);
	resources->TileSheet = LoadTexture(TILE_MAP_TEXTURE_PATH);

	resources->MainFontM = LoadFont(FONT_PATH);
    resources->MainFontS = LoadFontEx(FONT_PATH, 16, 0, 0);
    resources->FontSilver = LoadFont(SDF_FONT_PATH);

	LoadTileSet(&resources->TileSheet, 16, 16,
		&resources->MainTileSet);

	TraceLog(LOG_INFO, "Initialized Resources");
	return resources->IsInitialized = true;
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