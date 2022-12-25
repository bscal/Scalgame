#pragma once

#include "Core.h"

#include <vector>

struct GameApplication;
struct Game;
struct World;
struct Tile;

struct LightSource
{
	Vector2 Position;
	Color Color;
	float Intensity;
};

struct LightMap
{
	std::vector<LightSource> LightSources;
	std::vector<float> Solids;
	std::vector<Vector2i> QueryTiles;
	int Width;
	int Height;
	int PaddingWidth;
	int PaddingHeight;
	Rectangle LightMapBounds;
	RenderTexture2D Texture;

	void Initialize(Game* game, int paddingWidth, int paddingHeight);
	
	void Update(Game* game);
	
	void BuildLightMap(Game* game);

	void LateUpdate(World* world);
	void CastRays(World* world,
		const LightSource& light, int rayResolution);
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

