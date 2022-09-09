#include "TileMap.h"

#include "Game.h"
#include "SMemory.h"
#include "SArray.h"
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
	
	float playerAngle = AngleFromDirection(p.LookDirection);
	Vector2i playerTilePos = p.TilePosition;
	Vector2i fowardPlayer = PlayerFoward(&p);
	GetTilesInCone(tileMap,
		playerAngle,
		60.0f * DEG2RAD,
		(float)(p.Position.x / 16.0f) + 0.5f,
		(float)(p.Position.y / 16.0f) + 0.5f,
		fowardPlayer.x,
		fowardPlayer.y,
		24.0f,
		nullptr);

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
}

bool IsInBounds(int x, int y, int width, int height)
{
	return x >= 0 && x < width && y >= 0 && y < height;
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
				&& Distance((float)x, (float)y, (float)xi, (float)yi) < radius)
				outTiles[i] = &tileMap->MapTiles[xi + yi * tileMap->MapWidth];
			else
				outTiles[i] = nullptr;
			++i;
		}
	}
}

void Raytrace2DInt(int x0, int y0, int x1, int y1, bool* values)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int x = x0;
	int y = y0;
	int n = 1 + dx + dy;
	int xInc = (x1 > x0) ? 1 : -1;
	int yInc = (y1 > y0) ? 1 : -1;
	int error = dx - dy;
	dx *= 2;
	dy *= 2;
	for (; n > 0; --n)
	{
		if (IsInBounds(x, y, 64, 64))
			values[x + y * 64] = true;
		//DrawCircle(x, y, 1, PURPLE);

		if (error > 0)
		{
			x += xInc;
			error -= dy;
		} else
		{
			y += yInc;
			error += dx;
		}
	}
}

void Raytrace2D(float x0, float y0, float x1, float y1, bool* values)
{
	float dx = fabsf(x1 - x0);
	float dy = fabsf(y1 - y0);
	int x = (int)floorf(x0);
	int y = (int)floorf(y0);
	int n = 1;
	int xInc;
	int yInc;
	float error;
	if (dx == 0)
	{
		xInc = 0;
		error = INFINITY;
	} else if (x1 > x0)
	{
		xInc = 1;
		n += (int)floorf(x1) - x;
		error = (floorf(x0) + 1 - x0) * dy;
	} else
	{
		xInc = -1;
		n += x - (int)floorf(x1);
		error = (x0 - floorf(x0)) * dy;
	}

	if (dy == 0)
	{
		yInc = 0;
		error -= INFINITY;
	} else if (y1 > y0)
	{
		yInc = 1;
		n += (int)floorf(y1) - y;
		error -= (floorf(y0) + 1 - y0) * dx;
	} else
	{
		yInc = -1;
		n += y - (int)floorf(y1);
		error -= (y0 - floorf(y0)) * dx;
	}

	for (; n > 0; --n)
	{
		if (IsInBounds(x, y, 64, 64))
			values[x + y * 64] = true;

		if (error > 0.0f)
		{
			y += yInc;
			error -= dx;
		} else
		{
			x += xInc;
			error += dy;
		}
	}
}

void Raytrace(float x0, float y0, float x1, float y1, bool* values)
{
	float dx = fabsf(x1 - x0);
	float dy = fabsf(y1 - y0);
	int x = (int)floorf(x0);
	int y = (int)floorf(y0);
	float dt_dx = (1.0f / dx);
	float dt_dy = (1.0f / dy);
	float t = 0.0f;
	int n = 1;
	int xInc;
	int yInc;
	float tNextVertical;
	float tNextHorizontal;
	if (dx == 0)
	{
		xInc = 0;
		tNextHorizontal = dt_dx;
	}
	else if (x1 > x0)
	{
		xInc = 1;
		n += (int)floorf(x1) - x;
		tNextHorizontal = (floorf(x0) + 1 - x0) * dt_dx;
	}
	else
	{
		xInc = -1;
		n += x - (int)floorf(x1);
		tNextHorizontal = (x0 - floorf(x0)) * dt_dx;
	}

	if (dy == 0)
	{
		yInc = 0;
		tNextVertical = dt_dy;
	} else if (y1 > y0)
	{
		yInc = 1;
		n += (int)floorf(y1) - y;
		tNextVertical = (floorf(y0) + 1 - y0) * dt_dy;
	} else
	{
		yInc = -1;
		n += y - (int)floorf(y1);
		tNextVertical = (y0 - floorf(y0)) * dt_dy;
	}

	for (; n > 0; --n)
	{
		if (IsInBounds(x, y, 64, 64))
			values[x + y * 64] = true;

		if (tNextVertical < tNextHorizontal)
		{
			y += yInc;
			t = tNextVertical;
			tNextVertical += dt_dy;
		}
		else
		{
			x += xInc;
			t = tNextHorizontal;
			tNextHorizontal += dt_dx;
		}
	}
}

