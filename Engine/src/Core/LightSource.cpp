#include "LightSource.h"

#include "Game.h"
#include "World.h"
#include "Player.h"
#include "ResourceManager.h"

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
	LightLevels.reserve((size_t)width * (size_t)height);
}

bool LightMap::IsInBounds(Vector2i pos) const
{
	return (pos.x >= StartPos.x && pos.y >= StartPos.y &&
		pos.x <= EndPos.x && pos.y <= EndPos.y);
}

void LightMap::UpdatePositions(Vector2i pos)
{
	int halfWidth = (int)(Width / 2);
	int halfHeight = (int)(Height / 2);
	StartPos.x = pos.x - halfWidth;
	StartPos.y = pos.y - halfHeight;
	EndPos.x = pos.x + halfWidth;
	EndPos.y = pos.y + halfHeight;
}

void LightMap::AddLight(Vector2i pos, Vector3 color, float strength)
{
	Vector2i offset = pos.Subtract(StartPos);
	size_t index = offset.x + offset.y * Width;
	// TODO strength
	LightLevels[index].x = 
		ClampF(0.0f, 1.0f, LightLevels[index].x + color.x);
	LightLevels[index].y =
		ClampF(0.0f, 1.0f, LightLevels[index].y + color.y);
	LightLevels[index].z =
		ClampF(0.0f, 1.0f, LightLevels[index].z + color.z);
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