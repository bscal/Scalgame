#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#define SLIST_DEFAULT_RESIZE 2
#define SLIST_NO_POSITION UINT64_MAX

template<typename T>
struct SList
{
	T* Memory;
	uint32_t Capacity;
	uint32_t Count;
	SMemAllocator Allocator;

	void EnsureCapacity(uint32_t ensuredCapacity);
	void EnsureSize(uint32_t ensuredCount); // ensures capacity and count elements
	void Free();
	void Resize(uint32_t capacity);

	void Push(const T* valueSrc); // Checks resize, inserts and end of array
	T* PushZero(); // Checks resize, clears memory, returns ptr to end
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
	void Set(uint32_t index, const T* valueSrc);

	inline T* PeekAt(uint32_t index) const;
	inline T* Last() const;

	const T& operator[](size_t i) const { SASSERT(i < Count); return Memory[i]; }
	T& operator[](size_t i) { SASSERT(i < Count); return Memory[i]; }

	bool Contains(const T* value) const;
	int64_t Find(const T* value) const; // index to element, SLIST_NO_POSITION if not found
	inline void Clear();

	inline uint32_t End() const; // Index to last element, SLIST_NO_POSITION = 0 elements
	inline size_t MemUsed() const; // Total memory used in bytes
	inline size_t Stride() const;
	inline bool IsAllocated() const;
};

// ********************

template<typename T>
void SList<T>::EnsureCapacity(uint32_t ensuredCapacity)
{
	if (Capacity >= ensuredCapacity) return;
	Resize(ensuredCapacity);
}

template<typename T>
void SList<T>::EnsureSize(uint32_t ensuredCount)
{
	if (Count >= ensuredCount) return;

	if (ensuredCount > Capacity)
	{
		Capacity = ensuredCount;
		Allocator.Free(Memory);
		Memory = (T*)Allocator.Alloc(MemUsed());
	}

	uint32_t ensuredSize = Capacity - Count;
	SMemSet(&Memory[Count], 0, ensuredSize * sizeof(T));
	Count = Capacity;
}

template<typename T>
void SList<T>::Free()
{
	if (Memory)
	{
		SMemClear(Memory, MemUsed());
		Allocator.Free(Memory);
		Memory = NULL;
	}
	Capacity = 0;
	Count = 0;
}

template<typename T>
void SList<T>::Resize(uint32_t newCapacity)
{
	if (newCapacity == 0 && Capacity == 0) newCapacity = 1;
	Capacity = newCapacity;

	if (Memory)
	{
		T* newMem = (T*)Allocator.Alloc(MemUsed());
		SMemMove(newMem, Memory, MemUsed());
		Allocator.Free(Memory);
		Memory = newMem;
	}
	else Memory = (T*)Allocator.Alloc(MemUsed());
}

template<typename T>
void SList<T>::Push(const T* valueSrc)
{
	SASSERT(valueSrc);
	if (Count == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	Memory[Count] = *valueSrc;
	++Count;
}

template<typename T>
T* SList<T>::PushZero()
{
	if (Count == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	++Count;
	Memory[End()] = T{};
	return &Memory[End()];
}

template<typename T>
void SList<T>::PushAt(uint32_t index, const T* valueSrc)
{
	SASSERT(index < Count);
	SASSERT(valueSrc);
	if (Count == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	if (index != Count)
	{
		uint32_t dstOffset = (index + 1) * sizeof(T);
		uint32_t srcOffset = index * sizeof(T);
		uint32_t sizeTillEnd = (Count - index) * sizeof(T);
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
	if (Count == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
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
	*valueDest = Memory[End()];
	#if SCAL_DEBUG
	SMemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::PopAt(uint32_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	*valueDest = Memory[index];
	if (index < End())
	{
		uint32_t dstOffset = index * sizeof(T);
		uint32_t srcOffset = (index + 1) * sizeof(T);
		uint32_t sizeTillEnd = (Count - index) * sizeof(T);
		char* mem = (char*)Memory;
		SMemMove(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
	#if SCAL_DEBUG
	SMemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::PopAtFast(uint32_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	*valueDest = Memory[index];
	if (index < End())
	{
		Memory[index] = Memory[End()];
		#if SCAL_DEBUG
		SMemClear(&Memory[End()], sizeof(T));
		#endif
	}
	--Count;
}

template<typename T>
void SList<T>::Remove()
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	#if SCAL_DEBUG
	SMemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::RemoveAt(uint32_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	if (index != End())
	{
		uint32_t dstOffset = index * sizeof(T);
		uint32_t srcOffset = (index + 1) * sizeof(T);
		uint32_t sizeTillEnd = (End() - index) * sizeof(T);
		char* mem = (char*)Memory;
		SMemMove(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
	#if SCAL_DEBUG
	SMemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::RemoveAtFast(uint32_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	if (index < End())
	{
		Memory[index] = Memory[End()];
		#if SCAL_DEBUG
		SMemClear(&Memory[End()], sizeof(T));
		#endif
	}
	--Count;
}

template<typename T>
inline T* SList<T>::PeekAt(uint32_t index) const
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	return &Memory[index];
}

template<typename T>
void SList<T>::Set(uint32_t index, const T* valueSrc)
{
	SASSERT(Memory);
	SASSERT(index < Count);
	if (index < Count) Memory[index] = *valueSrc;
}		

template<typename T>
inline T* SList<T>::Last() const
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	return &Memory[End()];
}

template<typename T>
bool SList<T>::Contains(const T* value) const
{
	SASSERT(value);
	for (int i = 0; i < Count; ++i)
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
	SMemClear(Memory, MemUsed());
	Count = 0;
}

template<typename T>
inline uint32_t SList<T>::End() const
{
	return (Count == 0) ? SLIST_NO_POSITION : (Count - 1);
}

template<typename T>
inline size_t SList<T>::MemUsed() const
{
	return Capacity * sizeof(T);
}

template<typename T>
inline size_t SList<T>::Stride() const
{
	return sizeof(T);
}

template<typename T>
inline bool SList<T>::IsAllocated() const
{
	return (Memory);
}

inline void TestListImpl()
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
	SASSERT(list.Memory[list.End()] == 1);

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
}