#pragma once

#include "DebugWindow.h"
#include "Vector2i.h"

#include <raylib/src/raylib.h>
#include <stdint.h>

static_assert(sizeof(size_t) == sizeof(uint64_t), 
	"ScalEngine does not support 32bit");

#define Mode3D false
#define internal static
#define local_persist static
#define global_var static

typedef int bool32;

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
#define S_LOG_WARN(msg, ...) TraceLog(LOG_WARNING, msg, __VA_ARGS__)
#define S_LOG_ERR(msg, ...) TraceLog(LOG_ERROR, msg, __VA_ARGS__)

void SCrash(const char* file, int line);
#define S_CRASH(msg, ...) \
TraceLog(LOG_FATAL, msg, __VA_ARGS__); \
SCrash(__FILE__, __LINE__) \

#define CALL_CONSTRUCTOR(object) new (object)

struct MemorySizeData
{
	float Size;
	char BytePrefix;
};

MemorySizeData FindMemSize(uint64_t size);
