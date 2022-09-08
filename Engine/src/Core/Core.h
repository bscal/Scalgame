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
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif // SCAL_BUILD_DLL
#else
#define SAPI
#endif // SCAL_PLATFORM_WINDOWS

#define Kilobytes(n) (n * 1024)
#define Megabytes(n) (Kilobytes(n) * 1024)
#define Gigabytes(n) (Megabytes(n) * 1024)

struct Vector2i
{
	int x;
	int y;
};

struct Vector2ui
{
	uint32_t x;
	uint32_t y;
};