using namespace Scal::Array;

global_var SArray CoordArray;

void FloodFill(TileMap* tileMap, int x, int y, bool* values)
{
	if (!CoordArray.Memory)
	{
		ArrayCreate(64 * 64, sizeof(Vector2i), &CoordArray);
	}

	ArrayClear(&CoordArray);

	Vector2i startCoord = { x, y };
	ArrayPush(&CoordArray, &startCoord);

	while (CoordArray.Length > 0)
	{
		Vector2i popCoord = {};
		ArrayPop(&CoordArray, &popCoord);
		int index = popCoord.x + popCoord.y * 64;
		if (IsInBounds(popCoord.x, popCoord.y, 64, 64) && !values[index])
		{
			values[index] = true;

			Vector2i coord = { popCoord.x + 1, popCoord.y };
			ArrayPush(&CoordArray, &coord);

			coord = { popCoord.x - 1, popCoord.y };
			ArrayPush(&CoordArray, &coord);

			coord = { popCoord.x, popCoord.y + 1 };
			ArrayPush(&CoordArray, &coord);

			coord = { popCoord.x, popCoord.y - 1 };
			ArrayPush(&CoordArray, &coord);
		}
	}
}

global_var bool Values[64 * 64];

void GetTilesInCone(TileMap* tileMap, float playerAngle,
	float playerFov, float x, float y,
	int fowardX, int fowardY, float distance,
	Tile** outTiles)
{
	playerFov /= 2;
	float x0 = cosf(playerAngle + playerFov);
	float y0 = sinf(playerAngle + playerFov);
	float x1 = cosf(playerAngle - playerFov);
	float y1 = sinf(playerAngle - playerFov);

	Vector2 pos0 = { x0, y0 };
	pos0 = Vector2Scale(pos0, distance);

	Vector2 pos1 = { x1, y1 };
	pos1 = Vector2Scale(pos1, distance);

	Vector3 posFoward = { fowardX * distance, fowardY * distance };

	float xx = x;
	float yy = y;
	float xp0 = xx + pos0.x;
	float yp0 = yy + pos0.y;
	float xp1 = xx + pos1.x;
	float yp1 = yy + pos1.y;
	float fp0 = xx + posFoward.x;
	float fp1 = yy + posFoward.y;
	
	Scal::Memory::Clear(Values, 64 * 64);

	Raytrace(xx, yy, xp0, yp0, Values);
	Raytrace(fp0, fp1, xp0, yp0, Values);
	Raytrace(fp0, fp1, xp1, yp1, Values);
	Raytrace(xx, yy, xp1, yp1, Values);

	int mapSize = tileMap->MapWidth * tileMap->MapHeight;
	FloodFill(tileMap, x + (fowardX * 2), y + (fowardY * 2), Values);
	for (int i = 0; i < mapSize; ++i)
	{
		if (Values[i])
		{
			tileMap->MapTiles[i].Fow = FOWLevel::FullVision;
		}
	}
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
	assert(tileId < tileMap->TileSet->TextureTileWidth * tileMap->TileSet->TextureTileHeight);
	return &tileMap->TileSet->TileTypes[tileId];
}
