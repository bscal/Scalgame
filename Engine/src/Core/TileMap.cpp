#include "TileMap.h"

#include "Game.h"
#include "SMemory.h"
#include "raymath.h"

#include <cassert>

bool InitializeTileMap(TileSet* tileSet,
	uint32_t width, uint32_t height,
	uint16_t tileSize, TileMap* outTileMap)
{
	outTileMap->TileSet = tileSet;
	outTileMap->MapWidth = width;
	outTileMap->MapHeight = height;
	outTileMap->MapTileSize = tileSize;
	outTileMap->MapHalfTileSize = (float)tileSize / 2.0f;
	outTileMap->MapTiles =
		(Tile*)Scal::Memory::Alloc(width * height * sizeof(Tile));

	TraceLog(LOG_INFO, "Initialized TileMap");

	return true;
}

bool LoadTileSet(Texture2D* tileTexture,
	uint16_t tileSizeWidth, uint16_t tileSizeHeight,
	TileSet* outTileSet)
{
	outTileSet->TileTexture = *tileTexture;
	outTileSet->TextureTileWidth = tileSizeWidth;
	outTileSet->TextureTileHeight = tileSizeHeight;

	int width = tileTexture->width / tileSizeWidth;
	int height = tileTexture->height / tileSizeHeight;
	int totalTiles = width * height;
	outTileSet->TileTypes =
		(TileType*)Scal::Memory::Alloc(totalTiles * sizeof(TileType));

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int i = x + y * width;
			float xTexturePos = (float)x * (float)tileSizeWidth;
			float yTexturePos = (float)y * (float)tileSizeHeight;
			float rectX = xTexturePos;
			float rectY = yTexturePos;
			float rectW = tileSizeWidth;
			float rectH = tileSizeHeight;
			Rectangle rect = { rectX, rectY, rectW, rectH };

			TileType tileType{};
			tileType.TextureSrcRectangle = rect;
			tileType.MovementCost = 1;

			outTileSet->TileTypes[i] = tileType;
		}
	}

	TraceLog(LOG_INFO, "Loaded TileSet %d with %d tiles",
		tileTexture->id, totalTiles);

	return true;
}

void LoadTileMap(TileMap* tileMap)
{
	SetRandomSeed(0);
	for (uint32_t y = 0; y < tileMap->MapHeight; ++y)
	{
		for (uint32_t x = 0; x < tileMap->MapWidth; ++x)
		{
			uint32_t index = x + y * tileMap->MapWidth;
			int tileId = GetRandomValue(1, 2);

			Tile tile{};
			tile.TileId = (uint32_t)tileId;
			tile.Fow = FOWLevel::NoVision;
			tileMap->MapTiles[index] = tile;
		}
	}
}

void UnloadTileMap(TileMap* tileMap)
{
	MemFree(tileMap->MapTiles);
	MemFree(tileMap);
}

// TODO remove
global_var Tile* OutTiles[5 * 5 * 5];
global_var bool DisableFow = false;

