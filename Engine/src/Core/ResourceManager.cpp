#include "ResourceManager.h"

#include "SMemory.h"
#include "Renderer.h"
#include "SString.h"

#include "raylib/src/rlgl.h"

#define NUM_OF_FONT_GLYPHS 95

#define ENTITY_SHEET_PATH TEXTURES_PATH "SpriteSheet.png"
#define TILE_MAP_TEXTURE_PATH TEXTURES_PATH "tiles/16x16.png"

#define FONT_PATH TEXTURES_PATH "fonts/Pixuf.ttf"
#define SDF_FONT_PATH TEXTURES_PATH "fonts/UbuntuMono/UbuntuMono-Regular.ttf"

bool InitializeResources(Resources* resources)
{
    resources->UIAtlas = SpriteAtlasLoad(TEXTURES_PATH, "UIAtlas.atlas");

    Rectangle fontRect = SpriteAtlasRect(&resources->UIAtlas, RawStringNew("Silver", ALLOC_TEMP));
    Rectangle squareRect = SpriteAtlasRect(&resources->UIAtlas, RawStringNew("TileSprite", ALLOC_TEMP));

    resources->EntitySpriteSheet = LoadTexture(ENTITY_SHEET_PATH);
    resources->TileSheet = LoadTexture(TILE_MAP_TEXTURE_PATH);
    SetTextureFilter(resources->TileSheet, TEXTURE_FILTER_POINT);
    resources->TileSprite = LoadTexture(TEXTURES_PATH "tiles/TileSprite.png");

    SetTextureFilter(resources->TileSheet, TEXTURE_FILTER_POINT);

	resources->MainFontM = LoadFont(FONT_PATH);
    resources->MainFontS = LoadFontEx(FONT_PATH, 16, 0, 0);

    resources->FontSilver = Scal_LoadBMFont("assets/textures/fonts/Silver.fnt", resources->UIAtlas.Texture, { fontRect.x, fontRect.y });
    
    SetShapesTexture(resources->UIAtlas.Texture, { squareRect.x, squareRect.y, 1.0f, 1.0f });

	SLOG_INFO("[ RESOURCES ] Successfully initialized resources!");
	return true;
}

void FreeResouces(Resources* resources)
{
    UnloadFont(resources->FontSilver);
    UnloadFont(resources->MainFontS);
    UnloadFont(resources->MainFontM);
    UnloadTexture(resources->TileSheet);
    UnloadTexture(resources->TileSprite);
    UnloadTexture(resources->EntitySpriteSheet);
    SpriteAtlasUnload(&resources->UIAtlas);
}

uint32_t LoadComputeShader(const char* file)
{
    SASSERT(FileExists(file));
    uint32_t result = 0;
    char* fileData = LoadFileText(file);
    if (fileData)
    {
        uint32_t shader = rlCompileShader(fileData, RL_COMPUTE_SHADER);
        result = rlLoadComputeShaderProgram(shader);
    }
    else
    {
        SLOG_ERR("Could not find file: %s", file);
    }
    SASSERT(result > 0);
    UnloadFileText(fileData);
    return result;
}
