#include "SString.h"

#include <string.h>

static_assert(sizeof(char) == 1, "SString does not support char size > 1.");
static_assert(sizeof(SString) == 24, "SString should equal 24.");

SString::SString()
{
	Length = 0;
	IsTemp = false;
	m_Buffer.StrMemory = NULL;
}

SString::SString(const char* str)
	: SString(str, strlen(str) + 1)
{
}

SString::SString(const char* str, uint32_t length)
{
	Length = length;
	IsTemp = false;
	uint32_t last = SHORT_STRING_LENGTH;
	if (Length > SHORT_STRING_LENGTH)
	{
		last = Length;
		m_Buffer.StrMemory = (char*)SMemAllocTag(Length, MemoryTag::Strings);
	}
	SMemCopy(Data(), str, Length);
	Data()[last] = '\0';
}

SString::SString(const SString& other)
	: SString(other.Data(), other.Length)
{
}

SString SString::CreateTemp(const STempString* tempStr)
{
	SASSERT(tempStr);
	SASSERT(tempStr->Str);

	SString string;
	string.Length = tempStr->Length;
	string.IsTemp = true;
	if (string.Length > SHORT_STRING_LENGTH)
	{
		string.m_Buffer.StrMemory = tempStr->Str;
	}
	else
	{
		SMemCopy(string.Data(), tempStr->Str, string.Length);
		string.Data()[SHORT_STRING_LENGTH] = '\0';
	}
	return string;
}

SString SString::CreateTemp(const SStringView* tempStr)
{
	SASSERT(tempStr);
	SASSERT(tempStr->Str);

	SString string;
	string.Length = tempStr->Length;
	string.IsTemp = true;
	if (string.Length > SHORT_STRING_LENGTH)
	{
		string.m_Buffer.StrMemory = tempStr->Str;
	}
	else
	{
		SMemCopy(string.Data(), tempStr->Str, string.Length);
		string.Data()[SHORT_STRING_LENGTH] = '\0';
	}
	return string;
}

SString& SString::operator=(const SString& other)
{
	if (this == &other) return *this;
	Length = other.Length;
	if (Length > SHORT_STRING_LENGTH)
	{
		m_Buffer.StrMemory = (char*)SMemAllocTag(Length, MemoryTag::Strings);
	}
	SMemCopy(Data(), other.Data(), Length);
	return *this;
}

SString& SString::operator=(const char* cString)
{
	Length = strlen(cString) + 1;
	if (Length > SHORT_STRING_LENGTH)
	{
		m_Buffer.StrMemory = (char*)SMemAllocTag(Length, MemoryTag::Strings);
	}
	SMemCopy(Data(), cString, Length);
	return *this;
}

bool SString::operator==(const STempString& other) const { return SStrEquals(Data(), other.Str); }
bool SString::operator!=(const STempString& other) const { return !SStrEquals(Data(), other.Str); }

SString::~SString()
{
	if (!IsTemp && Length > SHORT_STRING_LENGTH)
	{
		SMemFreeTag(m_Buffer.StrMemory, Length, MemoryTag::Strings);
	}
}

STempString::STempString(const char* str)
	: STempString(str, strlen(str) + 1)
{
}

STempString::STempString(const char* str, uint32_t length)
{
	Length = length;
	uint32_t termiatedLength = Length + 1;
	Str = (char*)SMemTempAlloc(termiatedLength);
	SMemCopy(Str, str, termiatedLength);
	Str[Length] = '\0';
}

STempString::STempString(const STempString& other)
	: STempString(other.Str, other.Length)
{
}

STempString& STempString::operator=(const STempString& other)
{
	if (this == &other) return *this;

	Length = other.Length;

	uint32_t termiatedLength = Length + 1;
	Str = (char*)SMemTempAlloc(termiatedLength);
	SMemCopy(Str, other.Str, termiatedLength);
	Str[Length] = '\0';

	return *this;
}

STempString& STempString::operator=(const char* cString)
{
	Length = strlen(cString) + 1;
	uint32_t termiatedLength = Length + 1;
	Str = (char*)SMemTempAlloc(termiatedLength);
	SMemCopy(Str, cString, termiatedLength);
	Str[Length] = '\0';
	return *this;
}

SStringView::SStringView(const char* str)
	:SStringView(str, strlen(str) + 1)
{
}

SStringView::SStringView(const char* str, uint32_t length)
	: Str((char*)str), Length(length)
{
}

SStringView::SStringView(const char* str, uint32_t length, uint32_t offset)
{
	SASSERT(offset < length);
	Str = (char*)str + offset;
	Length = length;
}

SStringView::SStringView(const SStringView* string, uint32_t offset)
{
	SASSERT(offset < string->Length);
	Str = (char*)string->Str + offset;
	Length = string->Length - offset;
}

SStringView::SStringView(const SString* string)
	: Str((char*)string->Data()), Length(string->Length)
{
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
