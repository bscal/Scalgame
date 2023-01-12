#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

struct Stack
{
	uint64_t Capacity;
	uint64_t TopSize;
	uint8_t* Memory;
	uint8_t* TopStart;
	uint8_t* TopEnd;
};

void Initialize(Stack* stack, size_t capacity)
{
	stack->Capacity = capacity;
	stack->Memory = (uint8_t*)Scal::MemAlloc(capacity);
	stack->TopStart = stack->Memory;
	stack->TopEnd = stack->Memory;
}

void Reset(Stack* stack)
{
	Scal::MemClear(stack->Memory, stack->Capacity);
	stack->TopStart = stack->Memory;
	stack->TopEnd = stack->Memory;
}

void StackStart(Stack* stack)
{
}

void StackEnd(Stack* stack)
{
	Scal::MemClear(stack->TopStart, stack->TopSize);
	stack->TopSize = 0;
	stack->TopStart = stack->TopEnd;
}

void* Push(Stack* stack, size_t size)
{
	SASSERT(stack->TopEnd + size < stack->Memory + stack->Capacity);
	void* ptr = stack->TopEnd;
	stack->TopSize += size;
	stack->TopEnd += size;
	return ptr;
}
