#include "Lighting.h"

#include "Game.h"
#include "SEntity.h"
#include "ThreadedLights.h"
#include "WickedEngine/Jobs.h"

#include <raylib/src/raymath.h>

#define ENABLE_CONE_FOV 1
#define ENABLE_THREADED_LIGHTS 1
#define LIGHT_UPDATE_THREADS 2
#define LIGHT_STATIC_THREADS 2
#define LIGHT_MAX_THEADS (LIGHT_UPDATE_THREADS + LIGHT_STATIC_THREADS)

// Octants to search to if using cone fov, will not check the back 180degrees
// I think the algorithm can support fov directly using input slope and octants,
// however I just use a fov angle and get the angle between tile and player direction.
// But to help avoid unnecessary checks I still want to cull octants
// N, E, S, W
constexpr global_var uint8_t OctantsForDirection[4][4] =
{
	{ 0, 1, 2, 3 },
	{ 6, 7, 0, 1 },
	{ 4, 5, 6, 7 },
	{ 2, 3, 4, 5 }
};

internal void
ComputeOctant(ChunkedTileMap* tilemap, uint8_t octant,
	Vector2i origin, int rangeLimit, int x, Slope top, Slope bottom);

// Most of these implementations for lighting I have gotten
// from http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html

void 
LightsInitialize(LightingState* lightingState)
{
	float table[] = {
		0.0f, 0.0f, 0.1f, 0.0f, 0.0f,
		0.0f, 0.1f, 0.2f, 0.1f, 0.0f,
		0.1f, 0.2f, 0.4f, 0.2f, 0.1f,
		0.0f, 0.1f, 0.2f, 0.1f, 0.0f,
		0.0f, 0.0f, 0.1f, 0.0f, 0.0f
	};
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Basic].LightModifers.Assign(table, ArrayLength(table));
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Basic].x = -2;
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Basic].y = -2;
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Basic].Width = 5;
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Basic].Height = 5;

	float lavaTable[] =
	{
		0.0f, 0.2f, 0.0f,
		0.2f, 0.25f, 0.2f,
		0.0f, 0.2f, 0.0f,
	};
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Lava].LightModifers.Assign(lavaTable, ArrayLength(lavaTable));
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Lava].x = -1;
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Lava].y = -1;
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Lava].Width = 3;
	lightingState->StaticLightTypes[(uint8_t)StaticLightTypes::Lava].Height = 3;
}

internal constexpr unsigned char
Clamp0255(uint16_t val0, uint16_t val1)
{
	uint16_t res = val0 + val1;
	return (unsigned char)((res >= UINT8_MAX) ? UINT8_MAX : res);
}

internal constexpr bool
IsInBounds(Vector2i coord, Vector2i xy, Vector2i wh)
{
	return (coord.x >= xy.x && coord.y >= xy.y && coord.x < wh.x && coord.y < wh.y);
}

void 
StaticLightDrawToChunk(StaticLight* light, TileMapChunk* chunkDst, ChunkedTileMap* tilemap)
{
	//PROFILE_BEGIN();

	Vector2i wh = chunkDst->StartTile + Vector2i{ CHUNK_DIMENSIONS, CHUNK_DIMENSIONS };

	for (int i = 0; i < 9; ++i)
	{
		Vector2i pos = light->Pos + LavaLightOffsets[i];
		if (IsInBounds(pos, chunkDst->StartTile, wh))
		{
			uint16_t r = (uint16_t)((float)light->Color.r * LavaLightWeights[i]);
			uint16_t g = (uint16_t)((float)light->Color.g * LavaLightWeights[i]);
			uint16_t b = (uint16_t)((float)light->Color.b * LavaLightWeights[i]);
			uint16_t a = (uint16_t)((float)light->Color.a * LavaLightWeights[i]);
			size_t idx = CTileMap::GetTileLocalIndex(pos);
			chunkDst->TileColors[idx].r = Clamp0255(chunkDst->TileColors[idx].r, r);
			chunkDst->TileColors[idx].g = Clamp0255(chunkDst->TileColors[idx].g, g);
			chunkDst->TileColors[idx].b = Clamp0255(chunkDst->TileColors[idx].b, b);
			chunkDst->TileColors[idx].a = Clamp0255(chunkDst->TileColors[idx].a, a);
		}
	}

	//PROFILE_END();
}

