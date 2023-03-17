#pragma once

#include "Core.h"
#include "Vector2i.h"
#include "Tools/Profiling.h"

global_var constexpr float TAO = static_cast<float>(PI) * 2.0f;

// TODO: should probably move resolution to another place
global_var constexpr int SCREEN_WIDTH = 1600;
global_var constexpr int SCREEN_HEIGHT = 900;

global_var constexpr int TILE_SIZE = 16;
global_var constexpr float INVERSE_TILE_SIZE = 1.0f / TILE_SIZE;
global_var constexpr float TILE_SIZE_F = static_cast<float>(TILE_SIZE);
global_var constexpr float HALF_TILE_SIZE = TILE_SIZE_F / 2.0f;

global_var constexpr int SCREEN_WIDTH_TILES = SCREEN_WIDTH / TILE_SIZE;
global_var constexpr int SCREEN_HEIGHT_TILES = SCREEN_HEIGHT / TILE_SIZE;
global_var constexpr Vector2 SCREEN_CENTER = { (float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f };

global_var constexpr int CULL_PADDING_TOTAL_TILES = 4;
global_var constexpr float CULL_PADDING_EDGE_PIXELS = 2 * TILE_SIZE_F;
global_var constexpr int CULL_WIDTH_TILES = (SCREEN_WIDTH / TILE_SIZE) + CULL_PADDING_TOTAL_TILES;
global_var constexpr int CULL_HEIGHT_TILES = (SCREEN_HEIGHT / TILE_SIZE) + CULL_PADDING_TOTAL_TILES;
global_var constexpr int CULL_WIDTH = CULL_WIDTH_TILES * TILE_SIZE;
global_var constexpr int CULL_HEIGHT = CULL_HEIGHT_TILES * TILE_SIZE;
global_var constexpr int CULL_TOTAL_TILES = CULL_WIDTH_TILES * CULL_HEIGHT_TILES;

global_var constexpr int CHUNK_DIMENSIONS = 64;
global_var constexpr int CHUNK_SIZE = CHUNK_DIMENSIONS * CHUNK_DIMENSIONS;

// TODO: move to settings struct?
global_var constexpr Vector2i VIEW_DISTANCE = { 2, 2 };


namespace Colors
{
global_var constexpr Vector4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
global_var constexpr Vector4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
global_var constexpr Vector4 Clear = { 0.0f, 0.0f, 0.0f, 0.0f };
}

#define FMT_VEC2(v) TextFormat("Vector2(x: %.3f, y: %.3f)", v.x, v.y)
#define FMT_VEC2I(v) TextFormat("Vector2i(x: %d, y: %d)", v.x, v.y)
#define FMT_RECT(rect) TextFormat("Rectangle(x: %.3f, y: %.3f, w: %.3f, h: %.3f)", rect.x, rect.y, rect.width, rect.height)
#define FMT_BOOL(boolVar) TextFormat("%s", ((boolVar)) ? "true" : "false")