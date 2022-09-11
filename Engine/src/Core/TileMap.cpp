#include "TileMap.h"

#include "Game.h"
#include "SMemory.h"
#include "SArray.h"
#include "Structures/DirectAccessTable.h"
#include "raymath.h"

#include <cassert>

// TODO temp
global_var Scal::ResizableArray TilesInLOS;

bool InitializeTileMap(TileSet* tileSet,
	uint32_t width, uint32_t height,
	uint16_t tileSize, TileMap* outTileMap)
{
	outTileMap->TileSet = tileSet;
	outTileMap->MapSize = width * height;
	outTileMap->MapWidth = width;
	outTileMap->MapHeight = height;
	outTileMap->MapTileSize = tileSize;
	outTileMap->MapHalfTileSize = tileSize / 2;
	outTileMap->MapTiles =
		(Tile*)Scal::Memory::Alloc(outTileMap->MapSize * sizeof(Tile));

	Scal::ArrayCreate(width * height, sizeof(Vector2i), &TilesInLOS);

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
	outTileSet->TileDataArray =
		(TileData*)Scal::Memory::Alloc(totalTiles * sizeof(TileData));

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int i = x + y * width;
			float xTexturePos = (float)x * (float)tileSizeWidth;
			float yTexturePos = (float)y * (float)tileSizeHeight;
			TileData tileType = {};
			tileType.TextureCoord = { xTexturePos, yTexturePos };
			tileType.MovementCost = 1;
			tileType.TileVisibilty = TileVisibilty::Empty;
			tileType.TileType = TileType::Floor;

			if (i == 4)
			{
				tileType.TileType = TileType::Solid;
			}

			outTileSet->TileDataArray[i] = tileType;
		}
	}

	TraceLog(LOG_INFO, "Loaded TileSet %d with %d tiles",
		tileTexture->id, totalTiles);

	return true;
}

Tile CreateTile(TileMap* tileMap, uint32_t tileId)
{
	Tile tile = {};
	tile.TexturePosition = tileMap->TileSet->TileDataArray[tileId].TextureCoord;
	tile.TileId = tileId;
	tile.Fow = FOWLevel::NoVision;
	return tile;
}

void LoadTileMap(TileMap* tileMap)
{
	SetRandomSeed(0);
	for (uint32_t y = 0; y < tileMap->MapHeight; ++y)
	{
		for (uint32_t x = 0; x < tileMap->MapWidth; ++x)
		{
			uint32_t index = x + y * tileMap->MapWidth;
			uint32_t tileId = GetRandomValue(1, 2);
			tileMap->MapTiles[index] = CreateTile(tileMap, tileId);
		}
	}
}

void UnloadTileMap(TileMap* tileMap)
{
	MemFree(tileMap->MapTiles);
	MemFree(tileMap);
}

// TODO should be moved
global_var bool DisableFow = false;

void RenderTileMap(Game* game, TileMap* tileMap)
{
	float fowValues[5] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
	float fowAlphas[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 colorFullVision = ColorNormalize(WHITE);
	Vector4 colorNoVision = ColorNormalize(BLACK);

	//Scal::ArrayClear(&TilesInLOS);

	TileSet* tileSet = tileMap->TileSet;

	Player p = game->Player;
	float playerAngle = AngleFromDirection(p.LookDirection);
	float playerTileX = (float)p.TilePosition.x + 0.5f;
	float playerTileY = (float)p.TilePosition.y + 0.5f;
	Vector2i fowardPlayer = PlayerFoward(&p);

	GetSurronding(tileMap, playerTileX, playerTileY,
		8, 2.0f);

	GetTilesInCone(tileMap,
		playerAngle,
		145.0f / 360.0f,
		32,
		playerTileX,
		playerTileY,
		24.0f);

	//for (int i = 0; i < TilesInLOS.Length; ++i)
	//{
	//	Vector2i coord = *(Vector2i*)Scal::ArrayPeekAt(&TilesInLOS, i);
	//	if (coord.x > -1)
	//	{
	//		Tile* tile = GetTile(tileMap, coord.x, coord.y);
	//		if (tile->Fow == FOWLevel::FullVision)
	//		{
	//			GetSurroundingTilesRadiusCallback(tileMap, coord.x, coord.y, 2.0f,
	//			[](TileMap* tileMap, int x, int y)
	//			{
	//				Tile* tile = GetTile(tileMap, x, y);
	//				if (tile->Fow != FOWLevel::FullVision)
	//				{
	//					tile->Fow = FOWLevel::PeripheralVision;
	//				}
	//			});
	//		}
	//	}
	//}

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

			Rectangle textureRect = {
				tile.TexturePosition.x,
				tile.TexturePosition.y,
				(float)tileSet->TextureTileWidth,
				(float)tileSet->TextureTileHeight
			};
			DrawTextureRec(
				tileSet->TileTexture,
				textureRect,
				{ xPos, yPos }, 
				color);

			tileMap->MapTiles[index].Fow = FOWLevel::NoVision;
		}
	}
}

