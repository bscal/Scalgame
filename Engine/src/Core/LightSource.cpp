#include "LightSource.h"

#include "Game.h"
#include "World.h"
#include "Player.h"
#include "ResourceManager.h"
#include "ChunkedTileMap.h"
#include "TileMapUtils.h"
#include "raymath.h"
#include "Tile.h"

#define LIGHT_MAP_UNIFORM_COUNT "LightMapCount"
#define LIGHT_MAP_UNIFORM "LightMap"

static int LightMapUniformCountId = -1;
static int LightMapUniformArrayId = -1;

void LightingInitialize(GameApplication* gameApp, LightMap* lightMap)
{
	//LightMapUniformArrayId = GetShaderLocation(
	//	gameApp->Resources->TileShader, LIGHT_MAP_UNIFORM_COUNT);
	//LightMapUniformArrayId = GetShaderLocation(
	//	gameApp->Resources->TileShader, LIGHT_MAP_UNIFORM);
}

void UpdateLightMap(Shader* shader, LightMap* lightMap)
{
	//if (LightMapUniformCountId == -1) return;

	int size = static_cast<int>(lightMap->LightLevels.size());

	//SetShaderValue(*shader,
	//	LightMapUniformCountId,
	//	&size,
	//	SHADER_UNIFORM_INT);

	//SetShaderValueV(*shader, 
	//	LightMapUniformArrayId,
	//	lightMap->LightLevels.data(),
	//	SHADER_UNIFORM_FLOAT,
	//	size);
}

void LightMap::Initialize(int paddingWidth, int paddingHeight)
{
	Vector2i screenDims
	{
		GetRenderWidth() / 16,
		GetRenderHeight() / 16
	};

	PaddingWidth = paddingWidth;
	PaddingHeight = paddingHeight;

	Width = screenDims.x + PaddingWidth * 2;
	Height = screenDims.y + PaddingHeight * 2;

	size_t Count = (size_t)Width * (size_t)Height;

	LightLevels = std::vector<Vector3>(Count,
		{ 1.0f, 1.0f, 1.0f });
	Solids = std::vector<float>(Count,
		0.f);

	Texture = LoadRenderTexture(screenDims.x, screenDims.y);

	LightSource light = {};
	light.Position = { 32, 32 };
	light.Color = RED;
	light.Intensity = 16.0f;
	AddLight(light);
}

internal void Fill(World* world, int x, int y,
	const LightSource& light)
{
	auto tile = CTileMap::GetTile(&world->ChunkedTileMap,
		{ x, y });
	if (!tile) return;
	tile->TileColor = light.Color;
};

void LightMap::Update(World* world)
{
	for (int i = 0; i < LightSources.size(); ++i)
	{
		const auto& light = LightSources[i];
		if (CheckCollisionPointRec(light.Position, LightMapBounds))
		{
			//GetSurroundingTilesRadius(world,
				//light,
				//Fill);

			//CastRays(world, light, 12);
		}
	}
}

void LightMap::LateUpdate(World* world)
{
	for (int i = 0; i < LightSources.size(); ++i)
	{
		const auto& light = LightSources[i];
		if (CheckCollisionPointRec(light.Position, LightMapBounds))
		{
			CastRays(world, light, 48);
		}
	}
	DrawRectangleLinesEx(LightMapBounds, 4.0f, MAGENTA);
}

void LightMap::BuildLightMap(Game* game)
{	
	//Vector2i screenCoord = GetClientPlayer()->Transform.TilePos;
	//Vector2i tileSize = game->World.ChunkedTileMap.TileSize;
	//float offsetX = game->WorldCamera.offset.x / (float)tileSize.x;
	//float offsetY = game->WorldCamera.offset.y / (float)tileSize.y;
	//int screenX = screenCoord.x - (int)offsetX;
	//int screenY = screenCoord.y - (int)offsetY;
	//int screenW = screenX + Width - PaddingWidth * 2;
	//int screenH = screenY + Height - PaddingHeight * 2;
	//int startX = screenX - PaddingWidth;
	//int startY = screenY - PaddingHeight;
	//int endX = startX + Width;
	//int endY = startY + Height;
	Vector2i tileSize = game->World.ChunkedTileMap.TileSize;
	Vector2i screenCoord = GetClientPlayer()->Transform.TilePos;
	Vector2i screenDims
	{
		GetRenderWidth() / tileSize.x,
		GetRenderHeight() / tileSize.y
	};
	float offsetX = (game->WorldCamera.offset.x) / (float)tileSize.x;
	float offsetY = (game->WorldCamera.offset.y) / (float)tileSize.y;
	for (int y = 0; y < screenDims.y; ++y)
	{
		for (int x = 0; x < screenDims.x; ++x)
		{
			TileCoord coord = {
				x + (int)((float)screenCoord.x - offsetX),
				y + (int)((float)screenCoord.y - offsetY)
			};
			if (!IsTileInBounds(&game->World.ChunkedTileMap, coord)) continue;
			const auto& tile = CTileMap::GetTileRef(&game->World.ChunkedTileMap, coord);
			const auto& tileData = tile.GetTileData(&game->World.TileMgr);
			Color c;
			if (tileData.Type == TileType::Solid)
				c = RED;
			else
				c = {};
			DrawPixel(x, y, c);
		}
	}
}

internal bool LightVisit(World* world, int x, int y)
{
	return true;
}

