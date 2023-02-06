#pragma once

#include "Core/Core.h"

struct RegisterableType
{
	int UId;
};

template<typename T, uint16_t MaxEntries>
struct Registry
{
	uint16_t Size;
	T Values[MaxEntries];

	inline T* Register()
	{
		T* newEntry = &Values[Size];
		++Size;
		return newEntry;
	}
};