void RenderTileMap(Game* game, TileMap* tileMap)
{
	float fowValues[5] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
	float fowAlphas[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 colorFullVision = ColorNormalize(WHITE);
	Vector4 colorNoVision = ColorNormalize(DARKGRAY);

	Texture2D mapTexture = tileMap->TileSet->TileTexture;

	Player p = game->Player;
	GetSurroundingTilesRadius(
		tileMap,
		p.TilePosition.x,
		p.TilePosition.y,
		5,
		OutTiles);

	for (int i = 0; i < 5 * 5 * 5; ++i)
	{
		Tile* tile = OutTiles[i];
		if (tile == nullptr) continue;
		tile->Fow = FOWLevel::FullVision;
	}

	if (IsKeyPressed(KEY_F1))
		DisableFow = !DisableFow;

	for (uint32_t y = 0; y < tileMap->MapHeight; ++y)
	{
		for (uint32_t x = 0; x < tileMap->MapWidth; ++x)
		{
			uint32_t index = x + y * tileMap->MapWidth;
			float xPos = (float)x * (float)tileMap->MapTileSize;
			float yPos = (float)y * (float)tileMap->MapTileSize;
			Tile tile = tileMap->MapTiles[index];
			uint32_t tileId = tile.TileId;
			Rectangle textRect =
				tileMap->TileSet->TileTypes[tileId].TextureSrcRectangle;
			Vector2 pos = { xPos, yPos };

			float fowValue = fowValues[(uint8_t)tile.Fow];
			Vector4 finalColor;
			finalColor.x = Lerp(colorNoVision.x, colorFullVision.x, fowValue);
			finalColor.y = Lerp(colorNoVision.y, colorFullVision.y, fowValue);
			finalColor.z = Lerp(colorNoVision.z, colorFullVision.z, fowValue);
			finalColor.w = fowAlphas[(uint8_t)tile.Fow];

			Color color = ColorFromNormalized(finalColor);
			#if SCAL_DEBUG
			if (DisableFow) color = WHITE;
			#endif

			DrawTextureRec(mapTexture, textRect, pos, color);
			tileMap->MapTiles[index].Fow = FOWLevel::NoVision;
		}
	}

	DrawRectangleLinesEx(
		{ 0, 0,
		(float)tileMap->MapWidth * (float)tileMap->MapTileSize,
		(float)tileMap->MapHeight * (float)tileMap->MapTileSize
		},
		2.5f,
		RED);
}

bool IsInBounds(int x, int y, int width, int height)
{
	return x >= 0 && x < width&& y >= 0 && y < height;
}

Tile* GetTile(TileMap* tileMap, int x, int y)
{
	assert(x >= 0 && x < tileMap->MapWidth);
	assert(y >= 0 && y < tileMap->MapHeight);

	int index = x + y * tileMap->MapWidth;
	return &tileMap->MapTiles[index];
}

void SetTile(TileMap* tileMap, int x, int y, Tile* srcTile)
{
	assert(x >= 0 && x < tileMap->MapWidth);
	assert(y >= 0 && y < tileMap->MapHeight);

	int index = x + y * tileMap->MapWidth;
	tileMap->MapTiles[index] = *srcTile;
}

void GetSurroundingTilesBox(TileMap* tileMap, int x, int y, int boxWidth, int boxHeight,
	Tile* outTiles[])
{
	int startX = x - boxWidth;
	int startY = y - boxHeight;
	int endX = x + boxWidth;
	int endY = y + boxHeight;
	int i = 0;
	for (int yi = startY; yi <= endY; ++yi)
	{
		for (int xi = startX; xi <= endX; ++xi)
		{
			if (IsInBounds(xi, yi, tileMap->MapWidth, tileMap->MapHeight))
				outTiles[i] = &tileMap->MapTiles[xi + yi * tileMap->MapWidth];
			else
				outTiles[i] = nullptr;
			++i;
		}
	}
}

// bounds check for array
void GetSurroundingTilesRadius(TileMap* tileMap,
	int x, int y,
	float radius,
	Tile** outTiles)
{
	int startX = x - (int)radius;
	int startY = y - (int)radius;
	int endX = x + (int)radius;
	int endY = y + (int)radius;
	int i = 0;
	for (int yi = startY; yi <= endY; ++yi)
	{
		for (int xi = startX; xi <= endX; ++xi)
		{
			if (IsInBounds(xi, yi, tileMap->MapWidth, tileMap->MapHeight)
				&& Distance(x, y, xi, yi) < radius)
				outTiles[i] = &tileMap->MapTiles[xi + yi * tileMap->MapWidth];
			else
				outTiles[i] = nullptr;
			++i;
		}
	}
}

void PlotLine(int x1, int y1, int x2, int y2)
{
	int delta_x(x2 - x1);
	// if x1 == x2, then it does not matter what we set here
	signed char const ix = ((delta_x > 0) - (delta_x < 0));
	delta_x = abs(delta_x) << 1;

	int delta_y(y2 - y1);
	// if y1 == y2, then it does not matter what we set here
	signed char const iy = ((delta_y > 0) - (delta_y < 0));
	delta_y = abs(delta_y) << 1;

	DrawCircle(x1, y1, 1.0f, GREEN);

	// error may go below zero
	int error = (delta_y - (delta_x >> 1));

	if (delta_x >= delta_y)
	{
		while (x1 != x2)
		{
			// reduce error, while taking into account the corner case of error == 0
			if ((error > 0) || (!error && (ix > 0)))
			{
				error -= delta_x;
				y1 += iy;
			}
			// else do nothing

			error += delta_y;
			x1 += ix;
			DrawCircle(x1, y1, 1.0f, GREEN);
		}
	}
	else
	{
		while (y1 != y2)
		{
			// reduce error, while taking into account the corner case of error == 0
			if ((error > 0) || (!error && (iy > 0)))
			{
				error -= delta_y;
				x1 += ix;
			}
			// else do nothing

			error += delta_x;
			y1 += iy;

			DrawCircle(x1, y1, 1.0f, GREEN);
		}
	}
}

void GetTilesInCone(TileMap* tileMap, float playerAngle,
	float x, float y, int distance, Tile** outTiles)
{
	float playerFov = (90.0f / 2.0f) * (float)DEG2RAD;

	float x0 = cosf(playerAngle + playerFov);
	float y0 = sinf(playerAngle + playerFov);
	float x1 = cosf(playerAngle - playerFov);
	float y1 = sinf(playerAngle - playerFov);

	Vector2 pos0 = { x0, y0 };
	pos0 = Vector2Normalize(pos0);
	pos0 = Vector2Scale(pos0, distance);

	Vector2 pos1 = { x1, y1 };
	pos1 = Vector2Normalize(pos1);
	pos1 = Vector2Scale(pos1, distance);

	int xp0 = (int)x + (int)pos0.x;
	int yp0 = (int)y + (int)pos0.y;
	int xp1 = (int)x + (int)pos1.x;
	int yp1 = (int)y + (int)pos1.y;

	PlotLine(x, y, xp0, yp0);
	PlotLine(xp0, yp0, xp1, yp1);
	PlotLine(x, y, xp1, yp1);

	DrawLine(x, y, x + pos0.x, y + pos0.y, RED);
	DrawLine(x + pos0.x, y + pos0.y, x + pos1.x, y + pos1.y, RED);
	DrawLine(x, y, x + pos1.x, y + pos1.y, RED);
}



constexpr Vector2 CenterOfTile(TileMap* tileMap)
{
	float halfTileSize = tileMap->MapTileSize / 2.0f;
	return { halfTileSize, halfTileSize };
}

float Distance(float x0, float y0, float x1, float y1)
{
	float xL = x1 - x0;
	float yL = y1 - y0;
	return sqrtf(xL * xL + yL * yL);
}

int DistanceInTiles(int x0, int y0, int x1, int y1)
{
	int xL = x1 - x0;
	int yL = y1 - y0;
	return xL + yL;
}


TileType* GetTileInfo(TileMap* tileMap, uint32_t tileId)
{
	assert(tileId <
		tileMap->TileSet->TextureTileWidth* tileMap->TileSet->TextureTileHeight);
	return &tileMap->TileSet->TileTypes[tileId];
}
