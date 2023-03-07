#pragma once

#include "Core/Core.h"

#define ENABLE_PROFILING 1

#if ENABLE_PROFILING
#include "spall/spall.h"

SpallProfile* GetProfileCtx();
SpallBuffer* GetProfileBuffer();

#define PROFILE_BEGIN(void) spall_buffer_begin(GetProfileCtx(), GetProfileBuffer(), __FUNCTION__, sizeof(__FUNCTION__) - 1, GetMicroTime())
#define PROFILE_BEGIN_EX(str) spall_buffer_begin(GetProfileCtx(), GetProfileBuffer(), str, sizeof(str) - 1, GetMicroTime())
#define PROFILE_END(void) spall_buffer_end(GetProfileCtx(), GetProfileBuffer(), GetMicroTime())

#else
#define PROFILE_BEGIN(void)
#define PROFILE_BEGIN_EX(str)
#define PROFILE_END(void)
#endif

void InitProfile(const char* filename);
void ExitProfile();
