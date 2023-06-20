#include "Lighting.h"

#include "Game.h"
#include "Structures/SHashSet.h"
#include "Structures/SLinkedList.h"
#include "raylib/src/raymath.h"
#include "WickedEngine/Jobs.h"

#include <cmath>
#include <chrono>

#define USE_THREADED_LIGHTS 1

// Most of these implementations for lighting I have gotten
// from http://www.adammil.net/blog/v125_roguelike_vision_algorithms.html

void LightsInitialize(LightingState* lightingState)
{
	lightingState->UpdatingLights.Reserve(64);
	lightingState->StaticLights.Reserve(64);
	lightingState->StaticLightTypes.EnsureSize((uint8_t)StaticLightTypes::MaxTypes);

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

void DrawStaticLights(ChunkedTileMap* tilemap, const StaticLight* light)
{
	StaticLightType* lightType = &GetGame()->LightingState.StaticLightTypes[(uint8_t)light->StaticLightType];

	Vector2i pos = Vector2i::FromVec2(light->Pos);
	Vector2i startPos = pos + Vector2i{ lightType->x, lightType->y };

	uint8_t i = 0;
	for (uint8_t y = 0; y < lightType->Height; ++y)
	{
		for (uint8_t x = 0; x < lightType->Width; ++x)
		{
			Vector2i curWorld = startPos + Vector2i{ x, y };
			if (CTileMap::IsTileInBounds(tilemap, curWorld))
			{
				Vector2i curCull = WorldTileToCullTile(curWorld);
				size_t index = curCull.x + curCull.y * CULL_WIDTH_TILES;
				float multiplier = lightType->LightModifers[i];
				constexpr float inverse = 1.0f / 255.0f;
				GetGame()->LightingRenderer.Tiles[index].x += (float)light->Color.r * inverse * multiplier;
				GetGame()->LightingRenderer.Tiles[index].y += (float)light->Color.g * inverse * multiplier;
				GetGame()->LightingRenderer.Tiles[index].z += (float)light->Color.b * inverse * multiplier;
			}
			++i;
		}
	}
}

void DrawStaticTileLight(Vector2i tilePos, Color color, StaticLightTypes type)
{
	ChunkedTileMap* tilemap = &GetGame()->Universe.World.ChunkedTileMap;
	StaticLightType* lightType = &GetGame()->LightingState.StaticLightTypes[(uint8_t)type];

	Vector2i startPos = tilePos + Vector2i{ lightType->x, lightType->y };
	startPos = WorldTileToCullTile(startPos);
	uint8_t i = 0;
	for (uint8_t y = 0; y < lightType->Height; ++y)
	{
		for (uint8_t x = 0; x < lightType->Width; ++x)
		{
			float multiplier = lightType->LightModifers[i++];
			if (multiplier > 0.f)
			{
				Vector2i cur = startPos + Vector2i{ x, y };
				size_t index = cur.x + cur.y * CULL_WIDTH_TILES;
				if (index >= (size_t)CULL_TOTAL_TILES)
					continue;

				constexpr float inverse = 1.0f / 255.0f;
				float m = inverse * multiplier;
				GetGame()->LightingRenderer.Tiles[index].x += (float)color.r * m;
				GetGame()->LightingRenderer.Tiles[index].y += (float)color.g * m;
				GetGame()->LightingRenderer.Tiles[index].z += (float)color.b * m;
			}
		}
	}
}


constexpr global_var Vector2i LavaLightOffsets[9] =
{
	{-1, -1}, {0, -1}, {1,-1},
	{-1, 0}, {0, 0}, {1,0},
	{-1, 1}, {0, 1}, {1,1},
};
constexpr global_var float Inverse = 1.0f / 255.0f;
constexpr global_var float LavaLightWeights[9] =
{
		0.05f * Inverse, 0.15f * Inverse, 0.05f * Inverse,
		0.15f * Inverse, 0.25f * Inverse, 0.15f * Inverse,
		0.05f * Inverse, 0.15f * Inverse, 0.05f * Inverse,
};
void DrawStaticLavaLight(Vector2i tilePos, Color color)
{
	ChunkedTileMap* tilemap = &GetGame()->Universe.World.ChunkedTileMap;
	LightingRenderer* lightRenderer = &GetGame()->LightingRenderer;

	Vector2i cullPos = WorldTileToCullTile(tilePos);
	for (int i = 0; i < 9; ++i)
	{
		Vector2i pos = cullPos.Add(LavaLightOffsets[i]);
		size_t idx = pos.x + pos.y * CULL_WIDTH_TILES;
		if (idx < (size_t)CULL_TOTAL_TILES)
		{
			lightRenderer->Tiles[idx].x += (float)color.r * LavaLightWeights[i];
			lightRenderer->Tiles[idx].y += (float)color.g * LavaLightWeights[i];
			lightRenderer->Tiles[idx].z += (float)color.b * LavaLightWeights[i];
		}
	}
}

void ProcessLights(LightingState* lightState, Game* game)
{
	ChunkedTileMap* tilemap = &game->Universe.World.ChunkedTileMap;

	ThreadedLights threadedLights = {};
	threadedLights.AllocateArrays(CULL_TOTAL_TILES);

	wi::jobsystem::context ctx = {};

	std::function<void(wi::jobsystem::JobArgs)> task = [&lightState, &tilemap, &threadedLights](wi::jobsystem::JobArgs job)
	{
		PROFILE_BEGIN_EX("LightsUpdate::LightThread");

		uint32_t lightIndex = job.jobIndex;
		uint32_t threadIndex = job.groupID;

		SASSERT(lightIndex < lightState->UpdatingLights.Count);
		SASSERT(threadIndex < 4);

		UpdatingLight* light = &lightState->UpdatingLights[lightIndex];
		if (CheckCollisionPointRec(light->Pos, GetGameApp()->CullRect))
		{
			TileCoord coord = WorldTileToCullTile(Vector2i::FromVec2(light->Pos));
			ProcessLightUpdater(light, CULL_WIDTH_TILES, threadedLights.ColorArrayPtrs[threadIndex], tilemap);
		}

		PROFILE_BEGIN();
	};

	uint32_t totalLights = lightState->LightPtrs.Size;
	uint32_t totalThreads = wi::jobsystem::GetThreadCount();
	uint32_t num = (uint32_t)std::ceil(totalLights / totalThreads);
	wi::jobsystem::Dispatch(ctx, totalLights, num, task, 0);
}

uint32_t LightAddUpdating(LightingState* lightState, UpdatingLight* light)
{
	UpdatingLight* lightDst = lightState->UpdatingLightPool.allocate();
	SASSERT(lightDst);
	
	memcpy(lightDst, light, sizeof(UpdatingLight));

	return lightState->LightPtrs.Add((Light**)&lightDst);
}

void LightRemove(LightingState* lightState, uint32_t lightId)
{
	Light** lightPtr = lightState->LightPtrs.RemoveAndGetPtr(lightId);
	if (!lightPtr)
		return;

	Light* light = *lightPtr;
	SASSERT(light);
	switch (light->LightType)
	{
		case (0):
		{
			lightState->UpdatingLightPool.deallocate((UpdatingLight*)light);
		} break;

		case (1):
		{
			lightState->StaticLightPool.deallocate((StaticLight*)light);
		} break;

		default:
		{
			SASSERT_MSG(false, "Light removed with no matching type!");
			SLOG_ERR("Light removed with no matching type!");
		} break;
	}
}

void LightsAddUpdating(const UpdatingLight& light)
{
	GetGame()->LightingState.UpdatingLights.Push(&light);
}

void LightsAddStatic(const StaticLight& light)
{
	GetGame()->LightingState.StaticLights.Push(&light);
}

uint32_t GetNumOfLights()
{
	return GetGame()->LightingState.UpdatingLights.Count + GetGame()->LightingState.StaticLights.Count;
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

// Cone Field of view

#define ENABLE_CONE_FOV 1

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
#include "WickedEngine/Jobs.h"
#include "ThreadedLights.h"

internal void
ComputeOctant(ChunkedTileMap* tilemap, uint8_t octant,
	Vector2i origin, int rangeLimit, int x, Slope top, Slope bottom);

internal void
ComputeLightShadowCast(ChunkedTileMap* tilemap, const Light* light,
	uint8_t octant, int x, Slope top, Slope bottom);

void LightsUpdate(LightingState* lightingState, Game* game)
{
	PROFILE_BEGIN();

	GetGameApp()->NumOfLightsUpdated = 0;
	double start = GetTime();

	//wi::jobsystem::context ctx = {};
	//std::function<void(wi::jobsystem::JobArgs)> task = [&lightingState](wi::jobsystem::JobArgs job)
	//{
	//	PROFILE_BEGIN_EX("LightsUpdate::Job_UpdateLights");
	//	uint32_t lightIndex = job.jobIndex;
	//	SASSERT(lightIndex < lightingState->UpdatingLights.Count);
	//	UpdatingLight* light = &lightingState->UpdatingLights[lightIndex];
	//	TileCoord coord = WorldTileToCullTile(Vector2i::FromVec2(light->Pos));
	//	int index = coord.x + coord.y * CULL_WIDTH_TILES;
	//	UpdateLightColor(light, index, 0.0f);
	//	ThreadedLightUpdate(light);
	//	PROFILE_END();
	//};
	//wi::jobsystem::Dispatch(ctx, lightingState->UpdatingLights.Count, 16, task, 0);

	ChunkedTileMap* tilemap = &game->Universe.World.ChunkedTileMap;
	Vector2i playerPos = GetClientPlayer()->TilePos;
	TileDirection playerDir = GetClientPlayer()->LookDir;
	Vector2i lookDir = Vec2i_NEIGHTBORS[(uint8_t)playerDir];
	lightingState->PlayerLookVector = Vector2Normalize(lookDir.AsVec2());

#if USE_THREADED_LIGHTS
	ThreadedLights threadedLights = {};
	threadedLights.AllocateArrays(CULL_TOTAL_TILES);
	wi::jobsystem::context ctx = {};
	std::function<void(wi::jobsystem::JobArgs)> task = [&lightingState, &tilemap, &threadedLights](wi::jobsystem::JobArgs job)
	{
		PROFILE_BEGIN_EX("LightsUpdate::LightThread");
		uint32_t lightIndex = job.jobIndex;
		SASSERT(lightIndex < lightingState->UpdatingLights.Count);
		uint32_t threadIndex = job.groupID;
		SASSERT(threadIndex < 4);

		UpdatingLight* light = &lightingState->UpdatingLights[lightIndex];
		if (CheckCollisionPointRec(light->Pos, GetGameApp()->CullRect))
		{
			TileCoord coord = WorldTileToCullTile(Vector2i::FromVec2(light->Pos));
			int index = coord.x + coord.y * CULL_WIDTH_TILES;
			ProcessLightUpdater(light, CULL_WIDTH_TILES, threadedLights.ColorArrayPtrs[threadIndex], tilemap);
		}
		PROFILE_BEGIN();
	};
	uint32_t num = (uint32_t)std::ceilf((float)lightingState->UpdatingLights.Count / 3.0f);
	wi::jobsystem::Dispatch(ctx, lightingState->UpdatingLights.Count, num, task, 0);
#else
	for (uint32_t i = 0; i < lightingState->UpdatingLights.Count; ++i)
	{
		UpdatingLight* light = &lightingState->UpdatingLights[i];
		if (!TileInsideCullRect(Vector2i::FromVec2(light->Pos))) continue;
		light->Update(game);
		//FloodFillLighting(tilemap, light);
		SMemSet(lightingState->CheckedTiles.Data, 0, ArrayLength(lightingState->CheckedTiles));
		LightsUpdateTileColorTile(Vector2i::FromVec2(light->Pos), 1.0, light);
		for (uint8_t octant = 0; octant < 8; ++octant)
		{
			ComputeLightShadowCast(tilemap, light, octant, 1, { 1, 1 }, { 0, 1 });
		}
		++GetGameApp()->NumOfLightsUpdated;
	}
#endif

	PROFILE_BEGIN_EX("LightsUpdate::StaticLightsAndLOS");

	for (uint32_t i = 0; i < lightingState->StaticLights.Count; ++i)
	{
		StaticLight* light = &lightingState->StaticLights[i];
		DrawStaticLights(tilemap, light);
		++GetGameApp()->NumOfLightsUpdated;
	}

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
		for (uint8_t octant = 0; octant < 4; ++octant)
		{
			uint8_t trueOctant = OctantsForDirection[(uint8_t)playerDir][octant];
			ComputeOctant(tilemap, trueOctant, playerPos, 16, 1, { 1, 1 }, { 0, 1 });
		}
#else
		for (uint8_t octant = 0; octant < 8; ++octant)
		{
			ComputeOctant(tilemap, octant, playerPos, 16, 1, { 1, 1 }, { 0, 1 });
		}
#endif
	}
	PROFILE_END();

	//for (uint32_t i = 0; i < lightingState->Lights.Data.Count;)
	//{
	//	UpdatingLight* light = lightingState->Lights.At(i);
	//	if (light)
	//	{
	//		UpdatingLight* light = &lightingState->UpdatingLights[i];
	//		if (!TileInsideCullRect(Vector2i::FromVec2(light->Pos))) continue;
	//		light->Update(game);
	//		//FloodFillLighting(tilemap, light);
	//		SMemSet(lightingState->CheckedTiles.Data, 0, ArrayLength(lightingState->CheckedTiles));
	//		LightsUpdateTileColorTile(Vector2i::FromVec2(light->Pos), 1.0, light);
	//		for (uint8_t octant = 0; octant < 8; ++octant)
	//		{
	//			ComputeLightShadowCast(tilemap, light, octant, 1, { 1, 1 }, { 0, 1 });
	//		}
	//		++i;
	//	}
	//}

#if USE_THREADED_LIGHTS
	PROFILE_BEGIN_EX("LightsUpdate::WaitForLightsJob");
	// Wait for lighting to finish updating
	wi::jobsystem::Wait(ctx);
	threadedLights.UpdateLightColorArray(game->LightingRenderer.Tiles.Data);
	PROFILE_END();
#endif

	GetGameApp()->DebugLightTime = GetTime() - start;
	PROFILE_END();
}


internal bool BlocksLight(ChunkedTileMap* tilemap, int x, int y, Vector2i origin, uint8_t octant)
{
	Vector2i newPos = origin;
	newPos.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
	newPos.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];
	return CTileMap::BlocksLight(tilemap, newPos);
}

