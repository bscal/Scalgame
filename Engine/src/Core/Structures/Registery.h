#pragma once

#include "Core/Core.h"
#include "Core/SString.h"

#include "SList.h"
#include "SHashMap.h"

constexpr global_var uint32_t REGISTRY_NOT_FOUND = UINT32_MAX;

template<typename T>
struct Registry
{
	SList<T> Values;
	SHashMap<SString, uint32_t> NameToIndex;

	uint32_t Register(SString name, const T* value)
	{
		uint32_t id = Values.Count;
		
		Values.Push(value);

		NameToIndex.Insert(&name, &id);

		return id;
	}
};
