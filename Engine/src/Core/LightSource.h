#pragma once

#include "Core.h"

#include <vector>

struct GameApplication;
struct World;

struct LightSource
{
	Vector2 Position;
	Color Color;
	float Intensity;
};

struct LightMap
{
	Rectangle LightMapBounds;
	std::vector<LightSource> LightSources;
	std::vector<Vector3> LightLevels;
	std::vector<Vector2i> QueryTiles;
	std::vector<float> Solids;
	Vector2i StartPos;
	Vector2i EndPos;
	uint16_t Width;
	uint16_t Height;

	void Update(World* world);
	void LateUpdate(World* world);
private:
	void CastRays(World* world,
		const LightSource& light, int rayResolution);
public:

	void Initialize(uint16_t width, uint16_t height);
	bool IsInBounds(Vector2i pos) const;
	void UpdatePositions(Vector2i pos);
	void AddLight(const LightSource& light);
	Color GetLight(Vector2i pos) const;
};

struct SightMap
{
	std::vector<float> Sights;
	Vector2i StartPos;
	Vector2i EndPos;
	uint16_t Width;
	uint16_t Height;

	void Initialize(uint16_t width, uint16_t height);
	bool IsInBounds(Vector2i pos) const;
	void Update(World* world, Vector2i pos);
	void AddSight(Vector2i pos, float distance);
	bool HasSight(Vector2i pos) const;
};

struct Lighting
{

};

void LightingInitialize(GameApplication* gameApp, LightMap* lightmap);

