#include "ThreadWin32.h"

#include <Windows.h>

#include <string>
#include <cassert>

void InitThread(void* handle, unsigned int threadID)
{
	// Do Windows-specific thread setup:
	HANDLE winHandle = (HANDLE)handle;

	// Put each thread on to dedicated core:
	DWORD_PTR affinityMask = 1ull << threadID;
	DWORD_PTR affinity_result = SetThreadAffinityMask(winHandle, affinityMask);
	assert(affinity_result > 0);

	//// Increase thread priority:
	//BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_HIGHEST);
	//assert(priority_result != 0);

	// Name the thread:
	std::wstring wthreadname = L"wi::jobsystem_" + std::to_wstring(threadID);
	HRESULT hr = SetThreadDescription(winHandle, wthreadname.c_str());
	assert(SUCCEEDED(hr));
}