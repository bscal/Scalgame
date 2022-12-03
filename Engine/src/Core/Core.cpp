#include "Core.h"

#include <assert.h>

void SCrash(const char* file, int line)
{
	TraceLog(LOG_FATAL, "Attempting to break! File: %s. Line: %d");
	assert(false);
}

MemorySizeData FindMemSize(uint64_t size)
{
	const uint64_t gb = 1024 * 1024 * 1024;
	const uint64_t mb = 1024 * 1024;
	const uint64_t kb = 1024;

	if (size > gb)
		return { (float)size / (float)gb, 'G' };
	else if (size > mb)
		return { (float)size / (float)mb, 'M' };
	else if (size > kb)
		return { (float)size / (float)kb, 'K' };
	else
		return { (float)size, ' ' };
}
