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

global_var int LightMapUniformCountId = -1;
global_var int LightMapUniformArrayId = -1;

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

void LightMap::Initialize(uint16_t width, uint16_t height)
{
	Width = width;
	Height = height;
	size_t Count = (size_t)width * (size_t)height;
	LightLevels = std::vector<Vector3>(Count,
		{ 1.0f, 1.0f, 1.0f });
	Solids = std::vector<float>(Count,
		0.f);

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
			GetSurroundingTilesRadius(world,
				light,
				Fill);

			CastRays(world, light, 12);
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
	float endAngle = Radian;
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
	return (pos.x > StartPos.x && pos.y > StartPos.y &&
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
	Scal::MemSet(Solids.data(), 0, sizeof(float) * Solids.size());

	int halfWidth = (int)(Width / 2);
	int halfHeight = (int)(Height / 2);
	//StartPos.x = pos.x - halfWidth;
	//StartPos.y = pos.y - halfHeight;
	//EndPos.x = pos.x + halfWidth;
	//EndPos.y = pos.y + halfHeight;
	StartPos.x = (int)roundf(GetGameApp()->Game->CurScreenRect.x / 16.);
	StartPos.y = (int)roundf(GetGameApp()->Game->CurScreenRect.y / 16.);
	EndPos.x = StartPos.x + (int)(Width);
	EndPos.y = StartPos.y + (int)(Height);
	LightMapBounds.x = (float)StartPos.x;
	LightMapBounds.y = (float)StartPos.y;
	LightMapBounds.width = (float)EndPos.x - (float)StartPos.x;
	LightMapBounds.height = (float)EndPos.y - (float)StartPos.x;
}

void LightMap::AddLight(const LightSource& light)
{
	LightSources.push_back(light);
	//Vector2i tilePos = Vec2fToVec2i(light.Position);
	//Vector2i offset = tilePos;
	//size_t index = offset.x + offset.y * GetGameApp()->Game->World.ChunkedTileMap.BoundsEnd.x;
	//// TODO strength
	//auto t = ChunkedTileMap::GetTile(&GetGameApp()->Game->World.ChunkedTileMap,
	//	tilePos.x, tilePos.y);
	//if (!t) return;
	//t->TileColor = light.Color;
	/*LightLevels[index].x = light.Color.r;
	LightLevels[index].y = light.Color.g;
	LightLevels[index].z = light.Color.b;*/
}

Color LightMap::GetLight(Vector2i pos) const
{
	const auto t = CTileMap::GetTile(
		&GetGameApp()->Game->World.ChunkedTileMap, pos);
	if (!t) return WHITE;
	return t->TileColor;

	//Vector2i offset = pos;
	//size_t index = offset.x + offset.y * GetGameApp()->Game->World.ChunkedTileMap.BoundsEnd.x;
	//const auto& vec3 = LightLevels[index];
	//Color c;
	//c.r = vec3.x;
	//c.g = vec3.y;
	//c.b = vec3.z;
	//c.a = 1.0f;
	//return c;
}

void SightMap::Initialize(uint16_t width, uint16_t height)
{
	Width = width;
	Height = height;
	Sights.reserve((size_t)width * (size_t)height);
}

bool SightMap::IsInBounds(Vector2i pos) const
{
	return (pos.x >= StartPos.x && pos.y >= StartPos.y &&
		pos.x <= EndPos.x && pos.y <= EndPos.y);
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