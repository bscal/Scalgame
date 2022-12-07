#include "LightSource.h"

#include "Game.h"
#include "ResourceManager.h"

#define LIGHT_MAP_UNIFORM_COUNT "LightMapCount"
#define LIGHT_MAP_UNIFORM "LightMap"

global_var int LightMapUniformCountId = -1;
global_var int LightMapUniformArrayId = -1;

void LightingInitialize(GameApplication* gameApp, LightMap* lightMap)
{
	LightMapUniformArrayId = GetShaderLocation(
		gameApp->Resources->TileShader, LIGHT_MAP_UNIFORM_COUNT);
	LightMapUniformArrayId = GetShaderLocation(
		gameApp->Resources->TileShader, LIGHT_MAP_UNIFORM);
}

void UpdateLightMap(Shader* shader, LightMap* lightMap)
{
	if (LightMapUniformCountId == -1) return;

	int size = static_cast<int>(lightMap->LightLevels.size());

	SetShaderValue(*shader,
		LightMapUniformCountId,
		&size,
		RL_SHADER_UNIFORM_INT);

	SetShaderValueV(*shader, 
		LightMapUniformArrayId,
		lightMap->LightLevels.data(),
		RL_SHADER_UNIFORM_INT,
		size);
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

void LightMap::UpdateLevel(Vector2i pos, uint8_t newLevel)
{
	Vector2i offset = pos.Subtract(StartPos);
	size_t index = offset.x + offset.y * Width;
	LightLevels[index] = newLevel;
}