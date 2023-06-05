#include "SString.h"

#include <string.h>
#include <utility>

static_assert(sizeof(char) == 1, "SString does not support char size > 1.");
static_assert(sizeof(SString) == 32, "SString should equal 32.");

// *****************
// *    SString    *
// *****************

SString::SString(SAllocator allocator) 
	: m_Buffer({}), Length(0), Capacity(0), Allocator(allocator)
{
}

SString::SString(const char* str, uint32_t length)
	: Length(length), Capacity(SSTR_SSO_ARRAY_SIZE), Allocator(SAllocator::Game)
{
	SASSERT(length != 0);
	uint32_t terminatedLength = length + 1;
	if (terminatedLength > Capacity)
	{
		Capacity = terminatedLength;
		m_Buffer.StrMemory = (char*)SAlloc(Allocator, Capacity, MemoryTag::Strings);
	}
	SMemCopy(Data(), str, Length);
	Data()[Length] = '\0';
}

SString::SString(const char* str)
	: SString(str, (uint32_t)strlen(str))
{
}

SString::SString(const SString& other)
	: SString(other.Data(), other.Length)
{
}

SString::SString(SString&& other) noexcept
{
	Length = other.Length;
	Capacity = other.Capacity;
	Allocator = other.Allocator;
	m_Buffer = other.m_Buffer;

	SMemClear(&other.m_Buffer, sizeof(StringMemory));
	other.Length = 0;
	other.Capacity = 0;
}

SString::~SString()
{
	if (Capacity > SSTR_SSO_ARRAY_SIZE)
		SFree(Allocator, m_Buffer.StrMemory, Capacity, MemoryTag::Strings);
}

void SString::SetCapacity(uint32_t capacity)
{
	if (Capacity >= capacity)
		return;

	if (capacity <= SSTR_SSO_ARRAY_SIZE)
	{
		Capacity = SSTR_SSO_ARRAY_SIZE;
	}
	else
	{
		if (IsAllocated())
		{
			m_Buffer.StrMemory = (char*)SRealloc(SAllocator::Game, m_Buffer.StrMemory, Capacity, capacity, MemoryTag::Strings);
		}
		else
		{
			char* memory = (char*)SAlloc(Allocator, capacity, MemoryTag::Strings);
			SMemMove(memory, m_Buffer.ShortStringBuf, SSTR_SSO_ARRAY_SIZE);
			m_Buffer.StrMemory = memory;
		}
		Capacity = capacity;
	}
}

void
SString::Assign(const char* cStr, uint32_t length)
{
	SASSERT(cStr);

	if (Capacity > SSTR_SSO_ARRAY_SIZE)
		SFree(Allocator, m_Buffer.StrMemory, Capacity, MemoryTag::Strings);

	if (length == 0)
	{
		SMemClear(&m_Buffer, sizeof(SString::StringMemory));
		Length = 0;
		Capacity = 0;
		return;
	}

	if (Data() == cStr)
		return;

	Length = length;
	uint32_t terminatedLength = Length + 1;
	Capacity = SSTR_SSO_ARRAY_SIZE;
	if (terminatedLength > Capacity)
	{
		Capacity = terminatedLength;
		m_Buffer.StrMemory = (char*)SAlloc(Allocator, Capacity, MemoryTag::Strings);
	}
	SMemCopy(Data(), cStr, Length);
	Data()[Length] = '\0';
}

void SString::Assign(const char* cStr)
{
	Assign(cStr, (uint32_t)strlen(cStr));
}

void SString::Assign(const SString& other)
{
	Assign(other.Data(), other.Length);
}

uint32_t SString::FindChar(char c) const
{
	SASSERT(Data());
	for (uint32_t i = 0; i < Length; ++i)
	{
		char character = Data()[i];
		if (!character) break;
		if (character == c) return i;
	}
	return SSTR_NO_POS;
}

uint32_t SString::Find(const char* cString) const
{
	SASSERT(cString);
	const char* foundPtr = strstr(Data(), cString);
	return (foundPtr) ? static_cast<uint32_t>(foundPtr - Data()) : SSTR_NO_POS;
}

SString& SString::operator=(const SString& other)
{
	if (this != &other)
		Assign(other);
	return *this;
}

SString& SString::operator=(const SStringView& other)
{
	if (Data() != other.Str)
		Assign(other.Str);
	return *this;
}

SString& SString::operator=(const char* cString)
{
	Assign(cString);
	return *this;
}

void SString::Append(const char* str)
{
	Append(str, (uint32_t)strlen(str));
}

void SString::Append(const char* str, uint32_t length)
{
	uint32_t newLength = Length + length + 1;
	if (newLength > Capacity)
		SetCapacity(newLength);

	char* offsetData = Data() + Length;
	SMemMove(offsetData, str, length);
	Length += length;

	Data()[Length] = '\0';
}

