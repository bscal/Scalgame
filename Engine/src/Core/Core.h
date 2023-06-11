#pragma once

#include "raylib/src/raylib.h"

#include <stdint.h>

/*
* 
* -TODO
* -NOTE
* -COMMENT_THIS
* -FIXME
* 
*/

static_assert(sizeof(size_t) == sizeof(uint64_t), "ScalEngine does not support 32bit");

#define SCAL_ENABLE_PROFILING 0
#include "Tools/Profiling.h"

#define SCAL_GAME_TESTS 1

typedef int bool32;

#define internal static
#define local_var static
#define global_var static

#if defined(__clang__) || defined(__GNUC__)
#define _RESTRICT_ __restrict__
#elif defined(_MSC_VER)
#define _RESTRICT_ __restrict
#else
#define _RESTRICT_
#endif

#ifndef _ALWAYS_INLINE_
#if defined(__GNUC__)
#define _ALWAYS_INLINE_ __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define _ALWAYS_INLINE_ __forceinline
#else
#define _ALWAYS_INLINE_ inline
#endif
#endif

#ifndef _FORCE_INLINE_
#ifdef SCAL_DEBUG
#define _FORCE_INLINE_ inline
#else
#define _FORCE_INLINE_ _ALWAYS_INLINE_
#endif
#endif

#ifdef SCAL_PLATFORM_WINDOWS
#ifdef SCAL_BUILD_DLL
#define SAPI __declspec(dllexport)
#else
#define SAPI __declspec(dllimport)
#endif // SCAL_BUILD_DLL
#else
#define SAPI
#endif

#define ArrayLength(arr) (sizeof(arr) / sizeof(arr[0]))

#define Kilobytes(n) (n * 1024ULL)
#define Megabytes(n) (Kilobytes(n) * 1024ULL)
#define Gigabytes(n) (Megabytes(n) * 1024ULL)

#define BitGet(state, bit) ((state >> bit) & 1ULL)
#define BitSet(state, bit) (state | 1ULL << bit)
#define BitClear(state, bit) (state & ~(1ULL << bit))
#define BitToggle(state, bit) (state ^ 1ULL << bit)
#define BitMask(state, mask) ((state & mask) == mask)

#define Swap(x, y, T) T temp = x; x = y; y = temp

#if SCAL_DEBUG

#if _MSC_VER
	#define DEBUG_BREAK(void) __debugbreak()
#else
	#define DEBUG_BREAK(void) __builtin_trap()
#endif

#define SASSERT(expr) if (!(expr)) { TraceLog(LOG_ERROR, "Assertion Failure: %s\nMessage: % s\n  File : % s, Line : % d\n", #expr, "", __FILE__, __LINE__); DEBUG_BREAK(void); } 
#define SASSERT_MSG(expr, msg) if (!(expr)) { TraceLog(LOG_ERROR, "Assertion Failure: %s\nMessage: % s\n  File : % s, Line : % d\n", #expr, msg, __FILE__, __LINE__); DEBUG_BREAK(void); }

#define SLOG_DEBUG(msg, ...) TraceLog(LOG_DEBUG, msg, __VA_ARGS__)
#define SLOG_WARN(msg, ...) TraceLog(LOG_WARNING, msg, __VA_ARGS__)

#else
#define DEBUG_BREAK(void)
#define SASSERT(expr)
#define SASSERT_MSG(expr, msg)

#define SLOG_DEBUG(msg, ...)
#define SLOG_WARN(msg, ...)
#endif

#define SLOG_INFO(msg, ...) TraceLog(LOG_INFO, msg, __VA_ARGS__)
#define SLOG_ERR(msg, ...) TraceLog(LOG_ERROR, msg, __VA_ARGS__)

#define SFATAL(msg, ...) \
	TraceLog(LOG_ERROR, msg, __VA_ARGS__); \
	TraceLog(LOG_FATAL, "Fatal error detected, program crashed! File: %s, Line: %s", __FILE__, __LINE__) \

#define CALL_CONSTRUCTOR(object) new (object)

struct MemorySizeData
{
	float Size;
	char BytePrefix;
};

global_var constexpr float TAO = static_cast<float>(PI) * 2.0f;

global_var constexpr size_t SIZEOF_I64_BITS = (sizeof(uint64_t) * 8);

// TODO: should probably move resolution to another place
global_var constexpr int SCREEN_WIDTH = 1600;
global_var constexpr int SCREEN_HEIGHT = 900;

global_var constexpr int TILE_SIZE = 16;
global_var constexpr float TILE_SIZE_F = static_cast<float>(TILE_SIZE);
global_var constexpr float INVERSE_TILE_SIZE = 1.0f / TILE_SIZE_F;
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
global_var constexpr int VIEW_DISTANCE = 1;

// EntityId 
#define SetId(entity, id) (entity | (0x00ffffff & id))
#define SetGen(entity, gen) ((entity & 0x00ffffff) | ((uint8_t)gen << 24u))
#define GetId(entity) (entity & 0x00ffffff)
#define GetGen(entity) (uint8_t)((entity & 0xff000000) >> 24u)
#define IncGen(entity) (SetGen(entity, (GetGen(entity) + 1)))

namespace Colors
{
global_var constexpr Vector4 White = { 1.0f, 1.0f, 1.0f, 1.0f };
global_var constexpr Vector4 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
global_var constexpr Vector4 Clear = { 0.0f, 0.0f, 0.0f, 0.0f };
}

enum class TileDirection : uint8_t
{
	North,
	East,
	South,
	West
};

union UID
{
	struct
	{
		uint32_t Id : 24;
		uint32_t Gen : 8;
	};
	uint32_t Mask;
};

constexpr global_var float
TileDirectionToTurns[] = { TAO * 0.75f, 0.0f, TAO * 0.25f, TAO * 0.5f };

constexpr global_var Vector2
TileDirectionVectors[] = { { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } };

#define AngleFromTileDir(tileDirection) TileDirectionToTurns[(uint8_t)tileDirection]

#define FMT_VEC2(v) TextFormat("Vector2(x: %.3f, y: %.3f)", v.x, v.y)
#define FMT_VEC2I(v) TextFormat("Vector2i(x: %d, y: %d)", v.x, v.y)
#define FMT_RECT(rect) TextFormat("Rectangle(x: %.3f, y: %.3f, w: %.3f, h: %.3f)", rect.x, rect.y, rect.width, rect.height)
#define FMT_BOOL(boolVar) TextFormat("%s", ((boolVar)) ? "true" : "false")
#define FMT_ENTITY(ent) TextFormat("Entity(%u, Id: %u, Gen: %u", ent, GetId(ent), GetGen(ent))

inline MemorySizeData FindMemSize(uint64_t size);
inline double GetMicroTime();

inline MemorySizeData FindMemSize(uint64_t size)
{
	const uint64_t gb = 1024ull * 1024ull * 1024ull;
	const uint64_t mb = 1024ull * 1024ull;
	const uint64_t kb = 1024ull;

	if (size > gb)
		return { (float)size / (float)gb, 'G' };
	else if (size > mb)
		return { (float)size / (float)mb, 'M' };
	else if (size > kb)
		return { (float)size / (float)kb, 'K' };
	else
		return { (float)size, ' ' };
}

inline double GetMicroTime()
{
	return GetTime() * 1000000.0;
}