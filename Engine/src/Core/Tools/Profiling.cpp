#include "Profiling.h"

#if SCAL_ENABLE_PROFILING

#include "Core/SMemory.h"

global_var SpallProfile SpallCtx;

#define USE_THREAD 1
#if USE_THREAD 

#include <thread>

struct ThreadedProfiler
{
	SpallBuffer Buffer;
	uint32_t ThreadId;
	double Timer;

	ThreadedProfiler()
	{
		Buffer.length = Megabytes(1);
		Buffer.data = SMemAllocTag((uint8_t)SAllocator::Malloc, Buffer.length, MemoryTag::Profiling);
		SASSERT(Buffer.data);
		bool spallBufferInit = spall_buffer_init(&SpallCtx, &Buffer);
		SASSERT(spallBufferInit);

		std::thread::id id = std::this_thread::get_id();
		ThreadId = std::hash<std::thread::id>{}(id);

		Timer = 0.0f;
	}

	~ThreadedProfiler()
	{
		spall_buffer_quit(&SpallCtx, &Buffer);
		SMemFree(Buffer.data);
	}

};

thread_local ThreadedProfiler SpallData;

#else

global_var SpallBuffer SpallData;

#endif

void SpallBegin(const char* name, uint32_t len, double time)
{
#if USE_THREAD
	spall_buffer_begin_ex(&SpallCtx, &SpallData.Buffer, name, len, time, SpallData.ThreadId, 0);
	double curTime = GetTime();
	double timeDiff = curTime - SpallData.Timer;
	if (timeDiff >= 5.0)
	{
		SpallData.Timer = curTime;
		spall_buffer_flush(&SpallCtx, &SpallData.Buffer);
	}
#else
	spall_buffer_begin(&SpallCtx, &SpallData, name, len, time);
#endif
}

void SpallEnd(double time)
{
#if USE_THREAD
	spall_buffer_end_ex(&SpallCtx, &SpallData.Buffer, time, SpallData.ThreadId, 0);
#else
	spall_buffer_end(&SpallCtx, &SpallData, time);
#endif
}

#endif

void InitProfile(const char* filename)
{
#if SCAL_ENABLE_PROFILING
	SpallCtx = spall_init_file(filename, 1);
	
#if !USE_THREAD
	#define BUFFER_SIZE (64 * 1024 * 1024)
	void* buffer = SAlloc(SAllocator::Malloc, BUFFER_SIZE, MemoryTag::Profiling);
	memset(buffer, 1, BUFFER_SIZE);

	SpallData = {};
	SpallData.length = BUFFER_SIZE;
	SpallData.data = buffer;

	spall_buffer_init(&SpallCtx, &SpallData);
#endif

	SLOG_INFO("[ PROFILING ] Spall profiling initialized");
#endif
}

void ExitProfile()
{
#if SCAL_ENABLE_PROFILING
#if !USE_THREAD
	spall_buffer_quit(&SpallCtx, &SpallData);
#endif
	spall_quit(&SpallCtx);
#endif
}