void
StaticLightRemoveFromChunk(StaticLight* light, TileMapChunk* chunkDst, TileMapChunk* chunkSrc, Vector2i offset)
{
	Vector2i lightPos = light->Pos + offset;
	for (int i = 0; i < 9; ++i)
	{
		Vector2i pos = lightPos + LavaLightOffsets[i];
		uint16_t r = (uint16_t)((float)light->Color.r * LavaLightWeights[i]);
		uint16_t g = (uint16_t)((float)light->Color.g * LavaLightWeights[i]);
		uint16_t b = (uint16_t)((float)light->Color.b * LavaLightWeights[i]);
		uint16_t a = (uint16_t)((float)light->Color.a * LavaLightWeights[i]);
		size_t idx = CTileMap::GetTileLocalIndex(pos);
		chunkDst->TileColors[idx].r = Clamp0255(chunkDst->TileColors[idx].r, r);
		chunkDst->TileColors[idx].g = Clamp0255(chunkDst->TileColors[idx].g, g);
		chunkDst->TileColors[idx].b = Clamp0255(chunkDst->TileColors[idx].b, b);
		chunkDst->TileColors[idx].a = Clamp0255(chunkDst->TileColors[idx].a, a);
	}
}

internal void 
UpdatingLightUpdate(Light* lightPtr, Game* game, float dt)
{
	SASSERT(lightPtr);
	SASSERT(game);
	UpdatingLight* light = (UpdatingLight*)lightPtr;

	if (light->EntityId != ENTITY_NOT_FOUND)
	{
		UID uid = { .Number = light->EntityId };
		SEntity* entity = EntityGet(uid);
		SASSERT(entity);
		if (entity)
			light->Pos = entity->TilePos;
	}

	light->LastUpdate += dt;
	if (light->LastUpdate > UpdatingLight::UPDATE_RATE)
	{
		light->LastUpdate = 0.0f;

		SRandom* threadedRandom = GetThreadSRandom();
		float rand = SRandNextFloat(threadedRandom);
		if (rand < 0.35f)
		{
			uint64_t index = SRandNextRange(threadedRandom, 0, 3);
			light->Color = light->Colors[index];
		}
		light->Radius = SRandNextFloatRange(threadedRandom, light->MinIntensity, light->MaxIntensity);
	}
}

uint32_t 
LightAddUpdating(LightingState* lightState, UpdatingLight* light)
{
	SASSERT(lightState);
	SASSERT(light);

	UpdatingLight* lightDst = lightState->UpdatingLightPool.allocate();
	SASSERT(lightDst);
	
	memcpy(lightDst, light, sizeof(UpdatingLight));

	lightDst->LightType = LightType::Updating;
	lightDst->UpdateFunc = UpdatingLightUpdate;

	++lightState->NumOfUpdatingLights;

	uint32_t id = lightState->LightPtrs.Add((Light**)&lightDst);
	return id;
}

void 
LightRemove(LightingState* lightState, uint32_t lightId)
{
	Light** lightPtr = lightState->LightPtrs.RemoveAndGetPtr(lightId);
	if (!lightPtr)
		return;

	Light* light = *lightPtr;
	lightState->UpdatingLightPool.deallocate((UpdatingLight*)light);
	--lightState->NumOfUpdatingLights;
}


uint32_t 
GetNumOfLights()
{
	return GetGame()->LightingState.NumOfUpdatingLights;
}

