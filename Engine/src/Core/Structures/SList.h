#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#define SLIST_NO_FOUND UINT32_MAX

enum class SListResizeType
{
	Double = 0,
	IncreaseByOne
};

template<typename T>
struct SList
{
	T* Memory;
	uint32_t Capacity;
	uint32_t Count;
	SListResizeType ResizeType;
	SAllocator Allocator;

	void EnsureSize(uint32_t ensuredCount); // ensures capacity and count elements
	void Free();
	void Reserve(uint32_t capacity); // allocated memory based on capacity
	void Resize(); // Resizes the array, if size = 0, then size will be 1

	void Push(const T* valueSrc); // Checks resize, inserts and end of array
	T* PushNew(); // Checks resize, default constructs next element, and returns pointer
	void PushAt(uint32_t index, const T* valueSrc);
	void PushAtFast(uint32_t index, const T* valueSrc); // Checks resize, inserts at index, moving old index to end
	bool PushUnique(const T* valueSrc);
	bool PushAtUnique(uint32_t index, const T* valueSrc);
	bool PushAtFastUnique(uint32_t index, const T* valueSrc);
	void Pop(T* valueDest); // Pops end of array
	void PopAt(uint32_t index, T* valueDest);
	void PopAtFast(uint32_t index, T* valueDest); // pops index, moving last element to index
	void Remove(); // Same as pop, but does not do a copy
	bool RemoveAt(uint32_t index);
	bool RemoveAtFast(uint32_t index);
	void Set(uint32_t index, const T* data);

	_FORCE_INLINE_ T* PeekAt(uint32_t index) const;
	_FORCE_INLINE_ T* Last() const;

	SList<T>& Assign(T* inList, size_t listCount);

	bool Contains(const T* value) const;
	uint32_t Find(const T* value) const;
	void Clear();

	_FORCE_INLINE_ T& operator[](size_t idx) { SASSERT(idx < Count); return Memory[idx]; }
	_FORCE_INLINE_ const T& operator[](size_t idx) const { SASSERT(idx < Count); return Memory[idx]; }

	_FORCE_INLINE_ T* begin() { return Memory; }
	_FORCE_INLINE_ T* end() { return Memory + Count; }

	_FORCE_INLINE_ size_t MemUsed() const { return Capacity * sizeof(T); }
	_FORCE_INLINE_ bool IsAllocated() const { return (Memory); }

	_FORCE_INLINE_ uint32_t LastIndex() const
	{
		return (Count == 0) ? 0 : (Count - 1);
	}
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
	if (Memory)
		SFree(Allocator, Memory, MemUsed(), MemoryTag::Lists);
	Memory = nullptr;
	Capacity = 0;
	Count = 0;
}

template<typename T>
void SList<T>::Reserve(uint32_t newCapacity)
{
	if (newCapacity <= Capacity)
		return;

	size_t oldSize = MemUsed();
	size_t newSize = newCapacity * sizeof(T);
	Memory = (T*)SRealloc(Allocator, Memory, oldSize, newSize, MemoryTag::Lists);
	Capacity = newCapacity;
	SASSERT(Memory);
	SASSERT(Count <= newCapacity);
}

template<typename T>
void SList<T>::Resize()
{
	uint32_t capacity = Capacity;
	if (capacity == 0 || ResizeType == SListResizeType::IncreaseByOne)
		++capacity;
	else
		capacity *= 2;

	Reserve(capacity);
}

template<typename T>
void SList<T>::Push(const T* valueSrc)
{
	SASSERT(valueSrc);
	SASSERT(Count <= Capacity);
	if (Count == Capacity)
	{
		Resize();
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
		Resize();
	}
	
	T* pos = Memory + Count;
	++Count;
	SMemClear(pos, sizeof(T));
	return pos;
}

template<typename T>
void SList<T>::PushAt(uint32_t index, const T* valueSrc)
{
	SASSERT(index < Count);
	SASSERT(valueSrc);

	if (Count == Capacity)
	{
		Resize();
	}

	if (index != LastIndex())
	{
		T* dst = Memory + index + 1;
		T* src = Memory + index;
		size_t sizeTillEnd = (Count - index) * sizeof(T);
		SMemMove(dst, src, sizeTillEnd);
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
		Resize();
	}
	if (index != LastIndex())
	{
		SMemMove(Memory + Count, Memory + index, sizeof(T));
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
	SMemCopy(valueDest, Memory + LastIndex(), sizeof(T));
	--Count;
}

template<typename T>
void SList<T>::PopAt(uint32_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	SASSERT(Count <= Capacity);
	SMemCopy(valueDest, Memory + index, sizeof(T));
	if (index != LastIndex())
	{
		T* dst = Memory + index;
		T* src = Memory + index + 1;
		size_t size = (size_t)(LastIndex() - index) * sizeof(T);
		SMemMove(dst, src, size);
	}
	--Count;
}

template<typename T>
void SList<T>::PopAtFast(uint32_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	SASSERT(Count <= Capacity);
	SMemCopy(valueDest, Memory + index, sizeof(T));
	if (index != LastIndex())
	{
		SMemCopy(Memory + index, Memory + LastIndex(), sizeof(T));
	}
	--Count;
}

template<typename T>
void SList<T>::Remove()
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	--Count;
}

template<typename T>
bool SList<T>::RemoveAt(uint32_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	--Count;
	if (index != Count)
	{
		size_t dstOffset = index * sizeof(T);
		size_t srcOffset = ((size_t)(index) + 1) * sizeof(T);
		size_t sizeTillEnd = ((size_t)(Count) -(size_t)(index)) * sizeof(T);
		char* mem = (char*)Memory;
		SMemMove(mem + dstOffset, mem + srcOffset, sizeTillEnd);
		return true;
	}
	return false;
}

template<typename T>
bool SList<T>::RemoveAtFast(uint32_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);

	bool wasLastSwapped = false;

	if (index != LastIndex())
	{
		SMemCopy(Memory + index, Memory + LastIndex(), sizeof(T));
		wasLastSwapped = true;
	}

	--Count;
	return wasLastSwapped;
}

template<typename T>
void SList<T>::Set(uint32_t index, const T* data)
{
	EnsureSize(index);
	SMemCopy(Memory + index, data, sizeof(T));
}

template<typename T>
T* SList<T>::PeekAt(uint32_t index) const
{
	SASSERT(Memory);
	SASSERT(index < Count);
	SASSERT(index < Capacity);
	SASSERT(Count <= Capacity);
	return &Memory[index];
}

template<typename T>
T* SList<T>::Last() const
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
		if (Memory[i] == *value)
			return true;
	}
	return false;
}

template<typename T>
uint32_t SList<T>::Find(const T* value) const
{
	SASSERT(value);
	for (uint32_t i = 0; i < Count; ++i)
	{
		if (Memory[i] == *value)
			return i;
	}
	return SLIST_NO_FOUND;
}

template<typename T>
void SList<T>::Clear()
{
	Count = 0;
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