#pragma once

#include "raylib/src/raylib.h"

#include <stdint.h>

static_assert(sizeof(size_t) == sizeof(uint64_t), 
	"ScalEngine does not support 32bit");

#define internal static
#define local_var static
#define global_var static

typedef int bool32;

constexpr global_var float TAO = (float)PI * 2.0f;

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
#define BitMask(state, mask) ((state & mask) == mask)

inline void ReportAssertFailure(const char* expression, const char* msg, const char* file, int line);

#if SCAL_DEBUG
#define SLOG_DEBUG(msg, ...) TraceLog(LOG_DEBUG, msg, __VA_ARGS__)
#define SLOG_WARN(msg, ...) TraceLog(LOG_WARNING, msg, __VA_ARGS__)
#define SASSERT(expr) if (!(expr)) { ReportAssertFailure(#expr, "", __FILE__, __LINE__); }
#define SASSERT_MSG(expr, msg) if (!(expr)) { ReportAssertFailure(#expr, msg, __FILE__, __LINE__); }

#if SCAL_PLATFORM_WINDOWS
#include <intrin.h>
#define DEBUG_BREAK(void) __debugbreak()
#else
#define DEBUG_BREAK(void) __builtin_trap()
#endif

#else
#define SLOG_DEBUG(msg, ...)
#define SLOG_WARN(msg, ...)
#define SASSERT(expression, msg, ...)
#define SASSERT_MSG(expr, msg)
#define DEBUG_BREAK(void)
#endif

#define SLOG_INFO(msg, ...) TraceLog(LOG_INFO, msg, __VA_ARGS__)
#define SLOG_ERR(msg, ...) TraceLog(LOG_ERROR, msg, __VA_ARGS__)

#define SFATAL_CRASH(msg, ...) \
	TraceLog(LOG_FATAL, msg, __VA_ARGS__); \
	TraceLog(LOG_FATAL, "Fatal error detected, program crashed! File: %s, Line: %s", __FILE__, __LINE__) \
	exit(1); \

#define CALL_CONSTRUCTOR(object) new (object)

struct MemorySizeData
{
	float Size;
	char BytePrefix;
};

MemorySizeData FindMemSize(uint64_t size);

inline void 
ReportAssertFailure(const char* expression, const char* msg, const char* file, int line)
{
	TraceLog(LOG_FATAL, "Assertion Failure: %s,"
		"Message: % s\n  File : % s, Line : % d\n",
		expression, msg, file, line);
	DEBUG_BREAK();
}
