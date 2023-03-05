#include "Lighting.h"

#include "Game.h"
#include "LightMap.h"

// Most of these implementations for lighting I have gotten
// from http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html

LightingState State;

void LightsInitialized(GameApplication* gameApp)
{
	State.Lights.Reserve(16);
	State.UpdatingLights.Reserve(32);
}

void LightsAdd(const Light& light)
{
	State.Lights.Push(&light);
}

void LightsAddUpdating(const UpdatingLight& light)
{
	State.UpdatingLights.Push(&light);
}

size_t GetNumOfLights()
{
	return State.Lights.Count + State.UpdatingLights.Count;
}

void UpdatingLight::Update(Game* game)
{
	LastUpdate -= GetDeltaTime();
	if (LastUpdate < 0)
	{
		LastUpdate = UpdatingLight::UPDATE_RATE;
		float rand = SRandNextFloat(GetGlobalRandom());
		if (rand < 0.35f)
		{
			uint64_t index = SRandNextRange(GetGlobalRandom(), 0, 3);
			Color = Colors[index];
		}
		Radius = SRandNextFloatRange(GetGlobalRandom(), MinIntensity, MaxIntensity);
	}
}

struct Slope
{
	int y;
	int x;

	inline bool Greater(int y, int x) { return this->y * x > this->x * y; }; // this > y/x
	inline bool GreaterOrEqual(int y, int x) { return this->y * x >= this->x * y; } // this >= y/x
	inline bool Less(int y, int x) { return this->y * x < this->x* y; } // this < y/x
};


internal inline bool
IsTileValid(Vector2i pos);

internal void ComputeOctant(ChunkedTileMap* tilemap, uint8_t octant,
	Vector2i origin, int rangeLimit, int x, Slope top, Slope bottom);


internal void ComputeLightShadowCast(ChunkedTileMap* tilemap,
	const Light& light, uint8_t octant, Vector2i origin,
	int rangeLimit, int x, Slope top, Slope bottom);

void LightsUpdate(Game* game)
{
	PROFILE_BEGIN();
	Vector2i originTile = GetClientPlayer()->Transform.TilePos;
	ChunkedTileMap* tilemap = &game->World.ChunkedTileMap;

	if (CTileMap::IsTileInBounds(tilemap, originTile))
	{
		CTileMap::SetVisible(tilemap, originTile);

		for (uint8_t octant = 0; octant < 8; ++octant)
		{
			ComputeOctant(tilemap, octant, originTile, 16, 1, { 1, 1 }, { 0, 1 });
		}
	}

	for (int i = 0; i < State.Lights.Count; ++i)
	{
		const Light& light = State.Lights[i];

		// FIXME: dont render lights off screen, should add buffer
		Vector2i lightTilePos = Vec2fToVec2i(light.Pos);
		if (!LightMapInView(&game->LightMap, lightTilePos)) continue;

		SMemSet(State.CheckedTiles.data(), 0, State.Size);

		LightsUpdateTileColorTile({ lightTilePos.x, lightTilePos.y }, 0.0f, light);
		for (uint8_t octant = 0; octant < 8; ++octant)
		{
			ComputeLightShadowCast(tilemap, light, octant, lightTilePos,
				(int)light.Radius, 1, { 1, 1 }, { 0, 1 });
		}
	}

	for (int i = 0; i < State.UpdatingLights.Count; ++i)
	{
		Vector2i lightTilePos = Vec2fToVec2i(State.UpdatingLights[i].Pos);
		if (!LightMapInView(&game->LightMap, lightTilePos)) continue;

		SMemSet(State.CheckedTiles.data(), 0, State.Size);

		State.UpdatingLights[i].Update(game);
		const UpdatingLight& light = State.UpdatingLights[i];
		LightsUpdateTileColorTile({ lightTilePos.x, lightTilePos.y }, 0.0f, light);
		for (uint8_t octant = 0; octant < 8; ++octant)
		{
			ComputeLightShadowCast(tilemap, light, octant, lightTilePos,
				(int)light.Radius, 1, { 1, 1 }, { 0, 1 });
		}
	}
	PROFILE_END();
}

