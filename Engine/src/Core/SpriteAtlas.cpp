#include "SpriteAtlas.h"
#include "SHash.hpp"

#undef internal

#include <assert.h>
#include <sstream>

#include "SMemory.h"

bool SpriteAtlas::Load(const char* atlasDataPath,
	uint64_t estimatedSprites)
{
	if (!FileExists(atlasDataPath))
		return false;

	SpritesArray.InitializeCap(estimatedSprites);
	SpritesByName.reserve(estimatedSprites);

	// Info for sprite
	AtlasInfo tileInfo = {};
	// Once we found texture img no reason to look (1st line)
	bool foundTextureName = false;
	// Basically used to see if we are look at tiles and not header info
	bool isInTile = false;

	// Loads .atlas file and reads each line
	char* data = LoadFileText(atlasDataPath);
	std::istringstream input;
	input.str(data);
	for (std::string line; std::getline(input, line); )
	{
		if (line.empty()) continue;
		if (!foundTextureName) // Texture name is the 1st entry
		{
			foundTextureName = true;
			TextureName = line;
			continue;
		}

		if (line.substr(0, 2) == "  ") // Are we looking at tile data
		{
			if (line.substr(2, 3) == "xy:") // XY
			{
				auto found2 = line.find(':');
				if (found2 != std::string::npos) // split at :
				{
					const auto& xy = line.substr(found2 + 1, line.size());
					auto found3 = xy.find(',');
					if (found3 != std::string::npos) // split coords at ,
					{
						const auto& x = xy.substr(0, found3);
						const auto& y = xy.substr(found3+1, line.size());
						tileInfo.X = std::stoi(x);
						tileInfo.Y = std::stoi(y);

					};
				};
			}
			else if (line.substr(2, 5) == "size:") // Size
			{
				auto found2 = line.find(':');
				if (found2 != std::string::npos) // split at :
				{
					const auto& xy = line.substr(found2+1, line.size());
					auto found3 = xy.find(',');
					if (found3 != std::string::npos) // split coords at ,
					{
						const auto& x = xy.substr(0, found3);
						const auto& y = xy.substr(found3+1, line.size());
						tileInfo.TileW = std::stoi(x);
						tileInfo.TileH = std::stoi(y);

					};
				};
			}
			continue;
		}
		else
		{
			auto found = line.find(':'); 
			if (found == std::string::npos) // Reading tile info, not header info
			{
				if (isInTile) // Constructs current tile
				{
					Rectangle rect;
					rect.x = (float)tileInfo.X;
					rect.y = (float)tileInfo.Y;
					rect.width = (float)tileInfo.TileW;
					rect.height = (float)tileInfo.TileH;
					SpritesArray.Push(&rect);
					TraceLog(LOG_INFO,
						"Tile: %d, x: %d, y: %d, w: %d, h: %d",
						Size(),
						(int)rect.x, (int)rect.y,
						(int)rect.width, (int)rect.height);
				}
				// Inserts the next tile to be iterated
				SpritesByName.insert({ line, Size()});
				tileInfo = {};
				isInTile = true;
			}
		}
	}
	auto path = std::string("assets/textures/atlas/");
	path.append(TextureName);
	auto pathStr = path.c_str();

	assert(FileExists(pathStr));
	Texture = LoadTexture(pathStr);

	UnloadFileText(data);

	TraceLog(LOG_INFO, "Loaded atlas: %s, Texture: %s, Total Tiles: %d",
		atlasDataPath, pathStr, Size() - 1);
	IsLoaded = true;
	return true;
}

void SpriteAtlas::Unload()
{
	IsLoaded = false;
	SpritesArray.Free();
	UnloadTexture(Texture);
	// Other members should be freed by deconstructor
}

Rectangle SpriteAtlas::GetRectByName(std::string_view name) const
{
	auto find = SpritesByName.find({ name.data(), name.size() });
	SASSERT(find != SpritesByName.end());
	if (find != SpritesByName.end())
	{
		return SpritesArray[find->second];
	}
	return {};
}
