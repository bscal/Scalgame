#include "Profiling.h"

#if ENABLE_PROFILING

#include "Core/SMemory.h"

#include <thread>

global_var SpallProfile SpallCtx;


struct ProfileBuffer
{
	SpallBuffer SpallData;
	int ThreadId;
	bool IsInitialized;

	inline void TryInit()
	{
		if (!IsInitialized)
		{

			IsInitialized = true;
			SpallData.length = Megabytes(1);
			SpallData.data = SMemAllocTag(SAllocator::Game, SpallData.length, MemoryTag::Profiling);
			bool spallBufferInit = spall_buffer_init(&SpallCtx, &SpallData);
			SASSERT(spallBufferInit);

			std::thread::id id = std::this_thread::get_id();
			std::hash<std::thread::id> hasher = {};
			ThreadId = hasher(id);
		}
	}

	~ProfileBuffer()
	{
		IsInitialized = false;
		spall_buffer_quit(&SpallCtx, &SpallData);
		SMemFreeTag(SAllocator::Game, SpallData.data, SpallData.length, MemoryTag::Profiling);
	}
};

global_var thread_local ProfileBuffer Buffer;


void SpallBegin(const char* name, uint32_t len, double time)
{
	Buffer.TryInit();

	spall_buffer_begin_ex(&SpallCtx, &Buffer.SpallData, name, len, time, Buffer.ThreadId, 0);
}

void SpallEnd(double time)
{
	spall_buffer_end_ex(&SpallCtx, &Buffer.SpallData, time, Buffer.ThreadId, 0);
}

#endif

void InitProfile(const char* filename)
{
	#if ENABLE_PROFILING
	SpallCtx = spall_init_file(filename, 1);
	SLOG_INFO("[ PROFILING ] Spall profiling initialized");
	#endif
}

void ExitProfile()
{
	#if ENABLE_PROFILING
	Buffer = {};
	spall_quit(&SpallCtx);
	#endif
}