void 
LightsUpdate(LightingState* lightState, Game* game)
{
	double start = GetTime();

	ChunkedTileMap* tilemap = &game->Universe.World.ChunkedTileMap;

	// Threaded lighting

	Color* colorArrayPtrs[LIGHT_MAX_THEADS];
	size_t size = GetGameApp()->View.TotalTilesOnScreen * sizeof(Color);
	for (int i = 0; i < LIGHT_MAX_THEADS; ++i)
	{
		colorArrayPtrs[i] = (Color*)SMemTempAlloc(size);
		SMemClear(colorArrayPtrs[i], size);
	}

	uint32_t totalUpdatingLights = lightState->LightPtrs.Data.Capacity;
	uint32_t groupUpdateSize = (uint32_t)std::ceil((float)totalUpdatingLights / (float)LIGHT_UPDATE_THREADS);

	std::function<void(wi::jobsystem::JobArgs)> task = [lightState, tilemap, &colorArrayPtrs](wi::jobsystem::JobArgs job)
	{
		//PROFILE_BEGIN_EX("LightsUpdate::UpdatingLights");

		uint32_t lightIndex = job.jobIndex;
		uint32_t threadIndex = job.groupID + LIGHT_STATIC_THREADS;

		SASSERT(lightIndex < lightState->LightPtrs.Data.Capacity);
		SASSERT(threadIndex >= LIGHT_STATIC_THREADS);
		SASSERT(threadIndex < LIGHT_MAX_THEADS);

		Light** lightPtr = lightState->LightPtrs.At(lightIndex);
		if (lightPtr)
		{
			Light* light = *lightPtr;
			if (TileInsideCullRect(light->Pos))
			{
				Color* threadArray = colorArrayPtrs[threadIndex];
				ThreadedLightUpdate(light, threadArray, tilemap, GetGameApp()->View.ResolutionInTiles.x);
			}
		}
		//PROFILE_END();
	};

	wi::jobsystem::context ctx = {};
	wi::jobsystem::Dispatch(ctx, totalUpdatingLights, groupUpdateSize, task, 0);

	// Line of sight

	Vector2i playerPos = GetClientPlayer()->TilePos;
	if (CTileMap::IsTileInBounds(tilemap, playerPos))
	{
		// Tiles around player are always visible
		CTileMap::SetVisible(tilemap, playerPos);
		for (int i = 0; i < ArrayLength(Vec2i_NEIGHTBORS); ++i)
		{
			Vector2i pos = Vec2i_NEIGHTBORS[i].Add(playerPos);
			CTileMap::SetVisible(tilemap, pos);
		}

		// FOV visiblity
#if ENABLE_CONE_FOV
		uint8_t playerDirection = (uint8_t)GetClientPlayer()->LookDir;
		for (uint8_t octant = 0; octant < 4; ++octant)
		{
			uint8_t trueOctant = OctantsForDirection[playerDirection][octant];
			ComputeOctant(tilemap, trueOctant, playerPos, 16, 1, { 1, 1 }, { 0, 1 });
		}
#else
		for (uint8_t octant = 0; octant < 8; ++octant)
		{
			ComputeOctant(tilemap, octant, playerPos, 16, 1, { 1, 1 }, { 0, 1 });
		}
#endif
	}

	// Wait updating lights
	wi::jobsystem::Wait(ctx);

	// Sync all lights to light color array. All threads must finish!
	for (int i = 0; i < GetGameApp()->View.TotalTilesOnScreen; ++i)
	{
		for (int j = 0; j < LIGHT_MAX_THEADS; ++j)
		{
			game->LightingRenderer.TileColors.Memory[i].r = Clamp0255(game->LightingRenderer.TileColors.Memory[i].r, colorArrayPtrs[j][i].r);
			game->LightingRenderer.TileColors.Memory[i].g = Clamp0255(game->LightingRenderer.TileColors.Memory[i].g, colorArrayPtrs[j][i].g);
			game->LightingRenderer.TileColors.Memory[i].b = Clamp0255(game->LightingRenderer.TileColors.Memory[i].b, colorArrayPtrs[j][i].b);
			game->LightingRenderer.TileColors.Memory[i].a = Clamp0255(game->LightingRenderer.TileColors.Memory[i].a, colorArrayPtrs[j][i].a);
		}

	}

	GetGameApp()->DebugLightTime = GetTime() - start;
}

internal bool 
BlocksLight(ChunkedTileMap* tilemap, int x, int y, Vector2i origin, uint8_t octant)
{
	Vector2i newPos = origin;
	newPos.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
	newPos.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];
	return CTileMap::BlocksLight(tilemap, newPos);
}

internal void 
SetVisible(ChunkedTileMap* tilemap, int x, int y, Vector2i origin, uint8_t octant)
{
	Vector2i newPos = origin;
	newPos.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
	newPos.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];
#if ENABLE_CONE_FOV
	constexpr float coneFov = (80.0f * DEG2RAD);
	Vector2 lookVector = TileDirectionVectors[(uint8_t)GetClientPlayer()->LookDir];
	Vector2 length = Vector2Normalize(newPos.Subtract(GetClientPlayer()->TilePos).AsVec2());
	float dot = Vector2LineAngle(lookVector, length);
	if (dot < coneFov)
		CTileMap::SetVisible(tilemap, newPos);
#else
	CTileMap::SetVisible(tilemap, newPos);
#endif
}

internal void 
ComputeOctant(ChunkedTileMap* tilemap, uint8_t octant,
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
			float distance = Vector2i{ x, y }.Distance({});
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
