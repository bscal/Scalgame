#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

#define SLIST_DEFAULT_CAPACITY 1
#define SLIST_DEFAULT_RESIZE 2

template<typename T>
struct SList
{
	T* Memory;
	uint64_t Capacity;
	uint64_t Length;

	bool Initialize();
	bool InitializeCap(uint64_t capacity);

	bool Free();
	void Resize(uint64_t capacity);

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
	void Set(uint64_t index, const T* valueSrc);

	const T& PeekAt(uint64_t index) const;
	T* PeekAtPtr(uint64_t index) const;
	T* Last() const;

	inline const T& operator[](size_t i) const
	{
		return Memory[i];
	}

	bool Contains(const T* value) const;
	void Clear();

	inline uint64_t End() const;
	inline size_t Stride() const;
	inline size_t MemSize() const;
	inline bool IsInitialized() const;
};

// ********************

template<typename T>
bool SList<T>::Initialize()
{
	if (Memory)
	{
		S_CRASH("Memory already initialized!");
		return false;
	}
	Capacity = SLIST_DEFAULT_CAPACITY;
	Memory = (T*)Scal::MemAllocZero(Capacity * sizeof(T));
	return true;
}

template<typename T>
bool SList<T>::InitializeCap(uint64_t capacity)
{
	if (Memory)
	{
		TraceLog(LOG_ERROR, "Memory already initialized!");
		assert(false);
		return false;
	}
	Capacity = capacity;
	Memory = (T*)Scal::MemAllocZero(Capacity * sizeof(T));
	return true;
}

template<typename T>
bool SList<T>::Free()
{
	if (!Memory)
	{
		TraceLog(LOG_ERROR, "Memory was already freed!");
		assert(false);
		return false;
	}
	Length = 0;
	Scal::MemFree((void*)Memory);
	return true;
}

template<typename T>
void SList<T>::Resize(uint64_t capacity)
{
	assert(Memory);
	if (capacity < Capacity)
	{
		S_LOG_WARN("newSize was < capacity!");
		return;
	}
	Capacity = capacity;
	Memory = (T*)Scal::MemRealloc(Memory, Capacity * Stride());
}

template<typename T>
void SList<T>::Push(const T* valueSrc)
{
	assert(Memory);
	assert(valueSrc);
	if (Length == Capacity)
	{
		Resize((uint64_t)floorf((float)Capacity * SLIST_DEFAULT_RESIZE));
	}
	Memory[Length] = *valueSrc;
	++Length;
}

template<typename T>
void SList<T>::PushAt(uint64_t index, const T* valueSrc)
{
	assert(Memory);
	assert(valueSrc);
	if (Length == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	if (index > Length) index = Length;
	if (index != Length)
	{
		uint64_t dstOffset = (index + 1) * Stride();
		uint64_t srcOffset = index * Stride();
		uint64_t sizeTillEnd = (Length - index) * Stride();
		Scal::MemCopy(((char*)Memory) + dstOffset,
			((char*)Memory) + srcOffset, sizeTillEnd);
	}
	Memory[index] = *valueSrc;
	++Length;
}

template<typename T>
void SList<T>::PushAtFast(uint64_t index, const T* valueSrc)
{
	assert(Memory);
	assert(valueSrc);
	if (Length == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	Memory[Length] = Memory[index];
	Memory[index] = *valueSrc;
	++Length;
}

template<typename T>
bool SList<T>::PushUnique(const T* valueSrc)
{
	if (Contains(valueSrc)) return false;
	Push(valueSrc);
	return true;
}

template<typename T>
bool SList<T>::PushAtUnique(uint64_t index, const T* valueSrc)
{
	if (Contains(valueSrc))
		return false;
	PushAtUnique(index, valueSrc);
	return true;

}

template<typename T>
bool SList<T>::PushAtFastUnique(uint64_t index, const T* valueSrc)
{
	if (Contains(valueSrc))
		return false;
	PushAtFast(index, valueSrc);
	return true;
}

template<typename T>
void SList<T>::Pop(T* valueDest)
{
	assert(Memory);
	assert(Length > 0);
	*valueDest = Memory[End()];
	--Length;
}

template<typename T>
void SList<T>::PopAt(uint64_t index, T* valueDest)
{
	assert(Memory);
	assert(Length > 0);
	if (index > Length) return;
	*valueDest = Memory[index];
	if (index != Length)
	{
		uint64_t dstOffset = index * Stride();
		uint64_t srcOffset = (index + 1) * Stride();
		uint64_t sizeTillEnd = (Length - index) * Stride();
		Scal::MemCopy(((char*)Memory) + dstOffset,
			((char*)Memory) + srcOffset, sizeTillEnd);
	}
	--Length;
}

template<typename T>
void SList<T>::PopAtFast(uint64_t index, T* valueDest)
{
	assert(Memory);
	assert(Length > 0);
	uint64_t last = End();
	if (index > last) return;
	*valueDest = Memory[index];
	Memory[index] = Memory[last];
	--Length;
}

template<typename T>
void SList<T>::Remove()
{
	assert(Memory);
	assert(Length > 0);
	--Length;
}

template<typename T>
void SList<T>::RemoveAt(uint64_t index)
{
	assert(Memory);
	assert(Length > 0);
	if (index != Length)
	{
		uint64_t dstOffset = index * Stride();
		uint64_t srcOffset = (index + 1) * Stride();
		uint64_t sizeTillEnd = (Length - index) * Stride();
		Scal::MemCopy(((char*)Memory) + dstOffset,
			((char*)Memory) + srcOffset, sizeTillEnd);
	}
	--Length;
}

template<typename T>
void SList<T>::RemoveAtFast(uint64_t index)
{
	assert(Memory);
	assert(Length > 0);
	assert(index <= Capacity);
	Memory[index] = Memory[Length];
	Memory[Length] = {};
	--Length;
}

template<typename T>
const T& SList<T>::PeekAt(uint64_t index) const
{
	assert(Memory);
	assert(Length > 0);
	assert(index < Length);
	return Memory[index];
}

template<typename T>
T* SList<T>::PeekAtPtr(uint64_t index) const
{
	assert(Memory);
	assert(Length > 0);
	assert(index < Length);
	return &Memory[index];
}

template<typename T>
void SList<T>::Set(uint64_t index, const T* valueSrc)
{
	assert(Memory);
	if (index < Length)
	{
		Memory[index] = *valueSrc;
	}
	else
	{
		S_CRASH("index <= End() is false in SList::Set!");
	}
}		

template<typename T>
T* SList<T>::Last() const
{
	assert(Memory);
	return &Memory[End()];
}

template<typename T>
bool SList<T>::Contains(const T* value) const
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
void SList<T>::Clear()
{
	Length = 0;
}

template<typename T>
inline uint64_t SList<T>::End() const
{
	return (Length == 0) ? 0 : (Length - 1);
}

template<typename T>
inline size_t SList<T>::MemSize() const
{
	return Capacity * Stride();
}

template<typename T>
inline size_t SList<T>::Stride() const
{
	return sizeof(T);
}

template<typename T>
inline bool SList<T>::IsInitialized() const
{
	return (Memory != nullptr);
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