internal void SetVisible(ChunkedTileMap* tilemap, int x, int y, Vector2i origin, uint8_t octant)
{
	Vector2i newPos = origin;
	newPos.x += x * TranslationTable[octant][0] + y * TranslationTable[octant][1];
	newPos.y += x * TranslationTable[octant][2] + y * TranslationTable[octant][3];
#if ENABLE_CONE_FOV
	constexpr float coneFov = (80.0f * DEG2RAD);
	Vector2i length = newPos.Subtract(GetClientPlayer()->TilePos);

	float dot = Vector2LineAngle(GetGame()->LightingState.PlayerLookVector, Vector2Normalize(length.AsVec2()));
	if (dot < coneFov)
		CTileMap::SetVisible(tilemap, newPos);
#else
	CTileMap::SetVisible(tilemap, newPos);
#endif
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
			float distance = Vector2i{ x, y }.Distance(TILEMAP_ORIGIN);
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

internal void
ComputeLightShadowCast(ChunkedTileMap* tilemap, const Light* light,
	uint8_t octant, int x, Slope top, Slope bottom)
{
	Vector2i origin = Vector2i::FromVec2(light->Pos);
	int rangeLimit = (int)light->Radius;
	float rangeSqr = light->Radius * light->Radius;
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
				&& ((distance = Vector2Distance(Vector2i{ x, y }.AsVec2(), TILEMAP_ORIGIN.AsVec2())) <= rangeLimit);
			if (inRange)
			{
				TileCoord coord = WorldTileToCullTile(txty);
				int index = coord.x + coord.y * CULL_WIDTH_TILES;
				if (!GetGame()->LightingState.CheckedTiles[index])
				{
					GetGame()->LightingState.CheckedTiles[index] = true;
					LightsUpdateTileColor(index, distance, light);
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
						else ComputeLightShadowCast(tilemap, light, octant, x + 1, top, newBottom);
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
LightsUpdateTileColor(int index, float distance, const Light* light)
{
	SASSERT(index >= 0);
	SASSERT(index < CULL_TOTAL_TILES);

	// https://www.desmos.com/calculator/nmnaud1hrw
	constexpr float a = 0.0f;
	constexpr float b = 0.1f;
	float attenuation = 1.0f / (1.0f + a * distance + b * distance * distance);

	constexpr float inverse = 1.0f / 255.0f;
	float intensity = light->Color.a * inverse;
	float multiplier = attenuation * inverse * intensity;
	GetGame()->LightingRenderer.Tiles[index].x += (float)light->Color.r * multiplier;
	GetGame()->LightingRenderer.Tiles[index].y += (float)light->Color.g * multiplier;
	GetGame()->LightingRenderer.Tiles[index].z += (float)light->Color.b * multiplier;
}

void
LightsUpdateTileColorTile(Vector2i tileCoord, float distance, const Light* light)
{
	TileCoord coord = WorldTileToCullTile(tileCoord);
	int index = coord.x + coord.y * CULL_WIDTH_TILES;
	constexpr float a = 0.0f;
	constexpr float b = 0.1f;
	float attenuation = 1.0f / (1.0f + a * distance + b * distance * distance);
	constexpr float inverse = 1.0f / 255.0f;
	float intensity = light->Color.a * inverse;
	float multiplier = attenuation * inverse * intensity;
	GetGame()->LightingRenderer.Tiles[index].x += (float)light->Color.r * multiplier;
	GetGame()->LightingRenderer.Tiles[index].y += (float)light->Color.g * multiplier;
	GetGame()->LightingRenderer.Tiles[index].z += (float)light->Color.b * multiplier;
}

//void DrawLightWithShadows(Vector2 pos, const UpdatingLightSource& src)
//{
//	ChunkedTileMap* tilemap = &GetGame()->Universe.World.ChunkedTileMap;
//	Light light = {};
//	light.Pos = pos;
//	light.Radius = src.Radius;
//	light.Color = src.FinalColor;
//	SMemSet(GetGame()->LightingState.CheckedTiles.Data, 0, ArrayLength(GetGame()->LightingState.CheckedTiles));
//	LightsUpdateTileColorTile(Vector2i::FromVec2(pos), 1.0, &light);
//	for (uint8_t octant = 0; octant < 8; ++octant)
//	{
//		ComputeLightShadowCast(tilemap, &light, octant, 1, { 1, 1 }, { 0, 1 });
//	}
//}

internal inline bool BlocksLight(ChunkedTileMap* tilemap, Vector2i pos)
{
	return (!CTileMap::IsTileInBounds(tilemap, pos) || CTileMap::BlocksLight(tilemap, pos));
}

bool FloodFillLighting(ChunkedTileMap* tilemap, Light* light)
{
	SASSERT(tilemap);
	SASSERT(light);

	Vector2i pos = Vector2i::FromVec2(light->Pos);


	SHashSet<Vector2i> visited = {};
	visited.Allocator = SAllocator::Temp;
	int str = (int)light->Radius * 2;
	visited.Reserve(str * str);

	SLinkedList<Vector2i> stack = {};
	stack.Allocator = SAllocator::Temp;

	LightsUpdateTileColorTile(pos, 1.0, light);

	stack.Push(&pos);
	while (stack.Peek())
	{
		Vector2i cur = stack.PopValue();
		for (int i = 0; i < ArrayLength(Vec2i_NEIGHTBORS); ++i)
		{
			Vector2i next = cur + Vec2i_NEIGHTBORS[i];
			if (visited.Contains(&next)) continue;
			visited.Insert(&next);

			float dist = Vector2Distance(pos.AsVec2(), next.AsVec2());
			if (dist <= light->Radius)
			{
				LightsUpdateTileColorTile(next, dist, light);
				if (BlocksLight(tilemap, next)) continue;
				else stack.Push(&next);
			}
		}
	}
	return true;
}

union FloodFillData
{
	struct
	{
		int XMin;
		int XMax;
		int Y;
		int UpOrDown;
		int ExtendLeft;
		int ExtendRight;

	};
	int r[6];
};

internal inline bool test(const Light* light, int x, int y)
{
	ChunkedTileMap* tilemap = &GetGame()->Universe.World.ChunkedTileMap;
	if (CTileMap::BlocksLight(tilemap, { x, y })) return false;
	float distance = Vector2Distance(light->Pos, { (float)x, (float)y });
	return distance < light->Radius;
}

internal inline void paint(const Light* light, int x, int y)
{
	//ChunkedTileMap* tilemap = &GetGame()->World.ChunkedTileMap;
	LightsUpdateTileColorTile({ x, y }, 1.0, light);
	//CTileMap::LightsUpdateTileColorTile(tilemap, { x, y });
}

void FloodFillScanline(const Light* light, int x, int y, int width, int height, bool diagonal)//, bool (*test)(int, int)), void (*paint)(int, int))
{
	// xMin, xMax, y, down[true] / up[false], extendLeft, extendRight
	SLinkedList<FloodFillData> ranges = {};
	ranges.Allocator = SAllocator::Temp;

	FloodFillData data = { x, x, y, 0, 1, 1 };
	ranges.Push(&data);

	paint(light, x, y);
	int i = 0;
	while (ranges.HasNext())
	{
		i++;
		data = ranges.PopValue();
		bool down = data.r[3] == 1;
		bool up = data.r[3] == 0;

		int startX = x - (int)light->Radius;
		int startY = y - (int)light->Radius;

		// extendLeft
		int minX = data.r[0];
		int y = data.r[2];
		if (data.r[4])
		{
			while (minX > startX && test(light, minX - 1, y))
			{
				minX--;
				paint(light, minX, y);
			}
		}
		int maxX = data.r[1];
		// extendRight
		if (data.r[5])
		{
			while (maxX < width - 1 && test(light, maxX + 1, y))
			{
				maxX++;
				paint(light, maxX, y);
			}
		}

		if (diagonal)
		{
			// extend range looked at for next lines
			if (minX > 0) minX--;
			if (maxX < width - 1) maxX++;
		}
		else
		{
			// extend range ignored from previous line
			data.r[0]--;
			data.r[1]++;
		}

		auto addNextLine = [&](int newY, bool isNext, bool downwards)
		{
			int rMinX = minX;
			bool inRange = false;
			for (int x = minX; x <= maxX; x++)
			{
				// skip testing, if testing previous line within previous range
				bool empty = (isNext || (x<data.r[0] || x>data.r[1])) && test(light, x, newY);
				if (!inRange && empty)
				{
					rMinX = x;
					inRange = true;
				}
				else if (inRange && !empty)
				{
					FloodFillData push = { rMinX, x - 1, newY, downwards ? 1 : 0 , rMinX == minX ? 1 : 0 , 0 };
					ranges.Push(&push);
					inRange = false;
				}
				if (inRange)
				{
					paint(light, x, newY);
				}
				// skip
				if (!isNext && x == data.r[0])
				{
					x = data.r[1];
				}
			}
			if (inRange)
			{
				FloodFillData push = { rMinX, maxX - 1, newY, downwards ? 1 : 0 , rMinX == minX ? 1 : 0 , 1 };
				ranges.Push(&push);
			}
		};

		if (y < height)
			addNextLine(y + 1, !up, true);

		if (y > startY)
			addNextLine(y - 1, !down, false);
	}
	SLOG_INFO("%d", i);
}

//std::chrono::milliseconds interval(1000);
//std::chrono::milliseconds num_elements_per_frame(500);
//auto start_time = std::chrono::high_resolution_clock::now();
//int num_frames = 0;
//while (true)
//{
//	auto current_time = std::chrono::high_resolution_clock::now();
//	auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
//	if (elapsed_time >= interval)
//	{
//		start_time = current_time;
//		int start_index = num_frames * num_elements_per_frame.count() / interval.count();
//		int end_index = (num_frames + 1) * num_elements_per_frame.count() / interval.count();
//		for (int i = start_index; i < end_index && i < State.UpdatingLights.Count; i++)
//		{
//			UpdatingLight* light = &State.UpdatingLights[i];
//			light->Update(game);
//		}
//		num_frames++;
//	}
//	if (num_frames * num_elements_per_frame.count() / interval.count() >= State.UpdatingLights.Count)
//	{
//		break;
//	}
//}
