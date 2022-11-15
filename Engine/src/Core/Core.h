#pragma once

#include "DebugWindow.h"

#include <raylib/src/raylib.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_var static

#define TAO ((float)PI * 2.0f)

#ifdef SCAL_PLATFORM_WINDOWS
#ifdef SCAL_BUILD_DLL
#define SAPI __declspec(dllexport)
#else
#define SAPI __declspec(dllimport)
#endif // SCAL_BUILD_DLL
#else
#define SAPI
#endif // SCAL_PLATFORM_WINDOWS

#define Kilobytes(n) (n * 1024)
#define Megabytes(n) (Kilobytes(n) * 1024)
#define Gigabytes(n) (Megabytes(n) * 1024)

#define BitGet(state, bit) ((state >> bit) & 1U)
#define BitSet(state, bit) (state | 1U << bit)
#define BitClear(state, bit) (state & ~(1U << bit))
#define BitToggle(state, bit) (state ^ 1U << bit)

#if SCAL_DEBUG
#define S_LOG_DEBUG(msg, ...) TraceLog(LOG_DEBUG, msg, __VA_ARGS__)
#else
#define S_LOG_DEBUG(msg, ...)
#endif

#define S_LOG_INFO(msg, ...) TraceLog(LOG_INFO, msg, __VA_ARGS__)
#define S_LOG_ERR(msg, ...) TraceLog(LOG_ERROR, msg, __VA_ARGS__)

internal void SCrash();
#define S_LOG_CRASH(msg, ...) TraceLog(LOG_FATAL, msg, __VA_ARGS__); \
SCrash() \

struct MemorySizeData
{
	float Size;
	char BytePrefix;
};

MemorySizeData FindMemSize(uint64_t size);

struct Vector2i
{
	int x;
	int y;

	bool Vector2i::operator == (const Vector2i& rhs) const;
	bool Vector2i::operator != (const Vector2i& rhs) const;

	int64_t ToInt64() const;
	static Vector2i FromInt64(int64_t src);
};

struct Vector2iu
{
	uint32_t x;
	uint32_t y;

	bool AreEquals(const Vector2iu o) const;
	uint64_t ToUInt64() const;

	static Vector2iu FromUInt64(uint64_t src);
};

struct PackVector2i
{
	size_t operator()(const Vector2i& v) const
	{
		size_t packedVec = ((size_t(v.x) & 0xffL) << 32);
		packedVec |= (v.y & 0xffL);
		return packedVec;
	}
};

inline int64_t PackVec2i(Vector2i v)
{
	size_t packedVec = ((size_t(v.x) & 0xffL) << 32);
	packedVec |= (v.y & 0xffL);
	return packedVec;
}

Vector2i Vec2iFromInt64(int64_t packedVec2i);

Vector2i Vec2iAdd(Vector2i v0, Vector2i v1);
