#pragma once

#include "Core.h"
#include "SMemory.h"

#include <stdint.h>
#include <string.h>

global_var constexpr uint32_t SSTR_SSO_ARRAY_SIZE = 16;
global_var constexpr uint32_t SSTR_SSO_LENGTH = SSTR_SSO_ARRAY_SIZE - 1;

global_var constexpr uint32_t SSTR_NO_POS = UINT32_MAX - 1;

bool SStrEquals(const char* str0, const char* str1);

struct STempString;
struct SStringView;

struct SString
{
	union StringMemory
	{
		char* StrMemory;
		char ShortStringBuf[SSTR_SSO_ARRAY_SIZE];
	};

	uint32_t Length : 31, DoNotFree: 1;
	uint32_t Capacity;

	SString() = default;
	SString(const char* str);
	SString(const char* str, uint32_t length);
	SString(const SString& other);

	// If not SSOed then frees
	~SString();

	// Note: CreateFake is used if you want to create a
	// SString to compare in a map, but not worry about
	// allocations, like a hashmap lookup. These will use,
	// SSO and copy, but if larger will just point to the
	// pointer of the input string
	static SString CreateFake(const STempString* tempStr);
	static SString CreateFake(const SStringView* tempStr);

	void SetCapacity(uint32_t capacity);

	void Assign(const char* cStr);
	void Assign(const char* cStr, uint32_t length);

	void Append(const char* str);
	void Append(const char* str, uint32_t length);

	uint32_t FindChar(char c) const;
	uint32_t Find(const char* cString) const;

	SString& operator=(const SString& other);
	SString& operator=(const char* cString);

	inline bool operator==(const SString& other) const { return SStrEquals(Data(), other.Data()); }
	inline bool operator!=(const SString& other) const { return !SStrEquals(Data(), other.Data()); }
	bool operator==(const STempString& other) const;
	bool operator!=(const STempString& other) const;
	inline bool operator==(const char* other) const { return SStrEquals(Data(), other); }
	inline bool operator!=(const char* other) const { return !SStrEquals(Data(), other); }

	friend SString operator+(const SString& lhs, const SString& rhs)
	{
		SString str(lhs);
		str.Append(rhs.Data(), rhs.Length);
		return str;
	}

	friend SString operator+(const SString& lhs, const char* rhs)
	{
		SString str(lhs);
		size_t length = strlen(rhs) - 1;
		SASSERT(length > 0);
		str.Append(rhs, static_cast<uint32_t>(length));
		return str;
	}

	inline const char* Data() const
	{
		return (Length > SSTR_SSO_LENGTH) ? m_Buffer.StrMemory : m_Buffer.ShortStringBuf;
	}

	inline char* Data()
	{
		return (Length > SSTR_SSO_LENGTH) ? m_Buffer.StrMemory : m_Buffer.ShortStringBuf;
	}

	inline bool Empty() const { return Length == 0; }
	inline uint32_t End() const { return Length - 1; }

private:
	// NOTE: Not sure if I want to
	// have this private? Not a fan but
	// calling Data() always is easier
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

	// Returns a new SString, may allocate.
	inline SString ToSString() const { return SString(Str, Length); }

	STempString& operator=(const STempString& other);
	STempString& operator=(const char* cString);

	inline bool operator==(const SString& other) const { return SStrEquals(Str, other.Data()); }
	inline bool operator!=(const SString& other) const { return !SStrEquals(Str, other.Data()); }
	inline bool operator==(const STempString& other) const { return SStrEquals(Str, other.Str); }
	inline bool operator!=(const STempString& other) const { return !SStrEquals(Str, other.Str); }
	inline bool operator==(const char* other) const { return SStrEquals(Str, other); }
	inline bool operator!=(const char* other) const { return !SStrEquals(Str, other); }

	inline bool Empty() const { return Length == 0; }
	inline uint32_t End() const { return Length - 1; }
};

struct SStringView
{
	const char* Str;
	uint32_t Length;

	SStringView() = default;
	SStringView(const char* str);
	SStringView(const char* str, uint32_t length);
	SStringView(const char* str, uint32_t length, uint32_t offset);
	SStringView(const SString* string);
	SStringView(const SStringView* string, uint32_t offset);

	inline bool Empty() const { return Length == 0; }
	inline uint32_t End() const { return Length; } // SStringView are not null terminated
	inline const char* EndPtr() const { return Str + End(); }

	// Returns a new SString, may allocate.
	inline SString ToSString() const { return SString(Str, Length + 1); }
	inline STempString ToTempString() const { return STempString(Str, Length + 1); }

	inline bool operator==(const SStringView& other) const { return SStrEquals(Str, other.Str); }
	inline bool operator!=(const SStringView& other) const { return !SStrEquals(Str, other.Str); }

	SStringView SubString(uint32_t start, uint32_t end) const;

	uint32_t FindChar(char c) const;
	uint32_t FindChar(char c, uint32_t start) const;
	uint32_t Find(const char* cString) const;
};

inline int TestStringImpls()
{
	SString string0 = {};
	SASSERT(string0.Data());
	SASSERT(string0.Data()[0] == 0);
	SASSERT(string0.Capacity == 0);
	
	string0.Append("HELLO!");
	SASSERT(string0 == "HELLO!");
	SASSERT(string0.Capacity == SSTR_SSO_LENGTH);
	SASSERT(string0.Length == 7);
	string0.Append("1 2 3");
	SASSERT(string0 == "HELLO!1 2 3");
	SASSERT(string0.Capacity == SSTR_SSO_LENGTH);
	SASSERT(string0.Length == 12);

	SString string1 = "Literal";
	SASSERT(string1 == "Literal");
	SASSERT(string1 != "NotThis");
	SASSERT(string1 != string0);
	
	{
		SString string1Copy = string1;
		SASSERT(string1Copy == string1);
	}
	SASSERT(string1.Data());
	SASSERT(string1.Length == 8);
	SASSERT(string1.Capacity == SSTR_SSO_LENGTH);

	string1.Append("This");
	SASSERT(string1 == "LiteralThis");
	SASSERT(string1.Length == 12);
	SASSERT(string1.Capacity == SSTR_SSO_LENGTH);

	SString string2("Big long string test wow!!!!!");
	SASSERT(string2 == "Big long string test wow!!!!!");
	SASSERT(string2.Length == 30);
	SASSERT(string2.Capacity == 30);

	STempString tempString = "Testing Temporary!!";
	SASSERT(tempString == "Testing Temporary!!");

	SString sStringTemp = SString::CreateFake(&tempString);
	SASSERT(sStringTemp.DoNotFree);
	SASSERT(sStringTemp == tempString);

	SStringView view(&string2);
	SASSERT(SStrEquals(view.Str, string2.Data()));
	SASSERT(view.Str == string2.Data());

	SString testStr("String working");
	SASSERT(testStr == "String working");
	SASSERT(testStr.Length == 15);

	testStr = "Assigning!";
	SASSERT(testStr == "Assigning!");
	SASSERT(testStr.Length == 11);

	SStringView testStrView(&testStr);
	SASSERT(testStrView == "Assigning!");
	SASSERT(testStrView.Length == 10);

	SLOG_INFO("[ Test ] String test passed!");

	return 1;
}
