#pragma once

#include "Core/Core.h"

#include "FastNoiseLite/FastNoiseLite.h"

struct ChunkedTileMap;
struct TileMapChunk;

struct TileGenData
{
	float Height;
	float Temperature;
	float Rainfall;
};

struct MapGenerator
{
	FastNoiseLite Noise;
};

void MapGenInitialize(MapGenerator* generator, int seed);

void MapGenGenerateChunk(MapGenerator* generator, ChunkedTileMap* tilemap, TileMapChunk* chunk);
