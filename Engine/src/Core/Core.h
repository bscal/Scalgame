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
};

Vector2i Vec2iAdd(Vector2i v0, Vector2i v1);

struct Vector2ui
{
	uint32_t x;
	uint32_t y;
};
