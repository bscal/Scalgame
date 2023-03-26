#pragma once

#include "Core.h"
#include "Globals.h"
#include "Game.h"
#include "ChunkedTileMap.h"
#include "Lighting.h"
#include "Structures/StaticArray.h"

#include <mutex>

std::mutex LightArrayLock;

internal inline void 
UpdateLightColor(const UpdatingLight* light, int index, float distance)
{
	SASSERT(index >= 0);
	SASSERT(index < CULL_TOTAL_TILES);

	// https://www.desmos.com/calculator/nmnaud1hrw
	const float a = 0.0f;
	const float b = 0.1f;
	//distance = distance / light.Radius
	float attenuation = 1.0f / (1.0f + a * distance + b * distance * distance);

	constexpr float inverse = 1.0f / 255.0f;
	float x = (float)light->Color.r * inverse * attenuation;
	float y = (float)light->Color.g * inverse * attenuation;
	float z = (float)light->Color.b * inverse * attenuation;
	{
		std::scoped_lock lock(LightArrayLock);
		GetGame()->LightingRenderer.Tiles[index].x += x;
		GetGame()->LightingRenderer.Tiles[index].y += y;
		GetGame()->LightingRenderer.Tiles[index].z += z;
	}
}

internal inline void 
ProcessLight(const UpdatingLight* light, uint8_t octant, int x, Slope top, Slope bottom, bool* checkedTiles)
{
	ChunkedTileMap* tilemap = &GetGame()->World.ChunkedTileMap;
	Vector2i origin = Vector2i::FromVec2(light->Pos);
	int rangeLimit = (int)light->Radius;
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
			Vector2i txty = origin;
			txty.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
			txty.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];

			float distance;
			bool inRange = TileInsideCullRect(txty)
				&& CTileMap::IsTileInBounds(tilemap, txty)
				&& ((distance = Vector2i{ x, y }.Distance(TILEMAP_ORIGIN)) <= rangeLimit);
			if (inRange)
			{
				TileCoord coord = WorldTileToCullTile(txty);
				int index = coord.x + coord.y * CULL_WIDTH_TILES;
				if (!checkedTiles[index])
				{
					checkedTiles[index] = true;
					UpdateLightColor(light, index, distance);
				}
			}

			// NOTE: use the next line instead if you want the algorithm to be symmetrical
			// if(inRange && (y != topY || top.Y*x >= top.X*y) && (y != bottomY || bottom.Y*x <= bottom.X*y)) SetVisible(tx, ty);

			bool isOpaque = !inRange || CTileMap::BlocksLight(tilemap, txty);
			if (x != rangeLimit)
			{
				if (isOpaque)
				{
					if (wasOpaque == 0) // if we found a transition from clear to opaque, this sector is done in this column, so
					{                  // adjust the bottom vector upwards and continue processing it in the next column.
						Slope newBottom = { y * 2 + 1, x * 2 - 1 }; // (x*2-1, y*2+1) is a vector to the top-left of the opaque tile
						if (!inRange || y == bottomY) { bottom = newBottom; break; } // don't recurse unless we have to
						else ProcessLight(light, octant, x + 1, top, newBottom, checkedTiles);
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

void ThreadedLightUpdate(const UpdatingLight* light)
{
	bool checkedTiles[CULL_TOTAL_TILES] = { 0 };
	for (uint8_t octant = 0; octant < 8; ++octant)
	{
		ProcessLight(light, octant, 1, { 1, 1 }, { 0 , 1 }, checkedTiles);
	}
}
