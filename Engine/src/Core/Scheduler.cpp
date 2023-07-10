#include "Scheduler.h"

#include "ChunkedTileMap.h"

void DistributedTileUpdater::Update(ChunkedTileMap* tilemap, float dt)
{
	float updatesPerSecond = (float)Actions.Size / Interval;
	float updatesPerFrame = updatesPerSecond * dt;

	UpdateAccumulator = fminf(UpdateAccumulator + updatesPerFrame, (float)Actions.Size);

	UpdateCount = UpdateAccumulator;

	for (int i = 0; i < UpdateCount; ++i)
	{
		TileUpdaterData* actionPtr = Actions.At(Index);
		if (actionPtr)
		{
			TileData* tileData = CTileMap::GetTile(tilemap, actionPtr->Pos);
			actionPtr->OnUpdate(actionPtr->Pos, *tileData);
		}

		Index = (Index + 1) % Actions.Size;
	}

	UpdateAccumulator -= (float)UpdateCount;
}

uint32_t DistributedTileUpdater::Add(Vector2i coord, TileData* data)
{
	SASSERT(data);

	if (data->GetTile()->OnUpdate)
	{
		TileUpdaterData entry;
		entry.Pos = coord;
		entry.OnUpdate = data->GetTile()->OnUpdate;

		return Actions.Add(&entry);
	}

	return UINT32_MAX;
}
