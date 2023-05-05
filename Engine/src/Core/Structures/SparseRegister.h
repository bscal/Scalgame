#pragma once

#include "Core/Core.h"

#include "SList.h"

template<typename T>
struct Registry
{

	SList<T> Values;

	uint32_t NextId;

	inline uint32_t Register(const T& value)
	{
		Values.EnsureSize(NextId);

		uint32_t id = NextId++;
		Values[id] = value;
		return id;
	}
};
