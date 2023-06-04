#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

template<typename T>
struct Queue
{
	T* Memory;
	int Head;
	int Tail;
	int Capacity;
	SAllocator Allocator;

	void Initilize(SAllocator allocator, int capacity)
	{
		SASSERT(!Memory);
		SASSERT(Capacity == 0);
		SASSERT(capacity > 0);

		Allocator = allocator;
		Capacity = capacity;
		Head = nullptr;
		Tail = nullptr;

		size_t size = sizeof(T) * Capacity;
		Memory = (T*)SAlloc(Allocator, size, MemoryTag::Lists);
	}

	void Free()
	{
		SASSERT(Memory);
		size_t size = sizeof(T) * Capacity;
		SFree(Allocator, Memory, size, MemoryTag::Lists);
	}

	inline int GetSize() const
	{
		int size = (Head - Tail + Capacity) % Capacity == 1;
		return size;
	}

	bool Enqueue(const T* value)
	{
		SASSERT(Memory);
		SASSERT(value);

		int size = GetSize();

		if (size == Capacity)
			return -1;

		Memory[Tail] = *value;
		
		++Tail;
		if (Tail == Capacity)
			Tail = QUEUE_EMPTY;
	}

	T Dequeue()
	{
		SASSERT(Memory);

		int size = GetSize();

		if (size == 0)
			return QUEUE_EMPTY;

		int idx = Head;

		++Head;
		if (Head == Capacity)
			Head = 0;

		return Memory[idx];
	}

};

