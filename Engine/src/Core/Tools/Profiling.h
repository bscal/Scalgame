#pragma once

#include "Core/Core.h"

#define ENABLE_PROFILING 0

#if ENABLE_PROFILING
#include "spall/spall.h"

void SpallBegin(const char* name, uint32_t len, double time);
void SpallEnd(double time);

#define PROFILE_BEGIN(void) SpallBegin(__FUNCTION__, sizeof(__FUNCTION__) - 1, GetMicroTime())
#define PROFILE_BEGIN_EX(str) SpallBegin(str, sizeof(str) - 1, GetMicroTime())
#define PROFILE_END(void) SpallEnd(GetMicroTime())

#else
#define PROFILE_BEGIN(void)
#define PROFILE_BEGIN_EX(str)
#define PROFILE_END(void)

#endif

void InitProfile(const char* filename);
void ExitProfile();
