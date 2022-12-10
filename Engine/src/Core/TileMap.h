//#pragma once
//
//#include "Engine.h"
//
//struct Game;
//struct SArray;
//
//// TODO Think about moving TileType into Tile structure
//
//#define NORTH { 0, -1 }
//#define EAST  { 1,  0 }
//#define SOUTH { 0,  1 }
//#define WEST  {-1,  0 }
//
//enum class TileVisibilty : uint8_t
//{
//	Empty = 0,
//	Solid
//};
//
//enum class TileType : uint8_t
//{
//	Empty = 0,
//	Solid,
//	Floor,
//	Liquid
//};
//
//struct TileData
//{
//	Vector2 TextureCoord;
//	short MovementCost;
//	TileVisibilty TileVisibilty;
//	TileType TileType;
//};
//
//struct TileSet
//{
//	Texture2D TileTexture;
//	TileData* TileDataArray;
//	uint32_t TextureTileWidth;
//	uint32_t TextureTileHeight;
//};
//
//enum class FOWLevel : uint8_t
//{
//	NoVision = 0,
//	PastVision,
//	PeripheralVision,
//	SemiVision,
//	FullVision
//};
//
//struct Tile
//{
//	Vector2 TexturePosition;
//	uint32_t TileId;
//	FOWLevel Fow;
//};
//
//struct TileMap
//{
//	TileSet* TileSet;
//	Tile* MapTiles;
//	uint64_t MapSize;
//	uint32_t MapWidth;
//	uint32_t MapHeight;
//	uint16_t MapTileSize;
//	uint16_t MapHalfTileSize;
//};

//bool LoadTileSet(Texture2D* tileTexture,
//	uint16_t tileSizeWidth, uint16_t tileSizeHeight,
//	TileSet* outTileSet);
//
//bool InitializeTileMap(TileSet* tileSet, 
//	uint32_t width, uint32_t height,
//	uint16_t tileSize, TileMap* outTileMap);
//
//void LoadTileMap(TileMap* tileMap);
//
//void UnloadTileMap(TileMap* tileMap);
//void RenderTileMap(Game* game, TileMap* tileMap);
//
//bool IsInBounds(int tileX, int tileY, int width, int height);
//
//Tile CreateTile(TileMap* tileMap, uint32_t tileId);
//Tile* GetTile(TileMap* tileMap, int tileX, int tileY);
//void SetTile(TileMap* tileMap, int tileX, int tileY, Tile* srcTile);
//TileData* GetTileData(TileMap* tileMap, uint32_t tileId);
//
//void GetSurroundingTilesBox(TileMap* tileMap,
//	int tileX, int tileY, int boxWidth, int boxHeight,
//	SArray* outTileCoordsVector2i);
//
//void GetSurroundingTilesRadius(TileMap* tileMap,
//	int tileX, int tileY, float radius,
//	SArray* outTileCoordsVector2i);
//
//void GetSurroundingTilesRadiusCallback(TileMap* tileMap,
//	float x, float y, float radius,
//	void (OnVisit)(TileMap* tileMap, int tileX, int tileY));
//
//void ProcessFOVSurroundingPos(TileMap* tileMap,
//	float x, float y, int rayResolution, float distance);
//
//void GetTilesInCone(TileMap* tileMap,
//	float playerAngleRadians, float coneFovRadians, int rayResolution,
//	float x, float y, float distance);
//
//float DegreesToRadians(float degrees);
//float TurnsToRadians(float turn);
//float Distance(float x0, float y0, float x1, float y1);
//int DistanceInTiles(int x0, int y0, int x1, int y1);