bool IsInBounds(int tileX, int tileY, int width, int height)
{
	return tileX >= 0 && tileX < width&& tileY >= 0 && tileY < height;
}

Tile* GetTile(TileMap* tileMap, int tileX, int tileY)
{
	assert(tileX >= 0 && tileX < tileMap->MapWidth);
	assert(tileY >= 0 && tileY < tileMap->MapHeight);

	int index = tileX + tileY * tileMap->MapWidth;
	return &tileMap->MapTiles[index];
}

void SetTile(TileMap* tileMap, int tileX, int tileY, Tile* srcTile)
{
	assert(tileX >= 0 && tileX < tileMap->MapWidth);
	assert(tileY >= 0 && tileY < tileMap->MapHeight);

	int index = tileX + tileY * tileMap->MapWidth;
	tileMap->MapTiles[index] = *srcTile;
}

TileData* GetTileData(TileMap* tileMap, uint32_t tileId)
{
	return &tileMap->TileSet->TileDataArray[tileId];
}

void GetSurroundingTilesBox(TileMap* tileMap,
	int x, int y, int boxWidth, int boxHeight,
	Scal::ResizableArray* outTiles)
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
			{
				Vector2i coord = { xi, yi };
				Scal::ArrayPush(outTiles, &coord);
			}
			++i;
		}
	}
}


void GetSurroundingTilesRadius(TileMap* tileMap,
	float x, float y, float radius,
	Scal::ResizableArray* outTiles)
{
	int startX = (int)x - (int)radius;
	int startY = (int)y - (int)radius;
	int endX = (int)x + (int)radius;
	int endY = (int)y + (int)radius;
	int i = 0;
	for (int yi = startY; yi <= endY; ++yi)
	{
		for (int xi = startX; xi <= endX; ++xi)
		{
			if (IsInBounds(xi, yi, tileMap->MapWidth, tileMap->MapHeight)
				&& Distance(x, y, (float)xi, (float)yi) < radius)
			{
				Vector2i coord = { xi, yi };
				Scal::ArrayPush(outTiles, &coord);
			}
			++i;
		}
	}
}

void GetSurroundingTilesRadiusCallback(TileMap* tileMap,
	float x, float y, float radius,
	void (OnVisit)(TileMap* tileMap, int x, int y))
{
	int startX = (int)x - (int)radius;
	int startY = (int)y - (int)radius;
	int endX = (int)x + (int)radius;
	int endY = (int)y + (int)radius;
	int i = 0;
	for (int yi = startY; yi <= endY; ++yi)
	{
		for (int xi = startX; xi <= endX; ++xi)
		{
			if (IsInBounds(xi, yi, tileMap->MapWidth, tileMap->MapHeight)
				&& Distance(x, y, (float)xi, (float)yi) < radius)
			{
				OnVisit(tileMap, xi, yi);
			}
			++i;
		}
	}
}

void Raytrace2DInt(TileMap* tileMap, int x0, int y0, int x1, int y1,
	bool (OnVisit)(TileMap* tileMap, int x, int y))
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
		if (OnVisit(tileMap, x, y))
			return;

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
	} else if (x1 > x0)
	{
		xInc = 1;
		n += (int)floorf(x1) - x;
		tNextHorizontal = (floorf(x0) + 1 - x0) * dt_dx;
	} else
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
		} else
		{
			x += xInc;
			t = tNextHorizontal;
			tNextHorizontal += dt_dx;
		}
	}
}



//  TODO Needs some work, Needs some way of storing visited values
// more reliably
void FloodFill(TileMap* tileMap, int x, int y, bool* values)
{
	// TODO THIS IS BROKEN
	assert(false);
	//Scal::ArrayCreate(tileMap->MapSize, sizeof(Vector2i), 0);
	//Scal::DATCreate(tileMap->MapSize, sizeof(Vector2i), 0);
	//Scal::ArrayClear(&CoordArray);
	//DASClear(&VisitedTable);

	Vector2i startCoord = { x, y };
	//ArrayPush(&CoordArray, &startCoord);

	//while (CoordArray.Length > 0)
	{
		Vector2i popCoord = {};
		//ArrayPop(&CoordArray, &popCoord);

		int index = popCoord.x + popCoord.y * 64;
		if (IsInBounds(popCoord.x, popCoord.y, 64, 64) && !values[index])
		{
			Vector2i coord = { popCoord.x + 1, popCoord.y };
			//ArrayPush(&CoordArray, &coord);

			coord = { popCoord.x - 1, popCoord.y };
			//ArrayPush(&CoordArray, &coord);

			coord = { popCoord.x, popCoord.y + 1 };
			//ArrayPush(&CoordArray, &coord);

			coord = { popCoord.x, popCoord.y - 1 };
			//ArrayPush(&CoordArray, &coord);
		}
	}
}

