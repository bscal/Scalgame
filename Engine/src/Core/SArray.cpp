#include "SArray.h"

#include "SMemory.h"

#include <cassert>

namespace Scal
{

bool SArrayIterator::HasNext() const
{
	return Index <= Array->Length;
}

void* SArrayIterator::Next()
{
	void* offsetPtr = Peek();
	Offset += Array->Stride;
	++Index;
	return offsetPtr;
}

void* SArrayIterator::Peek()
{
	void* offsetPtr = (char*)(Array->Memory) + Offset;
	return offsetPtr;
}

void SArrayIterator::Remove()
{
	char* removeAtAddress = (char*)(Array->Memory) + Offset;
	if (Index != Array->Length)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = Array->Length * Array->Stride;
		char* lastIndexAddress = (char*)(Array->Memory) + lastIndexOffset;
		Memory::Copy(removeAtAddress, lastIndexAddress, Array->Stride);
		--Array->Length;
		Offset -= Array->Stride;
		--Index;
	}
}

SAPI void ArrayCreate(uint64_t capacity, uint64_t stride, ResizableArray* outSArray)
{
	assert(outSArray);
	if (outSArray->Memory)
	{
		TraceLog(LOG_ERROR, "outSArray Memory is already allocated!");
		return;
	}
	ResizableArray sArray = {};
	sArray.Memory = (ResizableArray*)Memory::Alloc(capacity * stride);
	sArray.Capacity = capacity;
	sArray.Stride = stride;
	*outSArray = sArray;
}

SAPI void ArrayDestroy(ResizableArray* sArray)
{
	assert(sArray);
	assert(sArray->Memory);
	Memory::Free(sArray->Memory);
}

SAPI ResizableArray* ArrayResize(ResizableArray* sArray)
{
	assert(sArray);
	assert(sArray->Memory);
	sArray->Capacity *= 2;
	uint64_t newSize = sArray->Capacity * sArray->Stride;
	sArray->Memory = Memory::ReAlloc(sArray->Memory, newSize);
	return sArray;
}

SAPI uint64_t GetArrayMemorySize(ResizableArray* sArray)
{
	return sArray->Capacity * sArray->Stride;
}

SAPI void ArrayPush(ResizableArray* sArray, const void* valuePtr)
{
	if (sArray->Length >= sArray->Capacity)
	{
		ArrayResize(sArray);
	}
	uint64_t offset = sArray->Length * sArray->Stride;
	char* dest = (char*)sArray->Memory;
	Memory::Copy(dest + offset, valuePtr, sArray->Stride);
	++sArray->Length;
}

SAPI void ArrayPop(ResizableArray* sArray, void* dest)
{
	if (sArray->Length == 0) return;
	uint64_t offset = (sArray->Length - 1) * sArray->Stride;
	const char* src = (char*)(sArray->Memory);
	Memory::Copy(dest, src + offset, sArray->Stride);
	--sArray->Length;
}

SAPI void ArraySetAt(ResizableArray* sArray, uint64_t index, const void* valuePtr)
{
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	char* dest = (char*)(sArray->Memory) + offset;
	Memory::Copy(dest, valuePtr, sArray->Stride);
}

SAPI void ArrayPopAt(ResizableArray* sArray, uint64_t index, void* dest)
{
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	char* popAtAddress = (char*)(sArray->Memory) + offset;
	Memory::Copy(dest, popAtAddress, sArray->Stride);
	if (index != sArray->Length)
	{
		// Moves last element in array popped position
		uint64_t lastIndexOffset = sArray->Length * sArray->Stride;
		char* lastIndexAddress = (char*)(sArray->Memory) + lastIndexOffset;
		Memory::Copy(popAtAddress, lastIndexAddress, sArray->Stride);
	}
	--sArray->Length;
}

SAPI void* ArrayPeekAt(ResizableArray* sArray, uint64_t index)
{
	assert(index <= sArray->Length);

	uint64_t offset = index * sArray->Stride;
	return (char*)(sArray->Memory) + offset;
}

SAPI void ArrayClear(ResizableArray* sArray)
{
	sArray->Length = 0;
}

}