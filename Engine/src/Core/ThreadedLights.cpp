#include "ThreadedLights.h"

#include "Game.h"
#include "ChunkedTileMap.h"
#include "Lighting.h"

#include <bitset>

struct LightUpdater
{
	std::bitset<CULL_TOTAL_TILES> CheckedTiles;
	UpdatingLight* Light;
	Vector3* ColorsArray;
	ChunkedTileMap* Tilemap;
	Vector2i Origin;
	size_t Width;

	void ProcessOctant(uint8_t octant, int x, Slope top, Slope bottom);
	void SetColor(uint32_t index, float distance);
};

void ThreadedLights::AllocateArrays(size_t elementCount)
{
	size_t size = elementCount * sizeof(Vector3);
	for (int i = 0; i < 4; ++i)
	{
		ColorArrayPtrs[i] = (Vector3*)SMemTempAlloc(size);
		SMemClear(ColorArrayPtrs[i], size);
	}
}

void ThreadedLights::UpdateLightColorArray(Vector4* finalColors) const
{
	for (int i = 0; i < CULL_TOTAL_TILES; ++i)
	{
		finalColors[i].x += ColorArrayPtrs[0][i].x;
		finalColors[i].y += ColorArrayPtrs[0][i].y;
		finalColors[i].z += ColorArrayPtrs[0][i].z;

		finalColors[i].x += ColorArrayPtrs[1][i].x;
		finalColors[i].y += ColorArrayPtrs[1][i].y;
		finalColors[i].z += ColorArrayPtrs[1][i].z;

		finalColors[i].x += ColorArrayPtrs[2][i].x;
		finalColors[i].y += ColorArrayPtrs[2][i].y;
		finalColors[i].z += ColorArrayPtrs[2][i].z;

		finalColors[i].x += ColorArrayPtrs[3][i].x;
		finalColors[i].y += ColorArrayPtrs[3][i].y;
		finalColors[i].z += ColorArrayPtrs[3][i].z;
	}
}

void ProcessLightUpdater(UpdatingLight* light, size_t width, Vector3* colorsArray, ChunkedTileMap* tilemap)
{
	LightUpdater updater = LightUpdater();
	updater.Light = light;
	updater.ColorsArray = colorsArray;
	updater.Tilemap = tilemap;
	updater.Origin = Vector2i::FromVec2(light->Pos);
	updater.Width = width;

	TileCoord coord = WorldTileToCullTile(updater.Origin);
	uint32_t idx = coord.x + coord.y * updater.Width;
	updater.SetColor(idx, 0.0f);
	for (uint8_t octant = 0; octant < 8; ++octant)
	{
		updater.ProcessOctant(octant, 1, { 1, 1 }, { 0, 1 });
	}
}

void LightUpdater::ProcessOctant(uint8_t octant, int x, Slope top, Slope bottom)
{
	int rangeLimit = (int)Light->Radius;
	for (; x <= rangeLimit; ++x) // rangeLimit < 0 || x <= rangeLimit
	{
		// compute the Y coordinates where the top vector leaves the column (on the right) and where the bottom vector
		// enters the column (on the left). this equals (x+0.5)*top+0.5 and (x-0.5)*bottom+0.5 respectively, which can
		// be computed like (x+0.5)*top+0.5 = (2(x+0.5)*top+1)/2 = ((2x+1)*top+1)/2 to avoid floating point math
		int topY = top.x == 1 ? x : ((x * 2 + 1) * top.y + top.x - 1) / (top.x * 2); // the rounding is a bit tricky, though
		int bottomY = bottom.y == 0 ? 0 : ((x * 2 - 1) * bottom.y + bottom.x) / (bottom.x * 2);

		int wasOpaque = -1; // 0:false, 1:true, -1:not applicable
		for (int y = topY; y >= bottomY; --y)
		{
			Vector2i txty = Origin;
			txty.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
			txty.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];

			float distance;
			bool inRange = TileInsideCullRect(txty)
				&& CTileMap::IsTileInBounds(Tilemap, txty)
				&& ((distance = Vector2i{ x, y }.Distance(TILEMAP_ORIGIN)) <= rangeLimit);
			if (inRange)
			{
				TileCoord coord = WorldTileToCullTile(txty);
				int index = coord.x + coord.y * CULL_WIDTH_TILES;
				if (!CheckedTiles.test(index))
				{
					CheckedTiles.set(index);
					SetColor(index, distance);
				}
			}

			// NOTE: use the next line instead if you want the algorithm to be symmetrical
			// if(inRange && (y != topY || top.Y*x >= top.X*y) && (y != bottomY || bottom.Y*x <= bottom.X*y)) SetVisible(tx, ty);

			bool isOpaque = !inRange || CTileMap::BlocksLight(Tilemap, txty);
			if (x != rangeLimit)
			{
				if (isOpaque)
				{
					if (wasOpaque == 0) // if we found a transition from clear to opaque, this sector is done in this column, so
					{                  // adjust the bottom vector upwards and continue processing it in the next column.
						Slope newBottom = { y * 2 + 1, x * 2 - 1 }; // (x*2-1, y*2+1) is a vector to the top-left of the opaque tile
						if (!inRange || y == bottomY) { bottom = newBottom; break; } // don't recurse unless we have to
						else ProcessOctant(octant, x + 1, top, newBottom);
					}
					wasOpaque = 1;
				}
				else // adjust top vector downwards and continue if we found a transition from opaque to clear
				{    // (x*2+1, y*2+1) is the top-right corner of the clear tile (i.e. the bottom-right of the opaque tile)
					if (wasOpaque > 0) top = { y * 2 + 1, x * 2 + 1 };
					wasOpaque = 0;
				}
			}
		}
		if (wasOpaque != 0) break; // if the column ended in a clear tile, continue processing the current sector
	}
}

void LightUpdater::SetColor(uint32_t index, float distance)
{
	// https://www.desmos.com/calculator/nmnaud1hrw
	constexpr float a = 0.0f;
	constexpr float b = 0.1f;
	//distance = distance / light.Radius
	float attenuation = 1.0f / (1.0f + a * distance + b * distance * distance);

	constexpr float inverse = 1.0f / 255.0f;
	ColorsArray[index].x += (float)Light->Color.r * inverse * attenuation;
	ColorsArray[index].y += (float)Light->Color.g * inverse * attenuation;
	ColorsArray[index].z += (float)Light->Color.b * inverse * attenuation;
}