internal bool OnVisitSurroundingTile(TileMap* tileMap, int x, int y)
{
	if (IsInBounds(x, y, tileMap->MapWidth, tileMap->MapHeight))
	{
		int index = x + y * tileMap->MapWidth;
		Tile tile = tileMap->MapTiles[index];
		tile.Fow = FOWLevel::FullVision;
		TileData* tileData = GetTileData(tileMap, tile.TileId);
		tileMap->MapTiles[index] = tile;
		return tileData->TileType == TileType::Solid;
	}
	return false;
}

void GetSurronding(TileMap* tileMap, float x, float y,
	int resolution, float distance)
{
	constexpr float startAngle = 0.0f * TAO;
	constexpr float endAngle = 1.0f * TAO;
	for (int i = 0; i < resolution; ++i)
	{
		float t = Lerp(startAngle, endAngle, (float)i / (float)resolution - 1.0f);
		float angleX = cosf(t);
		float angleY = sinf(t);
		float vX = x + angleX * distance;
		float vY = y + angleY * distance;
		Raytrace2DInt(tileMap, x, y, vX, vY, OnVisitSurroundingTile);
	}
}

internal bool OnVisitTile2(TileMap* tileMap, int x, int y)
{
	if (IsInBounds(x, y, tileMap->MapWidth, tileMap->MapHeight))
	{
		int index = x + y * tileMap->MapWidth;
		tileMap->MapTiles[index].Fow = FOWLevel::SemiVision;
		uint32_t tileId = tileMap->MapTiles[index].TileId;
		TileData* data = GetTileData(tileMap, tileId);
		return data->TileType == TileType::Solid;
	}
	return false;
}

internal bool OnVisitTile(TileMap* tileMap, int x, int y)
{
	if (IsInBounds(x, y, tileMap->MapWidth, tileMap->MapHeight))
	{
		int index = x + y * tileMap->MapWidth;
		tileMap->MapTiles[index].Fow = FOWLevel::FullVision;
		uint32_t tileId = tileMap->MapTiles[index].TileId;
		TileData* data = GetTileData(tileMap, tileId);
		return data->TileType == TileType::Solid;
	}
	return false;
}

void GetTilesInCone(TileMap* tileMap,
	float playerAngle, float playerFov, int resolution,
	float x, float y, float distance)
{	
	//float o = ((playerFov + 30.0f / 360.0f) * TAO) / 2.0f;
	playerFov /= 2;
	float startAngle = 0.0f * TAO;
	float endAngle = playerFov * TAO;
	//float endOther = o;

	//for (int i = 0; i < 8; ++i)
	//{
	//	float t = Lerp(endAngle, endOther, (float)i / (float)8);
	//	float x0 = cosf(playerAngle + t);
	//	float y0 = sinf(playerAngle + t);
	//	float x1 = cosf(playerAngle - t);
	//	float y1 = sinf(playerAngle - t);

	//	float xPos0 = x + x0 * distance;
	//	float yPos0 = y + y0 * distance;
	//	float xPos1 = x + x1 * distance;
	//	float yPos1 = y + y1 * distance;

	//	Raytrace2DInt(tileMap, x, y, xPos0, yPos0, OnVisitTile2);
	//	Raytrace2DInt(tileMap, x, y, xPos1, yPos1, OnVisitTile2);
	//}

	for (int i = 0; i < resolution; ++i)
	{
		float t = Lerp(startAngle, endAngle, (float)i / (float)resolution);
		float x0 = cosf(playerAngle + t);
		float y0 = sinf(playerAngle + t);
		float x1 = cosf(playerAngle - t);
		float y1 = sinf(playerAngle - t);

		float xPos0 = x + x0 * distance;
		float yPos0 = y + y0 * distance;
		float xPos1 = x + x1 * distance;
		float yPos1 = y + y1 * distance;

		Raytrace2DInt(tileMap, x, y, xPos0, yPos0, OnVisitTile);
		Raytrace2DInt(tileMap, x, y, xPos1, yPos1, OnVisitTile);
	}
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
