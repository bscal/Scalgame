#include "MapGeneration.h"

#include "Core/Globals.h"
#include "Core/Game.h"
#include "Core/ChunkedTileMap.h"

void MapGenInitialize(MapGenerator* generator, int seed)
{
	generator->Noise.SetSeed(seed);
	generator->Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

global_var const uint16_t TILE_IDS[] = { 0, 1, 2, 3, 4 };

void MapGenGenerateChunk(MapGenerator* generator, ChunkedTileMap* tilemap, TileMapChunk* chunk)
{
	for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
	{
		for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
		{
			int index = x + y * CHUNK_DIMENSIONS;
			SASSERT(index >= 0);
			SASSERT(index < CHUNK_SIZE);
			chunk->Tiles[index].HasCeiling = false;
			chunk->Tiles[index].LOS = TileLOS::NoVision;

			float worldX = (float)x + chunk->ChunkCoord.x * CHUNK_DIMENSIONS;
			float worldY = (float)y + chunk->ChunkCoord.y * CHUNK_DIMENSIONS;
			float noise = generator->Noise.GetNoise(worldX, worldY);
			int tileTableIndex = (int)(((noise + 1.f) / 2.f) * ArrayLength(TILE_IDS));
			if (tileTableIndex == ArrayLength(TILE_IDS)) tileTableIndex = ArrayLength(TILE_IDS) - 1;
			chunk->Tiles[index].TileId = GetGame()->TileMgr.Tiles[tileTableIndex].TileId;
		}
	}
}