#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#define SLIST_DEFAULT_RESIZE 2

template<typename T>
struct SList
{
	T* Memory;
	uint32_t Capacity;
	uint32_t Count; // Number of elements or last index to insert
	SAllocator Allocator;

	void EnsureSize(uint32_t ensuredCount); // ensures capacity and count elements
	void Free();
	void Reserve(uint32_t capacity); // allocated memory based on capacity

	void Push(const T* valueSrc); // Checks resize, inserts and end of array
	T* PushNew(); // Checks resize, default constructs next element, and returns pointer. Useful if you dont want to copy large objects
	void PushAt(uint32_t index, const T* valueSrc);
	void PushAtFast(uint32_t index, const T* valueSrc); // Checks resize, inserts at index, moving old index to end
	bool PushUnique(const T* valueSrc);
	bool PushAtUnique(uint32_t index, const T* valueSrc);
	bool PushAtFastUnique(uint32_t index, const T* valueSrc);
	void Pop(T* valueDest); // Pops end of array
	void PopAt(uint32_t index, T* valueDest);
	void PopAtFast(uint32_t index, T* valueDest); // pops index, moving last element to index
	void Remove(); // Same as pop, but does not do a copy
	void RemoveAt(uint32_t index);
	void RemoveAtFast(uint32_t index);

	inline T* PeekAt(uint32_t index) const;
	inline T* Last() const;

	const T& operator[](size_t i) const { SASSERT(i < Count); return Memory[i]; }
	T& operator[](size_t i) { SASSERT(i < Count); return Memory[i]; }

	T* begin() { return Memory; }
	T* end() { return Memory + Count; }

	SList<T>& Assign(T* inList, size_t listCount);

	bool Contains(const T* value) const;
	int64_t Find(const T* value) const;
	inline void Clear();

	inline uint32_t LastIndex() const; // last used index, or 0
	inline size_t MemUsed() const; // Total memory used in bytes
	inline bool IsAllocated() const;
};

// ********************

template<typename T>
void SList<T>::EnsureSize(uint32_t ensuredCount)
{
	if (ensuredCount <= Count) return;

	Reserve(ensuredCount);

	for (uint32_t i = Count; i < ensuredCount; ++i)
	{
		Memory[i] = T{};
	}
	Count = ensuredCount;

	SASSERT(ensuredCount <= Count);
	SASSERT(ensuredCount <= Capacity);
	SASSERT(Count <= Capacity);
}

template<typename T>
void SList<T>::Free()
{
	SFree(Allocator, Memory, MemUsed(), MemoryTag::Arrays);
	Memory = nullptr;
	Capacity = 0;
	Count = 0;
}

template<typename T>
void SList<T>::Reserve(uint32_t newCapacity)
{
	if (newCapacity == 0) newCapacity = 1;
	if (newCapacity <= Capacity) return;

	size_t oldSize = MemUsed();
	size_t newSize = newCapacity * sizeof(T);
	Memory = (T*)SRealloc(Allocator, Memory, oldSize, newSize, MemoryTag::Arrays);
	Capacity = newCapacity;
	SASSERT(Memory);
	SASSERT(Count <= newCapacity);
}

template<typename T>
void SList<T>::Push(const T* valueSrc)
{
	SASSERT(valueSrc);
	SASSERT(Count <= Capacity);
	if (Count == Capacity)
	{
		Reserve(Capacity * SLIST_DEFAULT_RESIZE);
	}
	Memory[Count] = *valueSrc;
	++Count;
}

template<typename T>
T* SList<T>::PushNew()
{
	SASSERT(Count <= Capacity);
	if (Count == Capacity)
	{
		Reserve(Capacity * SLIST_DEFAULT_RESIZE);
	}
	
	uint32_t index = Count;
	++Count;
	Memory[index] = T{};
	return &Memory[index];
}

