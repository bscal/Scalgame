#include "ThreadedLights.h"

#include "Game.h"
#include "ChunkedTileMap.h"
#include "Lighting.h"
#include "Structures/BitArray.h"

struct LightUpdater
{
	BitArray<MAX_TILE_COUNT> CheckedTiles;
	UpdatingLight* Light;
	Vector3* ColorsArray;
	ChunkedTileMap* Tilemap;
	Vector2i Origin;
	uint32_t Width;

	void ProcessOctant(uint8_t octant, int x, Slope top, Slope bottom);
	void SetColor(uint32_t index, float distance);
};

void ProcessLightUpdater(UpdatingLight* light, uint32_t screenLightsWidth, Vector3* colorsArray, ChunkedTileMap* tilemap)
{
	LightUpdater updater = LightUpdater();
	updater.Light = light;
	updater.ColorsArray = colorsArray;
	updater.Tilemap = tilemap;
	updater.Origin = light->Pos;
	updater.Width = screenLightsWidth;

	TileCoord coord = WorldTileToCullTile(updater.Origin);
	uint32_t idx = coord.x + coord.y * updater.Width;
	updater.SetColor(idx, 0.0f);
	for (uint8_t octant = 0; octant < 8; ++octant)
	{
		updater.ProcessOctant(octant, 1, { 1, 1 }, { 0, 1 });
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
				&& ((distance = Vector2i{ x, y }.Distance({})) <= rangeLimit);
			if (inRange)
			{
				TileCoord coord = WorldTileToCullTile(txty);
				int index = coord.x + coord.y * GetGameApp()->View.ResolutionInTiles.x;
				if (!CheckedTiles.Get(index))
				{
					CheckedTiles.Set(index);
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

_FORCE_INLINE_ internal
Color Clamp0255(Color c0, Color c1, float attenuation)
{
	Color res;

	float r = ((float)c0.r + ((float)c1.r * attenuation));
	float g = ((float)c0.g + ((float)c1.g * attenuation));
	float b = ((float)c0.b + ((float)c1.b * attenuation));
	float a = ((float)c0.a + ((float)c1.a * attenuation));

	res.r = (r > 255.0f) ? UINT8_MAX : (uint8_t)r;
	res.g = (g > 255.0f) ? UINT8_MAX : (uint8_t)g;
	res.b = (b > 255.0f) ? UINT8_MAX : (uint8_t)b;
	res.a = (a > 255.0f) ? UINT8_MAX : (uint8_t)a;

	return res;
}

struct ThreadedUpdater
{
	Light* Light;
	Color* ColorsArray;
	ChunkedTileMap* Tilemap;
	uint32_t Width;
	BitArray<MAX_TILE_COUNT> CheckedTiles;
};

internal void
SetColor(Color* colorsArray, uint32_t index, Color color, float distance)
{
	// https://www.desmos.com/calculator/nmnaud1hrw
	constexpr float a = 0.0f;
	constexpr float b = 0.1f;
	//distance = distance / light.Radius
	float attenuation = 1.0f / (1.0f + a * distance + b * distance * distance);

	constexpr float inverse = 1.0f / 255.0f;

	colorsArray[index] = Clamp0255(colorsArray[index], color, attenuation);
}

internal void
ProcessOctants(ThreadedUpdater* updater, uint8_t octant, int x, Slope top, Slope bottom)
{
	int rangeLimit = (int)updater->Light->Radius;
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
			Vector2i txty = updater->Light->Pos;
			txty.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
			txty.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];

			float distance;
			bool inRange = TileInsideCullRect(txty)
				&& CTileMap::IsTileInBounds(updater->Tilemap, txty)
				&& ((distance = Vector2i{ x, y }.Distance({})) <= rangeLimit);
			if (inRange)
			{
				TileCoord coord = WorldTileToCullTile(txty);
				int index = coord.x + coord.y * GetGameApp()->View.ResolutionInTiles.x;
				if (!updater->CheckedTiles.Get(index))
				{
					updater->CheckedTiles.Set(index);
					SetColor(updater->ColorsArray, index, updater->Light->Color, distance);
				}
			}

			// NOTE: use the next line instead if you want the algorithm to be symmetrical
			// if(inRange && (y != topY || top.Y*x >= top.X*y) && (y != bottomY || bottom.Y*x <= bottom.X*y)) SetVisible(tx, ty);

			bool isOpaque = !inRange || CTileMap::BlocksLight(updater->Tilemap, txty);
			if (x != rangeLimit)
			{
				if (isOpaque)
				{
					if (wasOpaque == 0) // if we found a transition from clear to opaque, this sector is done in this column, so
					{                  // adjust the bottom vector upwards and continue processing it in the next column.
						Slope newBottom = { y * 2 + 1, x * 2 - 1 }; // (x*2-1, y*2+1) is a vector to the top-left of the opaque tile
						if (!inRange || y == bottomY) { bottom = newBottom; break; } // don't recurse unless we have to
						else ProcessOctants(updater, octant, x + 1, top, newBottom);
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

void
UpdateStaticLight(StaticLight* light, Color* threadColorsArray, size_t width)
{
	SASSERT(light);
	SASSERT(threadColorsArray);
	SASSERT(width > 0);
	Vector2i cullPos = WorldTileToCullTile(light->Pos);
	for (int i = 0; i < 9; ++i)
	{
		Vector2i pos = cullPos + LavaLightOffsets[i];
		if (TileInsideCullRect(light->Pos + LavaLightOffsets[i]))
		{
			size_t idx = (size_t)pos.x + (size_t)pos.y * width;
			SASSERT(idx < GetGameApp()->View.TotalTilesOnScreen);
			threadColorsArray[idx] = Clamp0255(threadColorsArray[idx], light->Color, LavaLightWeights[i]);
		}
	}
}

void 
ThreadedLightUpdate(Light* light, Color* threadColorsArray, ChunkedTileMap* tilemap, uint32_t lightsScreenWidth)
{
	ThreadedUpdater updater = {};
	updater.Light = light;
	updater.ColorsArray = threadColorsArray;
	updater.Tilemap = tilemap;
	updater.Width = lightsScreenWidth;

	switch (light->LightType)
	{
		case (LightType::Updating): // Updating light
		{
			SASSERT(light->UpdateFunc);
			light->UpdateFunc(light, GetGame(), GetDeltaTime());

			TileCoord coord = WorldTileToCullTile(light->Pos);
			uint32_t idx = coord.x + coord.y * updater.Width;
			SetColor(updater.ColorsArray, idx, light->Color, 0.0f);
			for (uint8_t octant = 0; octant < 8; ++octant)
			{
				ProcessOctants(&updater, octant, 1, { 1, 1 }, { 0, 1 });
			}
		} break;

		case (LightType::Static): // Static light
		{
			UpdateStaticLight((StaticLight*)updater.Light, updater.ColorsArray, updater.Width);
		} break;

		default:
			break;
	}
}