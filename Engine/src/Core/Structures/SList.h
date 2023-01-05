#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#define SLIST_DEFAULT_CAPACITY 1
#define SLIST_DEFAULT_RESIZE 2

template<typename T>
struct SList
{
	T* Memory;
	uint64_t Capacity;
	uint64_t Count;

	void Initialize();
	void InitializeCap(uint64_t capacity);

	void Free();
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
	int64_t Find(const T* value) const;
	void Clear();

	// Points to last element, unless 0 then 0
	inline uint64_t End() const;
	inline size_t MemSize() const;
	inline bool IsInitialized() const;
};

// ********************

template<typename T>
void SList<T>::Initialize()
{
	SASSERT_MSG(!Memory, "Memory already initialized!");
	Capacity = SLIST_DEFAULT_CAPACITY;
	Memory = (T*)Scal::MemAllocZero(Capacity * sizeof(T));
	SASSERT(Memory);
}

template<typename T>
void SList<T>::InitializeCap(uint64_t capacity)
{
	SASSERT_MSG(!Memory, "Memory already initialized!");
	Capacity = capacity;
	Memory = (T*)Scal::MemAllocZero(Capacity * sizeof(T));
	SASSERT(Memory);
}

template<typename T>
void SList<T>::Free()
{
	SASSERT_MSG(Memory, "Trying to free uninitalized memory!");
	Scal::MemFree(Memory);
	Memory = NULL;
	Count = 0;
}

template<typename T>
void SList<T>::Resize(uint64_t newCapacity)
{
	SASSERT(Memory);
	SASSERT(newCapacity > Capacity);
	if (newCapacity < Capacity)
	{
		SLOG_WARN("newSize was < capacity!");
		return;
	}
	Capacity = newCapacity;
	size_t newSize = Capacity * sizeof(T);
	Memory = (T*)Scal::MemRealloc(Memory, newSize);
}

template<typename T>
void SList<T>::Push(const T* valueSrc)
{
	SASSERT(Memory);
	SASSERT(valueSrc);
	if (Count == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	Memory[Count] = *valueSrc;
	++Count;
}

template<typename T>
void SList<T>::PushAt(uint64_t index, const T* valueSrc)
{
	SASSERT(Memory);
	SASSERT(valueSrc);
	if (Count == Capacity)
	{
		Resize(Capacity * SLIST_DEFAULT_RESIZE);
	}
	if (index != Count)
	{
		uint64_t dstOffset = (index + 1) * sizeof(T);
		uint64_t srcOffset = index * sizeof(T);
		uint64_t sizeTillEnd = (Count - index) * sizeof(T);
		char* mem = (char*)Memory;
		Scal::MemCopy(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
	Memory[index] = *valueSrc;
	++Count;
}

template<typename T>
void SList<T>::PushAtFast(uint64_t index, const T* valueSrc)
{
	SASSERT(Memory);
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
bool SList<T>::PushAtUnique(uint64_t index, const T* valueSrc)
{
	if (Contains(valueSrc)) return false;
	PushAt(index, valueSrc);
	return true;
}

template<typename T>
bool SList<T>::PushAtFastUnique(uint64_t index, const T* valueSrc)
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
	Scal::MemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::PopAt(uint64_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	*valueDest = Memory[index];
	if (index < End())
	{
		uint64_t dstOffset = index * sizeof(T);
		uint64_t srcOffset = (index + 1) * sizeof(T);
		uint64_t sizeTillEnd = (Count - index) * sizeof(T);
		char* mem = (char*)Memory;
		Scal::MemCopy(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
	#if SCAL_DEBUG
	Scal::MemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::PopAtFast(uint64_t index, T* valueDest)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	*valueDest = Memory[index];
	if (index < End())
	{
		Memory[index] = Memory[End()];
		#if SCAL_DEBUG
		Scal::MemClear(&Memory[End()], sizeof(T));
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
	Scal::MemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::RemoveAt(uint64_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	if (index != End())
	{
		uint64_t dstOffset = index * sizeof(T);
		uint64_t srcOffset = (index + 1) * sizeof(T);
		uint64_t sizeTillEnd = (End() - index) * sizeof(T);
		char* mem = (char*)Memory;
		Scal::MemCopy(mem + dstOffset, mem + srcOffset, sizeTillEnd);
	}
	#if SCAL_DEBUG
	Scal::MemClear(&Memory[End()], sizeof(T));
	#endif
	--Count;
}

template<typename T>
void SList<T>::RemoveAtFast(uint64_t index)
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	if (index < End())
	{
		Memory[index] = Memory[End()];
		#if SCAL_DEBUG
		Scal::MemClear(&Memory[End()], sizeof(T));
		#endif
	}
	--Count;
}

template<typename T>
const T& SList<T>::PeekAt(uint64_t index) const
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	return Memory[index];
}

template<typename T>
T* SList<T>::PeekAtPtr(uint64_t index) const
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	SASSERT(index < Count);
	return &Memory[index];
}

template<typename T>
void SList<T>::Set(uint64_t index, const T* valueSrc)
{
	SASSERT(Memory);
	SASSERT(index < Count);
	if (index < Count) Memory[index] = *valueSrc;
}		

template<typename T>
T* SList<T>::Last() const
{
	SASSERT(Memory);
	SASSERT(Count > 0);
	return &Memory[End()];
}

template<typename T>
bool SList<T>::Contains(const T* value) const
{
	SASSERT(Memory);
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
	SASSERT(Memory);
	SASSERT(value);
	for (int64_t i = 0; i < Count; ++i)
	{
		if (Memory[i] == *value) return i;
	}
	return -1;
}

template<typename T>
void SList<T>::Clear()
{
	Count = 0;
	// NOTE: I only clear memory to 0
	// in debug mode to help debug.
	#if SCAL_DEBUG
	if (Count > 0) Scal::MemClear(Memory, MemSize());
	#endif
}

template<typename T>
inline uint64_t SList<T>::End() const
{
	return (Count == 0) ? 0 : (Count - 1);
}

template<typename T>
inline size_t SList<T>::MemSize() const
{
	return Capacity * sizeof(T);
}

template<typename T>
inline bool SList<T>::IsInitialized() const
{
	return (Memory);
}

inline void TestListImpl()
{
	SList<int> list = {};
	list.Initialize(); // Cap 1

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

	int peek = list.PeekAt(2);
	SASSERT(peek == 2);

	int* peekPtr = list.PeekAtPtr(3);
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

	SASSERT(!list.IsInitialized());
}