internal bool BlocksLight(ChunkedTileMap* tilemap, int x, int y, Vector2i origin, uint8_t octant)
{
	Vector2i newPos = origin;
	switch (octant)
	{
		case 0: newPos.x += x; newPos.y -= y; break;
		case 1: newPos.x += y; newPos.y -= x; break;
		case 2: newPos.x -= y; newPos.y -= x; break;
		case 3: newPos.x -= x; newPos.y -= y; break;
		case 4: newPos.x -= x; newPos.y += y; break;
		case 5: newPos.x -= y; newPos.y += x; break;
		case 6: newPos.x += y; newPos.y += x; break;
		case 7: newPos.x += x; newPos.y += y; break;
	}
	return CTileMap::BlocksLight(tilemap, newPos);
}

internal void SetVisible(ChunkedTileMap* tilemap, int x, int y, Vector2i origin, uint8_t octant)
{
	Vector2i newPos = origin;
	switch (octant)
	{
		case 0: newPos.x += x; newPos.y -= y; break;
		case 1: newPos.x += y; newPos.y -= x; break;
		case 2: newPos.x -= y; newPos.y -= x; break;
		case 3: newPos.x -= x; newPos.y -= y; break;
		case 4: newPos.x -= x; newPos.y += y; break;
		case 5: newPos.x -= y; newPos.y += x; break;
		case 6: newPos.x += y; newPos.y += x; break;
		case 7: newPos.x += x; newPos.y += y; break;
	}
	CTileMap::SetVisible(tilemap, newPos);
}

