#include "MapGeneration.h"

#include "Core/Game.h"
#include "Core/ChunkedTileMap.h"

void MapGenInitialize(MapGenerator* generator, int seed)
{
	generator->Noise.SetSeed(seed);
	generator->Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

global_var const TileSheetCoord TILE_IDS[] = { STONE_FLOOR, GOLD_ORE, DARK_STONE_FLOOR, ROCKY_WALL, LAVA_0 };

void MapGenGenerateChunk(MapGenerator* generator, ChunkedTileMap* tilemap, TileMapChunk* chunk)
{
	int idx = 0;
	for (int y = 0; y < CHUNK_DIMENSIONS; ++y)
	{
		for (int x = 0; x < CHUNK_DIMENSIONS; ++x)
		{
			float worldX = (float)x + (float)chunk->ChunkCoord.x * (float)CHUNK_DIMENSIONS;
			float worldY = (float)y + (float)chunk->ChunkCoord.y * (float)CHUNK_DIMENSIONS;

			float noise = generator->Noise.GetNoise(worldX, worldY);

			int tileTableIndex = (int)(((noise + 1.f) / 2.f) * ArrayLength(TILE_IDS));
			if (tileTableIndex == ArrayLength(TILE_IDS)) 
				tileTableIndex = ArrayLength(TILE_IDS) - 1;

			TileSheetCoord coord = TILE_IDS[tileTableIndex];
			chunk->Tiles[idx].TexX = coord.x;
			chunk->Tiles[idx].TexY = coord.y;
			chunk->Tiles[idx].HasCeiling = false;
			chunk->Tiles[idx].Ununsed = 0;
			++idx;
		}
	}
}