void LightMap::CastRays(World* world, 
	const LightSource& light, int rayResolution)
{
	float x = light.Position.x * 16.0f;
	float y = light.Position.y * 16.0f;
	float angle = 0.0f;
	float distance = light.Intensity * 16.0f;

	float startAngle = 0.0f;
	float endAngle = TAO;
	for (int i = 0; i < rayResolution; ++i)
	{
		float t = Lerp(startAngle, endAngle,
			(float)i / (float)rayResolution - 1);
		float x0 = cosf(angle + t);
		float y0 = sinf(angle + t);
		//float x1 = cosf(playerAngleRadians - t);
		//float y1 = sinf(playerAngleRadians - t);

		float xPos0 = x + x0 * distance;
		float yPos0 = y + y0 * distance;
		//float xPos1 = x + x1 * distance;
		//float yPos1 = y + y1 * distance;

		//Raytrace2DInt(world, (int)x, (int)y,
		//	(int)xPos0, (int)yPos0, LightVisit);
		//Raytrace2DInt(world, (int)x, (int)y,
		//	(int)xPos1, (int)yPos1, OnVisitTile);

		DrawLineEx({ x, y }, { xPos0, yPos0 }, 2.0f, PINK);
		//DrawLineEx({ x, y }, { xPos1, yPos1 }, 2.0f, PINK);
	}
}

bool LightMap::IsInBounds(Vector2i pos) const
{
	return (pos.x >= StartPos.x && pos.y >= StartPos.y &&
		pos.x < EndPos.x && pos.y < EndPos.y);
}

void LightMap::UpdateTile(World* world, Vector2i pos, const Tile* tile)
{
	if (IsInBounds(pos))
	{
		uint64_t index = CTileMap::TileToIndexLocal(StartPos,
			(uint64_t)Width, pos);
		const auto& tileData = tile->GetTileData(&world->TileMgr);
		Solids[index] = (tileData.TileId > 1) ? 1.0f : 0.f;
	}
}

void LightMap::UpdatePositions(Vector2i pos)
{
	const Rectangle& screenRect = GetGameApp()->Game->CurScreenRect;

	int screenWidthTiles = (int)(screenRect.width / 16.0f);
	int screenHeightTiles = (int)(screenRect.height / 16.0f);
	
	if (Width != screenWidthTiles ||
		Height != screenHeightTiles)
	{
		Width = screenWidthTiles;
		Height = screenHeightTiles;
		Solids.resize(Width * Height);

		UnloadRenderTexture(Texture);
		WaitTime(.1);
		Texture = LoadRenderTexture(Width, Height);
	}
	Scal::MemSet(Solids.data(), 0, sizeof(float) * Solids.size());

	int screenXTiles = (int)(screenRect.x / 16.0f);
	int screenYTiles = (int)(screenRect.y / 16.0f);
	StartPos.x = screenXTiles;
	StartPos.y = screenYTiles;
	EndPos.x = screenXTiles + screenWidthTiles;
	EndPos.y = screenYTiles + screenHeightTiles;

	constexpr float ratio = 0.2f;
	float expandWidth = Width * ratio;
	float expandHeight = Height * ratio;
	LightMapBounds.x = screenRect.x - expandWidth;
	LightMapBounds.y = screenRect.y - expandHeight;
	LightMapBounds.width = screenRect.width + expandWidth;
	LightMapBounds.height = screenRect.height + expandHeight;
}

void LightMap::AddLight(const LightSource& light)
{
	LightSources.push_back(light);
}

Color LightMap::GetLight(Vector2i pos) const
{
	const auto t = CTileMap::GetTile(
		&GetGameApp()->Game->World.ChunkedTileMap, pos);
	if (!t) return WHITE;
	return t->TileColor;
}

void SightMap::Initialize(uint16_t width, uint16_t height)
{
	Width = width;
	Height = height;
	Sights.reserve((size_t)width * (size_t)height);
}

bool SightMap::IsInBounds(Vector2i pos) const
{
	return (pos.x > StartPos.x && pos.y > StartPos.y &&
		pos.x < EndPos.x && pos.y < EndPos.y);
}

void SightMap::Update(World* world, Vector2i pos)
{
	int halfWidth = (int)(Width / 2);
	int halfHeight = (int)(Height / 2);
	StartPos.x = pos.x - halfWidth;
	StartPos.y = pos.y - halfHeight;
	EndPos.x = pos.x + halfWidth;
	EndPos.y = pos.y + halfHeight;

	auto player = world->EntityMgr.Players[0];
	float playerAngleRadians =
		GetRadiansFromDirection(player.LookDirection);
	constexpr float coneFov = 160.0f * DEG2RAD;
	float x = player.Transform.Pos.x + 8.0f;
	float y = player.Transform.Pos.y + 8.0f;
	constexpr float distance = 16.0f * 16.0f;

	GetTilesInCone(world, playerAngleRadians,
		coneFov, 16, x, y, distance);

	//uint64_t index = 0;
	//for (uint64_t y = StartPos.y; y < EndPos.y; ++y)
	//{
	//	for (uint64_t x = StartPos.x; x < EndPos.x; ++x)
	//	{
	//		

	//		++index;
	//	}
	//}
}

void SightMap::AddSight(Vector2i pos, float distance)
{
	Vector2i offset = pos.Subtract(StartPos);
	size_t index = offset.x + offset.y * Width;
	Sights[index] = distance;
}

bool SightMap::HasSight(Vector2i pos) const
{
	Vector2i offset = pos.Subtract(StartPos);
	size_t index = offset.x + offset.y * Width;
	return (Sights[index] != 0.0f);
}