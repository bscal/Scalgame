#pragma once

#include "STable.h"
#include "Core/Core.h"

template<typename T>
struct SSet
{
	STable<T, bool> Keys;
};

template<typename T>
void SSetInitialize(SSet<T>* set, uint64_t capacity)
{
	STableCreate(set->Keys, capacity);
}

template<typename T>
void SSetPut(SSet<T>* set, const T* key)
{
	STablePut(set, key, true);
}

template<typename T>
void SSetRemove(SSet<T>* set, const T* key)
{
	STablePut(set, key, false);
}

template<typename T>
bool SSetContains(SSet<T>* set, const T* key)
{
	return STableContains(set, key);
}