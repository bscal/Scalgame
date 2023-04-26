#pragma once

#include "Core/Core.h"

#define ENABLE_PROFILING 0

#if ENABLE_PROFILING
#include "spall/spall.h"

void SpallBegin(const char* name, uint32_t len, double time);
void SpallEnd(double time);

#define PROFILE_BEGIN() SpallBegin(__FUNCTION__, sizeof(__FUNCTION__) - 1, GetMicroTime())
#define PROFILE_BEGIN_EX(str) SpallBegin(str, sizeof(str) - 1, GetMicroTime())
#define PROFILE_END() SpallEnd(GetMicroTime())

#else
#define PROFILE_BEGIN()
#define PROFILE_BEGIN_EX(str)
#define PROFILE_END()

#endif

void InitProfile(const char* filename);
void ExitProfile();