internal void ComputeOctant(ChunkedTileMap* tilemap, uint8_t octant,
	Vector2i origin, int rangeLimit, int x, Slope top, Slope bottom)
{
	// throughout this function there are references to various parts of tiles. a tile's coordinates refer to its
		// center, and the following diagram shows the parts of the tile and the vectors from the origin that pass through
		// those parts. given a part of a tile with vector u, a vector v passes above it if v > u and below it if v < u
		//    g         center:        y / x
		// a------b   a top left:      (y*2+1) / (x*2-1)   i inner top left:      (y*4+1) / (x*4-1)
		// |  /\  |   b top right:     (y*2+1) / (x*2+1)   j inner top right:     (y*4+1) / (x*4+1)
		// |i/__\j|   c bottom left:   (y*2-1) / (x*2-1)   k inner bottom left:   (y*4-1) / (x*4-1)
		//e|/|  |\|f  d bottom right:  (y*2-1) / (x*2+1)   m inner bottom right:  (y*4-1) / (x*4+1)
		// |\|__|/|   e middle left:   (y*2) / (x*2-1)
		// |k\  /m|   f middle right:  (y*2) / (x*2+1)     a-d are the corners of the tile
		// |  \/  |   g top center:    (y*2+1) / (x*2)     e-h are the corners of the inner (wall) diamond
		// c------d   h bottom center: (y*2-1) / (x*2)     i-m are the corners of the inner square (1/2 tile width)
		//    h
	for (; x <= (int)rangeLimit; ++x) // (x <= (uint)rangeLimit) == (rangeLimit < 0 || x <= rangeLimit)
	{
		// compute the Y coordinates of the top and bottom of the sector. we maintain that top > bottom
		int topY;
		if (top.x == 1) // if top == ?/1 then it must be 1/1 because 0/1 < top <= 1/1. this is special-cased because top
		{              // starts at 1/1 and remains 1/1 as long as it doesn't hit anything, so it's a common case
			topY = x;
		}
		else // top < 1
		{
			// get the tile that the top vector enters from the left. since our coordinates refer to the center of the
			// tile, this is (x-0.5)*top+0.5, which can be computed as (x-0.5)*top+0.5 = (2(x+0.5)*top+1)/2 =
			// ((2x+1)*top+1)/2. since top == a/b, this is ((2x+1)*a+b)/2b. if it enters a tile at one of the left
			// corners, it will round up, so it'll enter from the bottom-left and never the top-left
			topY = ((x * 2 - 1) * top.y + top.x) / (top.x * 2); // the Y coordinate of the tile entered from the left
			// now it's possible that the vector passes from the left side of the tile up into the tile above before
			// exiting from the right side of this column. so we may need to increment topY
			if (BlocksLight(tilemap, x, topY, origin, octant)) // if the tile blocks light (i.e. is a wall)...
			{
				// if the tile entered from the left blocks light, whether it passes into the tile above depends on the shape
				// of the wall tile as well as the angle of the vector. if the tile has does not have a beveled top-left
				// corner, then it is blocked. the corner is beveled if the tiles above and to the left are not walls. we can
				// ignore the tile to the left because if it was a wall tile, the top vector must have entered this tile from
				// the bottom-left corner, in which case it can't possibly enter the tile above.
				//
				// otherwise, with a beveled top-left corner, the slope of the vector must be greater than or equal to the
				// slope of the vector to the top center of the tile (x*2, topY*2+1) in order for it to miss the wall and
				// pass into the tile above
				if (top.GreaterOrEqual(topY * 2 + 1, x * 2)
					&& !BlocksLight(tilemap, x, topY + 1, origin, octant)) topY++;
			}
			else // the tile doesn't block light
			{
				// since this tile doesn't block light, there's nothing to stop it from passing into the tile above, and it
				// does so if the vector is greater than the vector for the bottom-right corner of the tile above. however,
				// there is one additional consideration. later code in this method assumes that if a tile blocks light then
				// it must be visible, so if the tile above blocks light we have to make sure the light actually impacts the
				// wall shape. now there are three cases: 1) the tile above is clear, in which case the vector must be above
				// the bottom-right corner of the tile above, 2) the tile above blocks light and does not have a beveled
				// bottom-right corner, in which case the vector must be above the bottom-right corner, and 3) the tile above
				// blocks light and does have a beveled bottom-right corner, in which case the vector must be above the
				// bottom center of the tile above (i.e. the corner of the beveled edge).
				// 
				// now it's possible to merge 1 and 2 into a single check, and we get the following: if the tile above and to
				// the right is a wall, then the vector must be above the bottom-right corner. otherwise, the vector must be
				// above the bottom center. this works because if the tile above and to the right is a wall, then there are
				// two cases: 1) the tile above is also a wall, in which case we must check against the bottom-right corner,
				// or 2) the tile above is not a wall, in which case the vector passes into it if it's above the bottom-right
				// corner. so either way we use the bottom-right corner in that case. now, if the tile above and to the right
				// is not a wall, then we again have two cases: 1) the tile above is a wall with a beveled edge, in which
				// case we must check against the bottom center, or 2) the tile above is not a wall, in which case it will
				// only be visible if light passes through the inner square, and the inner square is guaranteed to be no
				// larger than a wall diamond, so if it wouldn't pass through a wall diamond then it can't be visible, so
				// there's no point in incrementing topY even if light passes through the corner of the tile above. so we
				// might as well use the bottom center for both cases.
				int ax = x * 2; // center
				if (BlocksLight(tilemap, x + 1, topY + 1, origin, octant)) ax++; // use bottom-right if the tile above and right is a wall
				if (top.Greater(topY * 2 + 1, ax)) topY++;
			}
		}

		int bottomY;
		if (bottom.y == 0) // if bottom == 0/?, then it's hitting the tile at Y=0 dead center. this is special-cased because
		{                 // bottom.Y starts at zero and remains zero as long as it doesn't hit anything, so it's common
			bottomY = 0;
		}
		else // bottom > 0
		{
			bottomY = ((x * 2 - 1) * bottom.y + bottom.x) / (bottom.x * 2); // the tile that the bottom vector enters from the left
			// code below assumes that if a tile is a wall then it's visible, so if the tile contains a wall we have to
			// ensure that the bottom vector actually hits the wall shape. it misses the wall shape if the top-left corner
			// is beveled and bottom >= (bottomY*2+1)/(x*2). finally, the top-left corner is beveled if the tiles to the
			// left and above are clear. we can assume the tile to the left is clear because otherwise the bottom vector
			// would be greater, so we only have to check above
			if (bottom.GreaterOrEqual(bottomY * 2 + 1, x * 2)
				&& BlocksLight(tilemap, x, bottomY, origin, octant) &&
				!BlocksLight(tilemap, x, bottomY + 1, origin, octant))
			{
				bottomY++;
			}
		}

		// go through the tiles in the column now that we know which ones could possibly be visible
		int wasOpaque = -1; // 0:false, 1:true, -1:not applicable
		for (int y = topY; (int)y >= (int)bottomY; y--) // use a signed comparison because y can wrap around when decremented
		{
			float distance = Vector2i{ x, y }.Distance(tilemap->Origin);
			if (distance < rangeLimit) // skip the tile if it's out of visual range
			{
				bool isOpaque = BlocksLight(tilemap, x, y, origin, octant);
				// every tile where topY > y > bottomY is guaranteed to be visible. also, the code that initializes topY and
				// bottomY guarantees that if the tile is opaque then it's visible. so we only have to do extra work for the
				// case where the tile is clear and y == topY or y == bottomY. if y == topY then we have to make sure that
				// the top vector is above the bottom-right corner of the inner square. if y == bottomY then we have to make
				// sure that the bottom vector is below the top-left corner of the inner square
				bool isVisible =
					isOpaque || ((y != topY || top.Greater(y * 4 - 1, x * 4 + 1)) && (y != bottomY || bottom.Less(y * 4 + 1, x * 4 - 1)));
				// NOTE: if you want the algorithm to be either fully or mostly symmetrical, replace the line above with the
				// following line (and uncomment the Slope.LessOrEqual method). the line ensures that a clear tile is visible
				// only if there's an unobstructed line to its center. if you want it to be fully symmetrical, also remove
				// the "isOpaque ||" part and see NOTE comments further down
				// bool isVisible = isOpaque || ((y != topY || top.GreaterOrEqual(y, x)) && (y != bottomY || bottom.LessOrEqual(y, x)));
				if (isVisible) SetVisible(tilemap, x, y, origin, octant);

				// if we found a transition from clear to opaque or vice versa, adjust the top and bottom vectors
				if (x != rangeLimit) // but don't bother adjusting them if this is the last column anyway
				{
					if (isOpaque)
					{
						if (wasOpaque == 0) // if we found a transition from clear to opaque, this sector is done in this column,
						{                  // so adjust the bottom vector upward and continue processing it in the next column
						  // if the opaque tile has a beveled top-left corner, move the bottom vector up to the top center.
						  // otherwise, move it up to the top left. the corner is beveled if the tiles above and to the left are
						  // clear. we can assume the tile to the left is clear because otherwise the vector would be higher, so
						  // we only have to check the tile above
							int nx = x * 2, ny = y * 2 + 1; // top center by default
							// NOTE: if you're using full symmetry and want more expansive walls (recommended), comment out the next line
							if (BlocksLight(tilemap, x, y, origin, octant)) nx--; // top left if the corner is not beveled
							if (top.Greater(ny, nx)) // we have to maintain the invariant that top > bottom, so the new sector
							{                       // created by adjusting the bottom is only valid if that's the case
							  // if we're at the bottom of the column, then just adjust the current sector rather than recursing
							  // since there's no chance that this sector can be split in two by a later transition back to clear
								if (y == bottomY) { bottom = { ny, nx }; break; } // don't recurse unless necessary
								else ComputeOctant(tilemap, octant, origin,
									rangeLimit, x + 1, top, { ny, nx });
							}
							else // the new bottom is greater than or equal to the top, so the new sector is empty and we'll ignore
							{    // it. if we're at the bottom of the column, we'd normally adjust the current sector rather than
								if (y == bottomY) return; // recursing, so that invalidates the current sector and we're done
							}
						}
						wasOpaque = 1;
					}
					else
					{
						if (wasOpaque > 0) // if we found a transition from opaque to clear, adjust the top vector downwards
						{
							// if the opaque tile has a beveled bottom-right corner, move the top vector down to the bottom center.
							// otherwise, move it down to the bottom right. the corner is beveled if the tiles below and to the right
							// are clear. we know the tile below is clear because that's the current tile, so just check to the right
							int nx = x * 2, ny = y * 2 + 1; // the bottom of the opaque tile (oy*2-1) equals the top of this tile (y*2+1)
							// NOTE: if you're using full symmetry and want more expansive walls (recommended), comment out the next line
							if (BlocksLight(tilemap, x + 1, y + 1, origin, octant)) nx++; // check the right of the opaque tile (y+1), not this one
							// we have to maintain the invariant that top > bottom. if not, the sector is empty and we're done
							if (bottom.GreaterOrEqual(ny, nx)) return;
							top = { ny, nx };
						}
						wasOpaque = 0;
					}
				}
			}
		}

		// if the column didn't end in a clear tile, then there's no reason to continue processing the current sector
		// because that means either 1) wasOpaque == -1, implying that the sector is empty or at its range limit, or 2)
		// wasOpaque == 1, implying that we found a transition from clear to opaque and we recursed and we never found
		// a transition back to clear, so there's nothing else for us to do that the recursive method hasn't already. (if
		// we didn't recurse (because y == bottomY), it would have executed a break, leaving wasOpaque equal to 0.)
		if (wasOpaque != 0) break;
	}
}

