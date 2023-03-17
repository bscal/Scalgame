#include "SString.h"

#include <string.h>

static_assert(sizeof(char) == 1, "SString does not support char size > 1.");
static_assert(sizeof(SString) == 24, "SString should equal 24.");

// *****************
// *    SString    *
// *****************

// Info: Most constructors or functions that create/assign strings,
// will add a null terminator to the end just to be safe.
//
// - SString and STempString ARE null terminated strings.
// - SStringView ARE NOT null terminated
//

SString::SString(const char* str)
	: SString(str, strlen(str) + 1)
{
}

SString::SString(const char* str, uint32_t length)
{
	SASSERT(length != 0);
	SASSERT_MSG(str[length - 1] == 0, "str should be null terminated.");
	Length = length;
	DoNotFree = false;
	Capacity = SSTR_SSO_LENGTH;
	if (Length > Capacity)
	{
		Capacity = Length;
		m_Buffer.StrMemory = (char*)SAlloc(SAllocator::Game, Capacity, MemoryTag::Strings);
	}
	SMemCopy(Data(), str, End());
	Data()[End()] = '\0';
}

SString::SString(const SString& other)
	: SString(other.Data(), other.Length)
{
	DoNotFree = other.DoNotFree;
}

SString::~SString()
{
	if (!DoNotFree && Capacity > SSTR_SSO_LENGTH)
		SFree(SAllocator::Game, m_Buffer.StrMemory, Capacity, MemoryTag::Strings);
}

SString SString::CreateFake(const STempString* tempStr)
{
	SASSERT(tempStr);
	SASSERT(tempStr->Str);

	SString string;
	string.Length = tempStr->Length;
	string.DoNotFree = true;
	if (string.Length > SSTR_SSO_LENGTH)
	{
		string.Capacity = string.Length;
		string.m_Buffer.StrMemory = tempStr->Str;
	}
	else
	{
		string.Capacity = SSTR_SSO_LENGTH;
		SMemCopy(string.Data(), tempStr->Str, string.Length);
		SASSERT(string.Data()[SSTR_SSO_LENGTH] == 0);
		string.Data()[SSTR_SSO_LENGTH] = '\0';
	}
	return string;
}

SString SString::CreateFake(const SStringView* tempStr)
{
	SASSERT(tempStr);
	SASSERT(tempStr->Str);

	SString string;
	string.Length = tempStr->Length + 1;
	string.DoNotFree = true;
	if (string.Length > SSTR_SSO_LENGTH)
	{
		string.Capacity = string.Length;
		// Note: const -> unconst, this points to another
		// strin's memory a not a unique memory
		string.m_Buffer.StrMemory = (char*)tempStr->Str;
	}
	else
	{
		string.Capacity = SSTR_SSO_LENGTH;
		SMemCopy(string.Data(), tempStr->Str, string.Length - 1);
	}
	string.Data()[string.End()] = '\0';
	string.Data()[SSTR_SSO_LENGTH] = '\0';
	SASSERT(string.Data()[SSTR_SSO_LENGTH] == 0);
	return string;
}

void SString::SetCapacity(uint32_t capacity)
{
	if (capacity < Capacity) return;
	if (capacity > SSTR_SSO_LENGTH)
	{
		if (Capacity > SSTR_SSO_LENGTH)
		{
			SRealloc(SAllocator::Game, m_Buffer.StrMemory, Capacity, capacity, MemoryTag::Strings);
		}
		else
		{
			char* memory = (char*)SAlloc(SAllocator::Game, capacity, MemoryTag::Strings);
			SMemCopy(memory, Data(), SSTR_SSO_LENGTH);
			m_Buffer.StrMemory = memory;
		}
		Capacity = capacity;
	}
	else
	{
		Capacity = SSTR_SSO_LENGTH;
	}
}

void SString::Assign(const char* cStr)
{
	SASSERT(cStr);
	Assign(cStr, strlen(cStr) + 1);
}

