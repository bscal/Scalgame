#pragma once

#include "Core.h"
#include "SMemory.h"

struct SBuffer
{
	uint8_t* Data;
	size_t Position;
	size_t Capacity;
	SAllocator Allocator;
	bool CanResize;
};

SBuffer BufferCreate(SAllocator allocator, uint32_t initialCapacity);
void BufferFree(SBuffer* buf);

void TryResize(SBuffer* buf)
{
	if (buf->Position >= buf->Capacity)
	{
		if (!buf->CanResize)
		{
			SASSERT_MSG(false, "SBuffer run out of memory and CanResize = false!");
			return;
		}

		size_t newCapacity = buf->Capacity * 2;
		buf->Data = (char*)SRealloc(buf->Allocator, buf->Data, buf->Capacity, newCapacity, MemoryTag::Arrays);
		buf->Capacity = newCapacity;
		SLOG_WARN("Reallocating buffer!");
	}
}

inline bool IsSystemLittleEndian()
{
	int value = 0x01;
	void* address = static_cast<void*>(&value);
	unsigned char* least_significant_address = static_cast<unsigned char*>(address);
	return (*least_significant_address == 0x01);
}

uint16_t SwapU16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

int16_t SwapI16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

uint32_t SwapU32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

int32_t SwapI32(int32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

uint64_t SwapU64(uint64_t val)
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return (val << 32) | (val >> 32);
}

int64_t SwapI64(int64_t val)
{
	val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
	val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
	return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

void WriteByte(SBuffer* buf, uint8_t value)
{
	SASSERT(buf);
	TryResize(buf);

	size_t size = sizeof(uint8_t);
	SMemCopy(buf->Data + buf->Position, &value, size);
	buf->Position += size;
}

uint8_t ReadByte(SBuffer* buf)
{
	SASSERT(buf);

	uint8_t value;
	size_t size = sizeof(uint8_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return value;
}

void WriteShort(SBuffer* buf, int16_t value)
{
	SASSERT(buf);
	TryResize(buf);

	int16_t swappedValue = (IsSystemLittleEndian()) ? SwapI16(value) : value;
	size_t size = sizeof(short);
	SMemCopy(buf->Data + buf->Position, &swappedValue, size);
	buf->Position += size;
}

int16_t ReadShort(SBuffer* buf)
{
	SASSERT(buf);

	int16_t value;
	size_t size = sizeof(int16_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return (IsSystemLittleEndian()) ? SwapI16(value) : value;
}

void WriteUShort(SBuffer* buf, uint16_t value)
{
	SASSERT(buf);
	TryResize(buf);

	uint16_t swappedValue = (IsSystemLittleEndian()) ? SwapU16(value) : value;
	size_t size = sizeof(short);
	SMemCopy(buf->Data + buf->Position, &swappedValue, size);
	buf->Position += size;
}

uint16_t ReadUShort(SBuffer* buf)
{
	SASSERT(buf);

	uint16_t value;
	size_t size = sizeof(uint16_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return (IsSystemLittleEndian()) ? SwapU16(value) : value;
}

void WriteInt(SBuffer* buf, int32_t value)
{
	SASSERT(buf);
	TryResize(buf);

	int32_t swappedValue = (IsSystemLittleEndian()) ? SwapI32(value) : value;
	size_t size = sizeof(int);
	SMemCopy(buf->Data + buf->Position, &swappedValue, size);
	buf->Position += size;
}

int32_t ReadInt(SBuffer* buf)
{
	SASSERT(buf);

	int32_t value;
	size_t size = sizeof(int32_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return (IsSystemLittleEndian()) ? SwapI32(value) : value;
}

void WriteUInt(SBuffer* buf, uint32_t value)
{
	SASSERT(buf);
	TryResize(buf);

	uint32_t swappedValue = (IsSystemLittleEndian()) ? SwapU32(value) : value;
	size_t size = sizeof(uint32_t);
	SMemCopy(buf->Data + buf->Position, &swappedValue, size);
	buf->Position += size;
}

uint32_t ReadUInt(SBuffer* buf)
{
	SASSERT(buf);

	uint32_t value;
	size_t size = sizeof(uint32_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return (IsSystemLittleEndian()) ? SwapU32(value) : value;
}

void WriteInt64(SBuffer* buf, int64_t value)
{
	SASSERT(buf);
	TryResize(buf);

	int64_t swappedValue = (IsSystemLittleEndian()) ? SwapI64(value) : value;
	size_t size = sizeof(int64_t);
	SMemCopy(buf->Data + buf->Position, &swappedValue, size);
	buf->Position += size;
}

int64_t ReadInt64(SBuffer* buf)
{
	SASSERT(buf);

	int64_t value;
	size_t size = sizeof(int64_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return (IsSystemLittleEndian()) ? SwapI64(value) : value;
}

void WriteUInt64(SBuffer* buf, uint64_t value)
{
	SASSERT(buf);
	TryResize(buf);

	uint64_t swappedValue = (IsSystemLittleEndian()) ? SwapU64(value) : value;
	size_t size = sizeof(uint64_t);
	SMemCopy(buf->Data + buf->Position, &swappedValue, size);
	buf->Position += size;
}

uint64_t ReadUInt64(SBuffer* buf)
{
	SASSERT(buf);

	uint64_t value;
	size_t size = sizeof(uint64_t);
	SMemCopy(&value, buf->Data + buf->Position, size);
	buf->Position += size;
	return (IsSystemLittleEndian()) ? SwapU64(value) : value;
}