global_var constexpr int TranslationTable[8][4] =
{
	{  1,  0,  0, -1 },
	{  0,  1, -1,  0 },
	{  0, -1, -1,  0 },
	{ -1,  0,  0, -1 },
	{ -1,  0,  0,  1 },
	{  0, -1,  1,  0 },
	{  0,  1,  1,  0 },
	{  1,  0,  0,  1 },
};

internal void ComputeLightShadowCast(ChunkedTileMap* tilemap,
	const Light& light, uint8_t octant, Vector2i origin,
	int rangeLimit, int x, Slope top, Slope bottom)
{
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
			TileCoord coord =
			{
				txty.x - GetGame()->LightMap.LightMapOffset.x,
				txty.y - GetGame()->LightMap.LightMapOffset.y
			};
			int index = coord.x + coord.y * SCREEN_WIDTH_TILES;
			bool inRange = index >= 0 
				&& index < State.Size
				&& CTileMap::IsTileInBounds(tilemap, txty)
				&& ((distance = Vector2i{ x, y }.Distance(tilemap->Origin)) <= rangeLimit);
			if (inRange && !State.CheckedTiles[index])
			{
				State.CheckedTiles[index] = true;
				LightsUpdateTileColor(index, distance, light);
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
						else ComputeLightShadowCast(tilemap, light, octant, origin, rangeLimit, x + 1, top, newBottom);
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
LightsUpdateTileColor(int index, float distance, const Light& light)
{
	// https://www.desmos.com/calculator/nmnaud1hrw
	const float a = 0.0f;
	const float b = 0.1f;
	//distance = distance / light.Radius
	float attenuation = 1.0f / (1.0f + a * distance + b * distance * distance);
	
	constexpr float inverse = 1.0f / 255.0f;
	GetGame()->LightMap.LightColors[index].x += (float)light.Color.r * inverse * attenuation;
	GetGame()->LightMap.LightColors[index].y += (float)light.Color.g * inverse * attenuation;
	GetGame()->LightMap.LightColors[index].z += (float)light.Color.b * inverse * attenuation;
}

void
LightsUpdateTileColorTile(Vector2i tileCoord, float distance, const Light& light)
{
	// im lazy and this just converts for you
	TileCoord coord =
	{
		tileCoord.x - GetGame()->LightMap.LightMapOffset.x,
		tileCoord.y - GetGame()->LightMap.LightMapOffset.y
	};
	int index = coord.x + coord.y * SCREEN_WIDTH_TILES;
	LightsUpdateTileColor(index, distance, light);
}

internal inline bool
IsTileValid(Vector2i pos)
{
	if (!CTileMap::IsTileInBounds(&GetGame()->World.ChunkedTileMap, pos)) return false;
	//if (pos.x < GetGame()->LightMap.LightMapOffset.x ||
	//	pos.y < GetGame()->LightMap.LightMapOffset.y ||
	//	pos.x >= GetGame()->LightMap.LightMapOffset.x + SCREEN_WIDTH_TILES ||
	//	pos.y >= GetGame()->LightMap.LightMapOffset.y + SCREEN_HEIGHT_TILES) return false;

	return true;
}