void SString::Assign(const char* cStr, uint32_t length)
{
	SASSERT(cStr);
	if (Data() == cStr) return;
	if (length == 0)
	{
		Data()[0] = '\0';
		Length = 0;
		return;
	}
	if (cStr[length - 1] != '\0') ++length;
	if (length > SSTR_SSO_LENGTH)
	{
		if (Capacity > SSTR_SSO_LENGTH)
			m_Buffer.StrMemory = (char*)SRealloc(SAllocator::Game, m_Buffer.StrMemory, Capacity, length, MemoryTag::Strings);
		else
			m_Buffer.StrMemory = (char*)SAlloc(SAllocator::Game, length, MemoryTag::Strings);
		Capacity = length;
	}
	else if (Capacity == 0)
		Capacity = SSTR_SSO_LENGTH;

	Length = length;
	DoNotFree = false;
	SMemCopy(Data(), cStr, End());
	Data()[End()] = '\0';
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
	return (foundPtr) ? foundPtr - Data() : SSTR_NO_POS;
}

SString& SString::operator=(const SString& other)
{
	if (this != &other) Assign(other.Data(), other.Length);
	return *this;
}

SString& SString::operator=(const char* cString)
{
	Assign(cString);
	return *this;
}

bool SString::operator==(const STempString& other) const { return SStrEquals(Data(), other.Str); }
bool SString::operator!=(const STempString& other) const { return !SStrEquals(Data(), other.Str); }

void SString::Append(const char* str)
{
	Append(str, strlen(str) + 1);
}

void SString::Append(const char* str, uint32_t length)
{
	if (Length + length > Capacity)
	{
		SetCapacity(Length + length);
	}
	uint32_t offset = (Length == 0) ? 0 : Length - 1;
	SMemMove(Data() + offset, str, length);
	Length = offset + length;
}

// *********************
// *    STempString    *
// *********************

STempString::STempString(const char* str)
	: STempString(str, strlen(str) + 1)
{
}

STempString::STempString(const char* str, uint32_t length)
{
	Length = length;
	Str = (char*)SMemTempAlloc(Length);
	SMemCopy(Str, str, Length - 1);
	Str[End()] = '\0';
}

STempString::STempString(const STempString& other)
	: STempString(other.Str, other.Length)
{
}

STempString& STempString::operator=(const STempString& other)
{
	if (this == &other) return *this;

	Length = other.Length;
	Str = (char*)SMemTempAlloc(Length);
	SMemCopy(Str, other.Str, Length);
	Str[Length - 1] = '\0';

	return *this;
}

STempString& STempString::operator=(const char* cString)
{
	Length = strlen(cString) + 1;
	Str = (char*)SMemTempAlloc(Length);
	SMemCopy(Str, cString, Length);
	Str[Length - 1] = '\0';
	return *this;
}

// *********************
// *    SStringView    *
// *********************

SStringView::SStringView(const char* str)
	: SStringView(str, strlen(str))
{
}

SStringView::SStringView(const char* str, uint32_t length)
	: SStringView((char*)str, length, 0)
{
}

SStringView::SStringView(const char* str, uint32_t length, uint32_t offset)
{
	if (length == 0)
	{
		Str = (char*)str;
		Length = length;
	}
	else
	{
		Str = (char*)str + offset;
		Length = (Str[length - 1] == '\0') ? length - 1 : length;
		SASSERT(Str < Str + Length);
	}
}

SStringView::SStringView(const SStringView* string, uint32_t offset)
	: SStringView(string->Str, string->Length - 1, offset)
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
		if (!character) break;
		if (character == c) return i;
	}
	return SSTR_NO_POS;
}

uint32_t SStringView::Find(const char* cString) const
{
	SASSERT(cString);
	const char* foundPtr = strstr(Str, cString);
	return (foundPtr) ? foundPtr - Str : SSTR_NO_POS;
}

// **************************
// *    String Functions    *
// **************************

bool SStrEquals(const char* str1, const char* str2)
{
	// NOTE: I am not sure what I want to do about
	// strings with null pointers. So right now, if
	// either are null just return false
	if (!str1 || !str2) return false;
	if (str1 == str2) return true;
	return (strcmp(str1, str2) == 0);
}
