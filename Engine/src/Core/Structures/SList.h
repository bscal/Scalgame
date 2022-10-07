#pragma once

#include "Core/SMemory.h"

#include <assert.h>

#define SLIST_DEFAULT_CAPACITY 8;
#define SLIST_DEFAULT_RESIZE 2;

template<typename T>
struct SList
{
	T* Memory;
	uint64_t Capacity;
	uint64_t Length;
	uint64_t Stride;
	uint64_t ResizeIncrease;

	bool Initialize();
	bool InitializeEx(uint64_t capacity, uint64_t resizeIncrease);
	bool Free();
	void Resize();
	void EnsureCapacity(uint64_t capacity);

	void Push(const T* valueSrc);
	void PushAt(uint64_t index, const T* valueSrc);
	void PushAtFast(uint64_t index, const T* valueSrc);
	bool PushUnique(const T* valueSrc);
	bool PushAtUnique(uint64_t index, const T* valueSrc);
	bool PushAtFastUnique(uint64_t index, const T* valueSrc);
	void Pop(T* valueDest);
	void PopAt(uint64_t index, T* valueDest);
	void PopAtFast(uint64_t index, T* valueDest);
	void Remove();
	void RemoveAt(uint64_t index);
	void RemoveAtFast(uint64_t index);

	T PeekAt(uint64_t index) const;
	T* Last() const;

	T& operator[](size_t i) { return Memory[i]; }
	const T& operator[](size_t i) const { return Memory[i]; }

	bool Contains(const T* value) const;
	void Clear();
};

// ********************

template<typename T>
inline bool SList<T>::Initialize()
{
	if (Memory)
	{
		TraceLog(LOG_ERROR, "Memory already initialized!");
		assert(!Memory);
		return false;
	}
	Capacity = SLIST_DEFAULT_CAPACITY;
	Stride = sizeof(T);
	ResizeIncrease = SLIST_DEFAULT_RESIZE
	Memory = (T*)Scal::MemAllocZero(Capacity * Stride);
	return true;
}

template<typename T>
inline bool SList<T>::InitializeEx(uint64_t capacity, uint64_t resizeIncrease)
{
	if (Memory)
	{
		TraceLog(LOG_ERROR, "Memory already initialized!");
		assert(!Memory);
		return false;
	}
	if (capacity == 0) capacity = 1;
	if (resizeIncrease == 0) resizeIncrease = SLIST_DEFAULT_RESIZE;
	Capacity = capacity;
	Stride = sizeof(T);
	ResizeIncrease = resizeIncrease;
	Memory = (T*)Scal::MemAllocZero(Capacity * Stride);
	return true;
}

template<typename T>
inline bool SList<T>::Free()
{
	if (!Memory)
	{
		TraceLog(LOG_ERROR, "Memory was already freed!");
		assert(Memory);
		return false;
	}
	Length = 0;
	Scal::MemFree((void*)Memory);
	return true;
}

template<typename T>
inline void SList<T>::Resize()
{
	if (!Memory)
	{
		TraceLog(LOG_ERROR, "Memory was null");
		assert(Memory);
		return;
	}
	if (Capacity == 0) Capacity = 1;
	if (ResizeIncrease == 0) ResizeIncrease = SLIST_DEFAULT_RESIZE;
	Capacity *= ResizeIncrease;
	Memory = (T*)Scal::MemRealloc(Memory, Capacity * Stride);
	assert(Memory);
}

template<typename T>
inline void SList<T>::EnsureCapacity(uint64_t capacity)
{
	if (!Memory)
	{
		TraceLog(LOG_ERROR, "Memory was null");
		return;
	}

	if (capacity <= Capacity) 
		return;

	Capacity = capacity;
	Memory = (T*)Scal::MemRealloc(Memory, Capacity * Stride);
}

template<typename T>
inline void SList<T>::Push(const T* valueSrc)
{
	assert(Memory);
	assert(valueSrc);

	if (Length == Capacity)
	{
		Resize();
	}

	Memory[Length] = *valueSrc;
	++Length;
}

template<typename T>
inline void SList<T>::PushAt(uint64_t index, const T* valueSrc)
{
	if (Length == Capacity)
	{
		Resize();
	}

	if (index > Length) index = Length;

	if (index != Length)
	{
		uint64_t dstOffset = (index + 1) * Stride;
		uint64_t srcOffset = index * Stride;
		uint64_t lenDiff = Length - index;
		uint64_t moveSize = lenDiff * Stride;
		Scal::MemCopy(((char*)Memory) + dstOffset, ((char*)Memory) + srcOffset, moveSize);
	}
	Memory[index] = *valueSrc;
	++Length;
}

template<typename T>
inline void SList<T>::PushAtFast(uint64_t index, const T* valueSrc)
{
	if (Length == Capacity)
	{
		Resize();
	}

	if (index < 0) index = 0;
	else if (index > Length) index = Length;

	if (index != Length)
	{
		Memory[Length] = Memory[index];
	}
	Memory[index] = *valueSrc;
	++Length;
}

