#include "SpriteAtlas.h"

#undef internal

#include "SMemory.h"

#include <string>
#include <sstream>


bool SpriteAtlas::Load(const char* atlasDataPath,
	uint64_t estimatedSprites)
{
	if (!FileExists(atlasDataPath))
		return false;

	SpritesArray.Resize(estimatedSprites);
	SpritesByName.Reserve(estimatedSprites);

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

	// TODO eventually like to fully remove std::string
	std::string lineBuf;
	lineBuf.reserve(512);
	for (;std::getline(input, lineBuf); )
	{
		if (lineBuf.empty()) continue;
		if (!foundTextureName) // Texture name is the 1st entry
		{
			foundTextureName = true;
			TextureName = SString(lineBuf.c_str());
			continue;
		}

		std::string_view line = lineBuf;
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

						std::string strX(x);
						tileInfo.X = std::stoi(strX);
						std::string strY(y);
						tileInfo.Y = std::stoi(strY);

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
						std::string strX(x);
						tileInfo.TileW = std::stoi(strX);
						std::string strY(y);
						tileInfo.TileH = std::stoi(strY);

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
					SLOG_INFO("Tile: %s, Id: %d, x: %d, y: %d, w: %d, h: %d",
						lineBuf.c_str(),
						Size(),
						(int)rect.x, (int)rect.y,
						(int)rect.width, (int)rect.height);
				}
				// Inserts the next tile to be iterated
				SString name(lineBuf.c_str());
				uint32_t size = Size();
				SpritesByName.Put(&name, &size);
				tileInfo = {};
				isInTile = true;
			}
		}
	}

	const char* texturePath = 
		TextFormat("assets/textures/atlas/%s", TextureName.Data());
	SASSERT(FileExists(texturePath));
	Texture = LoadTexture(texturePath);

	UnloadFileText(data);

	SLOG_INFO("[ SpriteAtlas] Loaded %s, Texture: %s, Total Tiles: %d",
		atlasDataPath, texturePath, Size());
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

Rectangle SpriteAtlas::GetRectByName(SStringView tileName) const
{
	SString tempSStr = SString::CreateFake(&tileName);
	uint32_t* index = SpritesByName.Get(&tempSStr);
	if (index) return SpritesArray[*index];
	// TODO probably want some type of pink tile to be more ERRORY
	return {};
}
