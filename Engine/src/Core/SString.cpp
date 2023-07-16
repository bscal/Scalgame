#include "SString.h"

static_assert(sizeof(char) == sizeof(uint8_t), "SString does not support char size > 1.");
static_assert(sizeof(SString) == 24, "SString should equal 24.");
static_assert((uint8_t)SAllocator::MaxTypes < 0xff, "Only 8 bytes to store allocator in string");

// *****************
// *    SString    *
// *****************

SString::SString(SAllocator allocator) 
	: Length(0), Capacity(0), Allocator((uint32_t)allocator), m_Buffer({})
{
}

SString::SString(const char* str)
	: SString(str, (uint32_t)strlen(str), SAllocator::Game)
{
}

SString::SString(const char* str, uint32_t length, SAllocator allocator)
	: Length(length), Capacity(SSTR_SSO_ARRAY_SIZE), Allocator((uint32_t)allocator)
{
	if (Length > 0)
	{
		uint32_t terminatedLength = Length + 1;
		if (terminatedLength > Capacity) // Allocate
		{
			Capacity = terminatedLength;
			m_Buffer.StrMemory = (char*)SAlloc(Allocator, Capacity, MemoryTag::Strings);
			SMemCopy(m_Buffer.StrMemory, str, Length);
			m_Buffer.StrMemory[Length] = '\0';
		}
		else // Use short string optimization
		{
			SMemCopy(m_Buffer.ShortStringBuf, str, Length);
			m_Buffer.ShortStringBuf[Length] = '\0';
		}
	}
	else
		m_Buffer.ShortStringBuf[0] = '\0';
}

SString::SString(const SString& other)
	: SString(other.Data(), other.Length, (SAllocator)other.Allocator)
{
}

SString::SString(SString&& other) noexcept
{
	if (this == &other)
		return;

	SMemCopy(this, &other, sizeof(SString));
	SMemClear(&other, sizeof(SString));
}

