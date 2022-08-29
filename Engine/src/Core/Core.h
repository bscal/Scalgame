#pragma once

#include <stdint.h>

#define internal static
#define local_persist static
#define global_var static

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

// TODO move
struct Vector2i
{
	int X;
	int Y;
};

struct Vector2ui
{
	uint32_t X;
	uint32_t Y;
};