#pragma once

#include "Core/Structures/STable.h"

template<typename T>
struct SSet
{
	STable<T, bool> Keys;

	inline bool Put(const T* key)
	{
		const bool val = true;
		return Keys.Put(key, &val);
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
