#include "Scheduler.h"

#include "ChunkedTileMap.h"

void DistributedTileUpdater::Update(ChunkedTileMap* tilemap, TileMapChunk* chunk, float dt)
{
	float updatesPerFrame = UPDATES_PER_SEC * dt;

	UpdateAccumulator = fminf(UpdateAccumulator + updatesPerFrame, (float)COUNT);

	int updateCount = (int)UpdateAccumulator;

	for (int i = 0; i < updateCount; ++i)
	{
		TileData data = chunk->Tiles[Index];
		OnUpdate onUpdateCB = data.GetTile()->OnUpdateCB;
		if (onUpdateCB)
		{
			onUpdateCB(Index, data);
		}

		Index = (Index + 1) % COUNT;
	}

	UpdateAccumulator -= (float)updateCount;
}
