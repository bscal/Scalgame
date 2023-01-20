#pragma once

#include "STable.h"
#include "Core/Core.h"

template<typename T>
struct SSet
{
	STable<T, bool> Keys;

	inline void Put(const T* key)
	{
		Keys.Put(key, true);
	}

	inline void Remove(const T* key)
	{
		Keys.Remove(key);
	}

	inline bool Contains(const T* key) const
	{
		return Keys.Contains(key);
	}
};