template<typename T>
inline bool SList<T>::PushUnique(const T* valueSrc)
{
	assert(Memory);
	assert(valueSrc);

	if (Contains(valueSrc))
		return false;

	Push(valueSrc);
	return true;
}

template<typename T>
inline bool SList<T>::PushAtUnique(uint64_t index, const T* valueSrc)
{
	assert(valueSrc);

	if (Contains(valueSrc))
		return false;

	PushAtUnique(index, valueSrc);
	return true;

}

template<typename T>
inline bool SList<T>::PushAtFastUnique(uint64_t index, const T* valueSrc)
{
	assert(valueSrc);

	if (Contains(valueSrc))
		return false;

	PushAtFast(index, valueSrc);
	return true;
}

template<typename T>
inline void SList<T>::Pop(T* valueDest)
{
	assert(valueDest);

	if (Length == 0) return;
	*valueDest = Memory[Length - 1];
	--Length;
}

template<typename T>
inline void SList<T>::PopAt(uint64_t index, T* valueDest)
{
	assert(valueDest);
	assert(Memory);

	if (Length == 0) return;
	if (index > Length) index = Length;
	*valueDest = Memory[index];
	if (index != Length)
	{
		uint64_t dstOffset = index * Stride;
		uint64_t srcOffset = (index + 1) * Stride;
		uint64_t lenDiff = Length - index;
		uint64_t moveSize = lenDiff * Stride;
		Scal::MemCopy(((char*)Memory) + dstOffset, ((char*)Memory) + srcOffset, moveSize);
	}
	--Length;
}

template<typename T>
inline void SList<T>::PopAtFast(uint64_t index, T* valueDest)
{
	assert(valueDest);
	assert(Memory);

	if (Length == 0) return;
	if (index > Length) index = Length;
	*valueDest = Memory[index];

	if (index != Length) Memory[index] = Memory[Length - 1];
	--Length;
}

template<typename T>
inline void SList<T>::Remove()
{
	assert(Memory);
	--Length;
}

template<typename T>
inline void SList<T>::RemoveAt(uint64_t index)
{
	assert(Memory);
	if (index != Length)
	{
		uint64_t dstOffset = index * Stride;
		uint64_t srcOffset = (index + 1) * Stride;
		uint64_t lenDiff = Length - index;
		uint64_t moveSize = lenDiff * Stride;
		Scal::MemCopy(((char*)Memory) + dstOffset, ((char*)Memory) + srcOffset, moveSize);
	}
	--Length;
}

template<typename T>
inline void SList<T>::RemoveAtFast(uint64_t index)
{
	assert(Memory);
	Memory[index] = Memory[Length];
	--Length;
}

template<typename T>
inline T SList<T>::PeekAt(uint64_t index) const
{
	assert(Memory);
	if (index > Length) index = Length;
	return Memory[index];
}

template<typename T>
inline T* SList<T>::Last() const
{
	assert(Memory);
	if (Length == 0) return nullptr;
	return &Memory[Length];
}

template<typename T>
inline bool SList<T>::Contains(const T* value) const
{
	assert(Memory);
	assert(value);

	for (int i = 0; i < Length; ++i)
	{
		if (Memory[i] == *value) return true;
	}
	return false;
}

template<typename T>
inline void SList<T>::Clear()
{
	Length = 0;
}


inline void Test()
{
	SList<int> list = {};
	list.Initialize();

	int i;
	i = 1;
	list.Push(&i);
	i = 2;
	list.Push(&i);
	i = 3;
	list.Push(&i);
	i = 4;
	list.Push(&i);
	i = 5;
	list.Push(&i);
	i = 6;
	list.Push(&i);
	i = 7;
	list.Push(&i);
	i = 8;
	list.Push(&i);
	i = 9;
	list.Push(&i);
	i = 10;
	list.Push(&i);

	assert(list.Length == 10);
	assert(list.Capacity == 16);

	i = 5;
	bool u = list.PushAtUnique(5, &i);

	assert(!u);

	i = 0;
	list.PushAt(0, &i);

	int peek0 = list.PeekAt(1);

	i = 999;
	list.PushAtFast(1, &i);

	int peek = list.PeekAt(1);

	assert(peek == 999);

	int peek2 = list.PeekAt(2);
	
	assert(peek2 == 2);

	int pop = 0;
	list.Pop(&pop);

	assert(pop == 1);

	pop = 0;
	list.Pop(&pop);

	assert(pop == 10);

	list.RemoveAt(0);

	list.PopAt(0, &pop);
	
	assert(pop == 999);

	list.PopAtFast(0, &pop);

	assert(pop == 2);

	list.PopAt(0, &pop);

	assert(pop == 9);

	int c = 9;
	bool contains = list.Contains(&c);

	assert(!contains);

	int c2 = 5;
	bool contains2 = list.Contains(&c2);

	assert(contains2);

	i = 5;
	list.PushAt(3, &i);
	i = 255;
	list.Push(&i);
	i = 255;
	list.PushAt(3, &i);

	assert(list.Memory[4] = 5);
	assert(list.Memory[list.Length - 1] = 255);
	assert(list.Memory[3] = 255);

	list.Free();
}