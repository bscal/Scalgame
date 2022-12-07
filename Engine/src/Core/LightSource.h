#pragma once

#include "Core.h"

#include <vector>

struct GameApplication;

struct LightSource
{
	Vector2 Position;
	Color Color;
	float Intensity;
};

struct LightMap
{
	std::vector<uint8_t> LightLevels;
	Vector2i StartPos;
	Vector2i EndPos;
	uint16_t Width;
	uint16_t Height;

	void Initialize(uint16_t width, uint16_t height);
	bool IsInBounds(Vector2i pos) const;
	void UpdatePositions(Vector2i pos);
	void UpdateLevel(Vector2i pos, uint8_t newLevel);
};

struct Lighting
{

};

void LightingInitialize(GameApplication* gameApp, LightMap* lightmap);

