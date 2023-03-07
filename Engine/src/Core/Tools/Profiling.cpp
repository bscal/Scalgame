#include "Profiling.h"

#include "Core/SMemory.h"

#if ENABLE_PROFILING

global_var SpallProfile SpallCtx;
global_var SpallBuffer SpallData;

SpallProfile* GetProfileCtx() { return &SpallCtx; }
SpallBuffer* GetProfileBuffer() { return &SpallData; }

#endif

void InitProfile(const char* filename)
{
	#if ENABLE_PROFILING
	SpallCtx = spall_init_file(filename, 1);
	SpallData.length = Megabytes(1);
	SpallData.data = SMemAlloc(SpallData.length);
	bool spallBufferInit = spall_buffer_init(&SpallCtx, &SpallData);
	if (!spallBufferInit)
	{
		SLOG_ERR("Spall buffer failed to initialize");
		return;
	}
	else SLOG_INFO("[ PROFILING ] Spall profiling initialized");
	#endif
}

void ExitProfile()
{
	#if ENABLE_PROFILING
	spall_buffer_quit(&SpallCtx, &SpallData);
	spall_quit(&SpallCtx);
	#endif
}


