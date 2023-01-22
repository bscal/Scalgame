#pragma once

#include "SMemory.h"

#include <stdint.h>

#define SHORT_STRING_ARRAY_SIZE 16
#define SHORT_STRING_LENGTH (SHORT_STRING_ARRAY_SIZE - 1)

bool SStrEquals(const char* str0, const char* str1);

struct STempString;
struct SStringView;

struct SString
{
	union StringMemory
	{
		char* StrMemory;
		char ShortStringBuf[SHORT_STRING_ARRAY_SIZE];
	};

	uint32_t Length;

	SString();
	SString(const char* str);
	SString(const char* str, uint32_t length);
	SString(const SString& other);
	
	~SString();

	SString& operator=(const SString& other);
	SString& operator=(const char* cString);

	inline bool operator==(const SString& other) const { return SStrEquals(Data(), other.Data()); }
	inline bool operator!=(const SString& other) const { return !SStrEquals(Data(), other.Data()); }
	bool operator==(const STempString& other) const;
	bool operator!=(const STempString& other) const;
	inline bool operator==(const char* other) const { return SStrEquals(Data(), other); }
	inline bool operator!=(const char* other) const { return !SStrEquals(Data(), other); }

	inline const char* Data() const
	{
		return (Length > SHORT_STRING_LENGTH) ? m_Buffer.StrMemory : m_Buffer.ShortStringBuf;
	}

	inline char* Data()
	{
		return (Length > SHORT_STRING_LENGTH) ? m_Buffer.StrMemory : m_Buffer.ShortStringBuf;
	}

private:
	StringMemory m_Buffer;
};

struct STempString
{
	char* Str;
	uint32_t Length;

	STempString() = default;
	STempString(const char* str);
	STempString(const char* str, uint32_t length);
	STempString(const STempString& other);

	STempString& operator=(const STempString& other);
	STempString& operator=(const char* cString);

	inline bool operator==(const SString& other) const { return SStrEquals(Str, other.Data()); }
	inline bool operator!=(const SString& other) const { return !SStrEquals(Str, other.Data()); }
	inline bool operator==(const STempString& other) const { return SStrEquals(Str, other.Str); }
	inline bool operator!=(const STempString& other) const { return !SStrEquals(Str, other.Str); }
	inline bool operator==(const char* other) const { return SStrEquals(Str, other); }
	inline bool operator!=(const char* other) const { return !SStrEquals(Str, other); }
};

struct SStringView
{
	const char* Str;
	uint32_t Length;

	SStringView() = default;
	SStringView(const char* str, uint32_t length);
	SStringView(const SString* string);
};

inline void TestStringImpls()
{
	SString string0 = {};
	SASSERT(sizeof(SString) == 24);
	SASSERT(string0.Data());
	SASSERT(string0.Data()[0] == 0);

	SString string1 = "Literal";
	SASSERT(string1 == "Literal");
	SASSERT(string1 != "NotThis");
	SASSERT(string1 != string0);
	
	{
		SString string1Copy = string1;
		SASSERT(string1Copy == string1);
	}
	SASSERT(string1.Data());

	SString string2("Big long string test wow!!!!!");
	SASSERT(string2 == "Big long string test wow!!!!!");
	SASSERT(string2.Length == 30);

	STempString tempString = "Testing Temporary!!";
	SASSERT(tempString == "Testing Temporary!!")

	SStringView view(&string2);
	SASSERT(SStrEquals(view.Str, string2.Data()));
	SASSERT(view.Str == string2.Data());

	SLOG_INFO("[ Test ] String test passed!");
}