template<typename T>
void SList<T>::PushAt(uint32_t index, const T* valueSrc)
{
	SASSERT(index < Count);
	SASSERT(valueSrc);
	SASSERT(Count <= Capacity);
	if (Count == Capacity)
	{
		Reserve(Capacity * SLIST_DEFAULT_RESIZE);
	}
	if (index != Count)
	{
		size_t dstOffset = (size_t)(index + 1) * sizeof(T);
		size_t srcOffset = index * sizeof(T);
		size_t sizeTillEnd = (size_t)(Count - index) * sizeof(T);
		char* mem = (char*)Memory;
		SMemMove(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
	Memory[index] = *valueSrc;
	++Count;
}

template<typename T>
void SList<T>::PushAtFast(uint32_t index, const T* valueSrc)
{
	SASSERT(index < Count);
	SASSERT(valueSrc);
	SASSERT(Count <= Capacity);
	if (Count == Capacity)
	{
		Reserve(Capacity * SLIST_DEFAULT_RESIZE);
	}
	if (Count > 1)
	{
		Memory[Count] = Memory[index];
	}
	Memory[index] = *valueSrc;
	++Count;
}

template<typename T>
bool SList<T>::PushUnique(const T* valueSrc)
{
	if (Contains(valueSrc)) return false;
	Push(valueSrc);
	return true;
}

template<typename T>
bool SList<T>::PushAtUnique(uint32_t index, const T* valueSrc)
{
	if (Contains(valueSrc)) return false;
	PushAt(index, valueSrc);
	return true;
}

template<typename T>
bool SList<T>::PushAtFastUnique(uint32_t index, const T* valueSrc)
{
	if (Contains(valueSrc)) return false;
	PushAtFast(index, valueSrc);
	return true;
}

template<typename T>
void SList<T>::Pop(T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(Count <= Capacity);
	--Count;
	*valueDest = Memory[Count];
}

template<typename T>
void SList<T>::PopAt(uint32_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	SASSERT(Count <= Capacity);
	*valueDest = Memory[index];
	--Count;
	if (index < Count)
	{
		size_t dstOffset = index * sizeof(T);
		size_t srcOffset = (size_t)(index + 1) * sizeof(T);
		size_t sizeTillEnd = (size_t)(Count - index) * sizeof(T);
		char* mem = (char*)Memory;
		SMemMove(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}

}

template<typename T>
void SList<T>::PopAtFast(uint32_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	SASSERT(Count <= Capacity);
	*valueDest = Memory[index];
	--Count;
	if (index != Count)
	{
		Memory[index] = Memory[Count];
	}
}

template<typename T>
void SList<T>::Remove()
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	--Count;
}

template<typename T>
void SList<T>::RemoveAt(uint32_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	--Count;
	if (index != Count)
	{
		size_t dstOffset = index * sizeof(T);
		size_t srcOffset = (size_t)(index + 1) * sizeof(T);
		size_t sizeTillEnd = (size_t)(Count - index) * sizeof(T);
		char* mem = (char*)Memory;
		SMemMove(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
}

template<typename T>
void SList<T>::RemoveAtFast(uint32_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	--Count;
	if (index != Count)
	{
		Memory[index] = Memory[Count];
	}
}

template<typename T>
inline T* SList<T>::PeekAt(uint32_t index) const
{
	SASSERT(Memory);
	SASSERT(index < Count);
	SASSERT(index < Capacity);
	SASSERT(Count <= Capacity);
	return &Memory[index];
}

template<typename T>
inline T* SList<T>::Last() const
{
	SASSERT(Memory);
	return &Memory[LastIndex()];
}

template<typename T>
SList<T>& SList<T>::Assign(T* inList, size_t listCount)
{
	SASSERT(inList);
	SASSERT(listCount > 0);
	SASSERT(!IsAllocated());
	SASSERT(Count == 0);
	SASSERT(listCount < UINT32_MAX)
	EnsureSize((uint32_t)listCount);
	SMemCopy(Memory, inList, listCount * sizeof(T));
	Count = (uint32_t)listCount;

	return *this;
}

template<typename T>
bool SList<T>::Contains(const T* value) const
{
	SASSERT(value);
	for (uint32_t i = 0; i < Count; ++i)
	{
		if (Memory[i] == *value) return true;
	}
	return false;
}

template<typename T>
int64_t SList<T>::Find(const T* value) const
{
	SASSERT(value);
	for (int64_t i = 0; i < Count; ++i)
	{
		if (Memory[i] == *value) return i;
	}
	return -1;
}

template<typename T>
inline void SList<T>::Clear()
{
	Count = 0;
}

template<typename T>
inline uint32_t SList<T>::LastIndex() const
{
	return (Count == 0) ? 0 : (Count - 1);
}

template<typename T>
inline size_t SList<T>::MemUsed() const
{
	return Capacity * sizeof(T);
}

template<typename T>
inline bool SList<T>::IsAllocated() const
{
	return (Memory);
}

inline int TestListImpl()
{
	SList<int> list = {};

	int i;
	i = 1;
	list.Push(&i); // Cap 1
	i = 2;
	list.Push(&i); // Cap 2
	i = 3;
	list.Push(&i); // Cap 4

	SASSERT(list.Capacity == 4);

	i = 4;
	list.Push(&i); // Cap 4
	i = 5;
	list.Push(&i); // Cap 8
	i = 6;
	list.Push(&i); // Cap 8
	i = 7;
	list.Push(&i); // Cap 8 
	i = 8;
	list.Push(&i); // Cap 8

	SASSERT(list.Count == 8);
	SASSERT(list.Capacity == 8);

	i = 9;
	list.Push(&i); // Cap 16
	i = 10;
	list.Push(&i); // Cap 16

	SASSERT(list.Count == 10);
	SASSERT(list.Capacity == 16);

	i = 5;
	bool wasUnique = list.PushAtUnique(5, &i);
	SASSERT(!wasUnique);

	i = 20;
	list.PushAt(0, &i);
	SASSERT(list.Memory[0] == 20);

	i = 21;
	list.PushAtFast(1, &i);
	SASSERT(list.Memory[1] == 21);
	SASSERT(list.Memory[list.LastIndex()] == 1);

	SASSERT(list.Count == 12);
	SASSERT(list.Capacity == 16);

	int peek = list[2];
	SASSERT(peek == 2);

	int* peekPtr = list.PeekAt(3);
	SASSERT(peekPtr);
	SASSERT(*peekPtr == 3);

	int pop = 0;
	list.Pop(&pop);
	SASSERT(pop == 1);

	pop = 0;
	list.Pop(&pop);
	SASSERT(pop == 10);

	SASSERT(list.Count == 10);
	SASSERT(list.Capacity == 16);
	
	pop = 0;
	list.PopAt(0, &pop);
	SASSERT(pop == 20);
	SASSERT(list.Memory[1] == 2);
	int last = *list.Last();
	SASSERT(last == 9);

	pop = 0;
	list.PopAtFast(0, &pop);
	SASSERT(pop == 21);
	SASSERT(list.Memory[0] == last);

	SASSERT(*list.Last() == 8);

	SASSERT(list.Count == 8);

	list.Remove();
	list.RemoveAt(1);
	list.RemoveAtFast(2);

	SASSERT(list.Count == 5);
	SASSERT(list.Capacity == 16);
	SASSERT(list.Memory[0] == 9);
	SASSERT(list.Memory[1] == 3);
	SASSERT(list.Memory[2] == 7);
	SASSERT(list.Memory[3] == 5);
	SASSERT(*list.Last() == 6);

	list.Free();

	SASSERT(!list.IsAllocated());
	return 1;
}