SString::~SString()
{
	if (Capacity > SSTR_SSO_ARRAY_SIZE && (SAllocator)Allocator != SAllocator::Temp)
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
SString::Assign(const char* cStr, uint32_t length, SAllocator allocator)
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

void SString::Assign(const char* cStr, SAllocator allocator)
{
	Assign(cStr, (uint32_t)strlen(cStr), allocator);
}

void SString::Assign(const SString& other, SAllocator allocator)
{
	Assign(other.Data(), other.Length, allocator);
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

uint32_t SString::Substring(char delimiter, uint32_t offset) const
{
	uint32_t idx = SSTR_NO_POS;

	if (Empty())
	{
		return SSTR_NO_POS;
	}

	const char* data = Data();

	for (uint32_t i = offset; i < Length; ++i)
	{
		char c = data[i];
		if (c == delimiter)
		{
			idx = i;
			break;
		}
	}

	return idx;
}

SString SString::FirstNew(char delimiter, uint32_t offset) const
{
	uint32_t idx = Substring(delimiter, offset);
	
	return (idx == SSTR_NO_POS) ? SString() : Slice(0, idx, SAllocator::Game);
}

SString SString::FirstTemp(char delimiter, uint32_t offset) const
{
	uint32_t idx = Substring(delimiter, offset);

	return (idx == SSTR_NO_POS) ? SString() : Slice(0, idx, SAllocator::Temp);
}

SString SString::Slice(uint32_t start, uint32_t end, SAllocator allocator) const
{
	SASSERT(start < end);

	size_t size = end - start;
	SString string = SString(allocator);

	if (size > 0)
	{
		string.SetCapacity(size);
		string.Length = size;
		SMemCopy(string.Data(), Data() + start, size);
	}
	
	return string;
}

SString& SString::operator=(const SString& other)
{
	if (this != &other)
		Assign(other, (SAllocator)other.Allocator);
	return *this;
}

//SString& SString::operator=(const SStringView& other)
//{
//	if (Data() != other.Str)
//		Assign(other.Str);
//	return *this;
//}

SString& SString::operator=(const char* cString)
{
	Assign(cString, SAllocator::Game);
	return *this;
}

void SString::Append(const char* str)
{
	Append(str, (uint32_t)strlen(str));
}

void SString::Append(const char* str, uint32_t length)
{
	uint32_t newLength = Length + length + 1; // +1 for null terminator
	if (newLength > Capacity)
		SetCapacity(newLength);

	char* offsetData = Data() + Length;
	SMemCopy(offsetData, str, length);
	Length += length;

	Data()[Length] = '\0';
}

// *********************
// *    SStringView    *
// *********************

//SStringView::SStringView(const char* str)
//	: SStringView(str, (uint32_t)strlen(str))
//{
//}
//
//SStringView::SStringView(const char* str, uint32_t length)
//	: SStringView((char*)str, length, 0)
//{
//}
//
//SStringView::SStringView(const char* str, uint32_t length, uint32_t offset)
//	: Str(str + offset), Length(length)
//{
//}
//
//SStringView::SStringView(const SStringView* string, uint32_t offset)
//	: SStringView(string->Str, string->Length, offset)
//{
//}
//
//SStringView::SStringView(const SString* string)
//	: SStringView(string->Data(), string->Length - 1, 0)
//{
//}
//
//SStringView SStringView::SubString(uint32_t start, uint32_t end) const
//{
//	return SStringView(Str, end - start, start);
//}
//
//uint32_t SStringView::FindChar(char c) const
//{
//	return FindChar(c, 0);
//}
//
//uint32_t SStringView::FindChar(char c, uint32_t start) const
//{
//	SASSERT(Str);
//	SASSERT(start < Length);
//	for (uint32_t i = start; i < Length; ++i)
//	{
//		char character = Str[i];
//		if (character == '\0') break;
//		if (character == c) return i;
//	}
//	return SSTR_NO_POS;
//}
//
//uint32_t SStringView::Find(const char* cString) const
//{
//	SASSERT(cString);
//	const char* foundPtr = strstr(Str, cString);
//	SASSERT(!foundPtr || ((foundPtr - Str) >= 0 && (foundPtr - Str) < UINT32_MAX))
//	return (foundPtr) ? uint32_t(foundPtr - Str) : SSTR_NO_POS;
//}

// **************************
// *    String Functions    *
// **************************

SRawString RawStringNew(const char* cStr, SAllocator allocator)
{
	return RawStringNew(cStr, strlen(cStr), allocator);
}

SRawString RawStringNew(const char* cStr, uint32_t length, SAllocator allocator)
{
	SASSERT(cStr);
	SASSERT(length > 0);

	SRawString res;

	res.Length = length;
	res.Allocator = allocator;

	res.Data = (char*)SAlloc(res.Allocator, res.Length + 1, MemoryTag::Strings);
	
	SMemCopy(res.Data, cStr, res.Length);

	res.Data[res.Length] = '\0';

	return res;
}

void RawStringFree(SRawString* string)
{
	SFree(string->Allocator, string->Data, string->Length + 1, MemoryTag::Strings);

	string->Data = nullptr;
	string->Length = 0;
}

uint32_t StrFind(const char* str, const char* find)
{
	SASSERT(str);
	SASSERT(find);
	int text = TextFindIndex(str, find);
	return (text == -1) ? SSTR_NO_POS : (uint32_t)text;
}

bool SStrEquals(const char* str1, const char* str2)
{
	if (!str1 || !str2)
		return false;
	if (str1 == str2)
		return true;
	return (strcmp(str1, str2) == 0);
}

void SStringsBuffer::Initialize(uint32_t stringCount, uint32_t stringCapacity)
{
	SASSERT(stringCount > 0);
	SASSERT(stringCapacity > 0);

	StringCount = stringCount;
	StringCapacity = stringCapacity;

	size_t size = sizeof(char*) * stringCount;
	size_t stride = sizeof(char) * stringCapacity;

	StringsMemory = (char**)SAlloc(SAllocator::Game, size, MemoryTag::Arrays);

	for (size_t i = 0; i < StringCount; ++i)
	{
		StringsMemory[i] = (char*)SAlloc(SAllocator::Game, stride, MemoryTag::Strings);
		SMemClear(StringsMemory[i], stride);
	}
}

void SStringsBuffer::Free()
{
	size_t size = sizeof(char*) * StringCount;
	size_t stride = sizeof(char) * StringCapacity;

	for (size_t i = 0; i < StringCount; ++i)
	{
		SFree(SAllocator::Game, StringsMemory[i], stride, MemoryTag::Strings);
	}
	SFree(SAllocator::Game, StringsMemory, size, MemoryTag::Arrays);
	StringsMemory = 0;
}

char* SStringsBuffer::Push()
{
	SASSERT(StringsMemory);
	SASSERT(StringCount > 0);
	SASSERT(StringCapacity > 0);

	char* last = StringsMemory[StringCount - 1];
	SMemClear(last, StringCapacity);

	void** dst = (void**)(StringsMemory + 1);
	void** src = (void**)(StringsMemory);
	size_t size = (StringCount - 1) * sizeof(char*);

	SMemMove(dst, src, size);

	StringsMemory[0] = last;

	return last;
}