// *********************
// *    SStringView    *
// *********************

SStringView::SStringView(const char* str)
	: SStringView(str, (uint32_t)strlen(str))
{
}

SStringView::SStringView(const char* str, uint32_t length)
	: SStringView((char*)str, length, 0)
{
}

SStringView::SStringView(const char* str, uint32_t length, uint32_t offset)
	: Str(str + offset), Length(length)
{
}

SStringView::SStringView(const SStringView* string, uint32_t offset)
	: SStringView(string->Str, string->Length, offset)
{
}

SStringView::SStringView(const SString* string)
	: SStringView(string->Data(), string->Length, 0)
{
}

SStringView SStringView::SubString(uint32_t start, uint32_t end) const
{
	return SStringView(Str, end - start, start);
}

uint32_t SStringView::FindChar(char c) const
{
	return FindChar(c, 0);
}

uint32_t SStringView::FindChar(char c, uint32_t start) const
{
	SASSERT(Str);
	SASSERT(start < Length);
	for (uint32_t i = start; i < Length; ++i)
	{
		char character = Str[i];
		if (character == '\0') break;
		if (character == c) return i;
	}
	return SSTR_NO_POS;
}

uint32_t SStringView::Find(const char* cString) const
{
	SASSERT(cString);
	const char* foundPtr = strstr(Str, cString);
	SASSERT(!foundPtr || ((foundPtr - Str) >= 0 && (foundPtr - Str) < UINT32_MAX))
	return (foundPtr) ? uint32_t(foundPtr - Str) : SSTR_NO_POS;
}

// **************************
// *    String Functions    *
// **************************

SRawString RawStringNew(const char* cStr)
{
	SRawString res;
	res.Length = (uint32_t)strlen(cStr);
	res.Data = (char*)SAlloc(SAllocator::Game, res.Length + 1, MemoryTag::Strings);
	SMemCopy(res.Data, cStr, res.Length);
	res.Data[res.Length] = '\0';
	return res;
}

SRawString TempRawString(const char* cStr, uint32_t length)
{
	SRawString res;
	res.Length = length;
	res.Data = (char*)SAlloc(SAllocator::Temp, res.Length + 1, MemoryTag::Strings);
	SMemCopy(res.Data, cStr, res.Length);
	res.Data[res.Length] = '\0';
	return res;
}

void RawStringFree(SRawString* string)
{
	SFree(SAllocator::Game, string->Data, string->Length + 1, MemoryTag::Strings);
	string->Data = nullptr;
	string->Length = 0;
}

bool SStrEquals(const char* str1, const char* str2)
{
	// NOTE: I am not sure what I want to do about
	// strings with null pointers. So right now, if
	// either are null just return false
	if (!str1 || !str2) return false;
	if (str1 == str2) return true;
	return (strcmp(str1, str2) == 0);
}

void SStringsBuffer::Initialize(uint32_t poolCapacity, uint32_t stringCapacity)
{
	SASSERT(poolCapacity > 0);
	SASSERT(stringCapacity > 0);

	PoolCapacity = poolCapacity;
	StringStride = sizeof(char) * stringCapacity;
	Head = 0;
	 
	size_t arraySize = StringStride * PoolCapacity;
	StringsMemory = (char*)SAlloc(SAllocator::Game, arraySize, MemoryTag::Strings);
	SMemClear(StringsMemory, arraySize);
}

void SStringsBuffer::Free()
{
	size_t arraySize = StringStride * PoolCapacity;
	SFree(SAllocator::Game, StringsMemory, arraySize, MemoryTag::Strings);
}

void SStringsBuffer::Clear()
{
	if (StringsMemory)
	{
		size_t arraySize = StringStride * PoolCapacity;
		SMemClear(StringsMemory, arraySize);
	}
}

char* SStringsBuffer::Next()
{
	SASSERT(StringsMemory);

	uint32_t head = Head;
	Head = (Head + 1) % PoolCapacity;

	uint32_t offset = head * StringStride;
	char* ptr = StringsMemory + offset;
	SASSERT(ptr >= StringsMemory);
	SASSERT(ptr < (StringsMemory + (StringStride * PoolCapacity)));
	return ptr;
}

void SStringsBuffer::Copy(const char* string)
{
	memcpy(Next(), string, StringStride);
}

char* SStringsBuffer::Get(uint32_t idx)
{
	SASSERT(StringsMemory);

	uint32_t offset = idx * StringStride;
	char* ptr = StringsMemory + offset;
	SASSERT(ptr >= StringsMemory);
	SASSERT(ptr < (StringsMemory + (StringStride * PoolCapacity)));
	return ptr;
}
