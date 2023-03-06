#include "Core.h"

void
ReportAssertFailure(const char* expression, const char* msg, const char* file, int line)
{
	TraceLog(LOG_ERROR, "Assertion Failure: %s\n"
		"  Message: % s\n  File : % s, Line : % d\n",
		expression, msg, file, line);
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

double GetMicroTime()
{
	return GetTime() * 1000000.0;
}
