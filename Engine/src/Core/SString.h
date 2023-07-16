#pragma once

#include "Core.h"
#include "SMemory.h"
#include "SHash.hpp"

#include <stdint.h>
#include <string.h>

constexpr global_var uint32_t SSTR_SSO_ARRAY_SIZE = 16;
constexpr global_var uint32_t SSTR_NO_POS = UINT32_MAX;

bool SStrEquals(const char* str0, const char* str1);

struct SStringView;

struct SString
{
	union StringMemory
	{
		char* StrMemory;
		char ShortStringBuf[SSTR_SSO_ARRAY_SIZE];
	};

	uint32_t Length : 24;
	uint32_t Capacity : 24;
	uint32_t Allocator : 8;

	SString() = default;
	SString(SAllocator allocator);
	SString(const char* str);
	SString(const char* str, uint32_t length, SAllocator allocator);
	SString(const SString& other);
	SString(SString&& other) noexcept;

	// If not SSOed then frees
	~SString();

	void SetCapacity(uint32_t capacity);

	void Assign(const char* cStr, uint32_t length, SAllocator allocator);
	void Assign(const char* cStr, SAllocator allocator);
	void Assign(const SString& other, SAllocator allocator);

	void Append(const char* str);
	void Append(const char* str, uint32_t length);

	uint32_t FindChar(char c) const;
	uint32_t Find(const char* cString) const;

	uint32_t Substring(char delimiter, uint32_t offset) const;
	SString FirstNew(char delimiter, uint32_t offset) const;
	SString FirstTemp(char delimiter, uint32_t offset) const;
	SString Slice(uint32_t start, uint32_t end, SAllocator allocator) const;

	SString& operator=(const SString& other);
	SString& operator=(const char* cString);

	inline bool operator==(const SString& other) const { return SStrEquals(Data(), other.Data()); }
	inline bool operator!=(const SString& other) const { return !SStrEquals(Data(), other.Data()); }
	inline bool operator==(const char* other) const { return SStrEquals(Data(), other); }
	inline bool operator!=(const char* other) const { return !SStrEquals(Data(), other); }

	friend SString operator+(const SString& lhs, const SString& rhs)
	{
		size_t length = lhs.Length + rhs.Length;

		SString res = SString((SAllocator)lhs.Allocator);
		res.SetCapacity(length + 1); // +1 for null terminator

		res.Append(lhs.Data(), lhs.Length);
		res.Append(rhs.Data(), rhs.Length);

		return res;
	}

	friend SString operator+(const SString& lhs, const char* rhs)
	{
		size_t cStrLength = strlen(rhs);
		size_t length = lhs.Length + cStrLength;

		SString res = SString((SAllocator)lhs.Allocator);
		res.SetCapacity(length + 1); // +1 for null terminator

		res.Append(lhs.Data(), lhs.Length);
		res.Append(rhs, cStrLength);

		return res;
	}

	_FORCE_INLINE_ bool IsAllocated() const { return Capacity > SSTR_SSO_ARRAY_SIZE;  }
	_FORCE_INLINE_ bool Empty() const { return Length == 0 || Data()[0] == '\0'; }
	_FORCE_INLINE_ uint32_t Last() const { return Length - 1; }
	_FORCE_INLINE_ char* begin() { return Data(); }
	_FORCE_INLINE_ char* end() { return Data() + Length; }

	_FORCE_INLINE_ const char* Data() const
	{
		return (Capacity > SSTR_SSO_ARRAY_SIZE) ? m_Buffer.StrMemory : m_Buffer.ShortStringBuf;
	}

	_FORCE_INLINE_ char* Data()
	{
		return (Capacity > SSTR_SSO_ARRAY_SIZE) ? m_Buffer.StrMemory : m_Buffer.ShortStringBuf;
	}

private:
	// NOTE: Not sure if I want to
	// have this private? Not a fan but
	// calling Data() always is easier
	StringMemory m_Buffer; 
};

//struct SStringView
//{
//	const char* Str;
//	uint32_t Length;
//
//	SStringView() = default;
//	SStringView(const char* str);
//	SStringView(const char* str, uint32_t length);
//	SStringView(const char* str, uint32_t length, uint32_t offset);
//	SStringView(const SString* string);
//	SStringView(const SStringView* string, uint32_t offset);
//
//	SStringView& operator=(const SString& other) = delete;
//	SStringView& operator=(const char* cString) = delete;
//
//	inline bool Empty() const { return (Length == 0 || Str[0] == '\0'); }
//	inline uint32_t End() const { return (Length == 0) ? 0 : Length - 1; }
//
//	inline bool operator==(const SStringView& other) const { return SStrEquals(Str, other.Str); }
//	inline bool operator!=(const SStringView& other) const { return !SStrEquals(Str, other.Str); }
//
//	SStringView SubString(uint32_t start, uint32_t end) const;
//
//	uint32_t FindChar(char c) const;
//	uint32_t FindChar(char c, uint32_t start) const;
//	uint32_t Find(const char* cString) const;
//};

struct SRawString
{
	char* Data;
	uint32_t Length;
	SAllocator Allocator;

	inline bool operator==(const SRawString& other) const { return Length == other.Length && SStrEquals(Data, other.Data); }
	inline bool operator!=(const SRawString& other) const { return Length != other.Length && !SStrEquals(Data, other.Data); }
};

SRawString RawStringNew(const char* cStr, SAllocator allocator);
SRawString RawStringNew(const char* cStr, uint32_t length, SAllocator allocator);
void RawStringFree(SRawString* string);

uint32_t StrFind(const char* str, const char* find);

struct SStringsBuffer
{
	char** StringsMemory;
	uint32_t StringCount;
	uint32_t StringCapacity;

	void Initialize(uint32_t stringCount, uint32_t stringCapacity);
	void Free();

	char* Push();
};

struct SRawStringHasher
{
	[[nodiscard]] constexpr uint32_t operator()(const SRawString* key, size_t size) const noexcept
	{
		//size is sizeof(SRawString), we ignore because we want to compare string contents
		return FNVHash32((const uint8_t*)key->Data, key->Length);
	}
};

struct SStringHasher
{
	[[nodiscard]] constexpr uint32_t operator()(const SString* key, size_t size) const noexcept
	{
		//size is sizeof(SRawString), we ignore because we want to compare string contents
		return FNVHash32((const uint8_t*)key->Data(), key->Length);
	}
};

inline int TestStringImpls()
{
	SString string0 = {};
	SASSERT(string0.Data());
	SASSERT(string0.Data()[0] == 0);
	SASSERT(string0.Capacity == 0);
	
	string0.Append("HELLO!");
	SASSERT(string0 == "HELLO!");
	SASSERT(string0.Capacity == SSTR_SSO_ARRAY_SIZE);
	SASSERT(string0.Length == 6);
	string0.Append("1 2 3");
	SASSERT(string0 == "HELLO!1 2 3");
	SASSERT(string0.Capacity == SSTR_SSO_ARRAY_SIZE);
	SASSERT(string0.Length == 11);

	SString string1 = "Literal";
	SASSERT(string1 == "Literal");
	SASSERT(string1 != "NotThis");
	SASSERT(string1 != string0);
	
	{
		SString string1Copy = string1;
		SASSERT(string1Copy == string1);
	}
	SASSERT(string1.Data());
	SASSERT(string1.Length == 7);
	SASSERT(string1.Capacity == SSTR_SSO_ARRAY_SIZE);

	string1.Append("This");
	SASSERT(string1 == "LiteralThis");
	SASSERT(string1.Length == 11);
	SASSERT(string1.Capacity == SSTR_SSO_ARRAY_SIZE);

	SString string2("Big long string test wow!!!!!");
	SASSERT(string2 == "Big long string test wow!!!!!");
	SASSERT(string2.Length == 29);
	SASSERT(string2.Capacity == 30);


	SString testStr("String working!!");
	SASSERT(testStr == "String working!!");
	SASSERT(testStr.Length == 16);
	SASSERT(testStr.IsAllocated());

	testStr = "Assigning!12345";
	SASSERT(testStr == "Assigning!12345");
	SASSERT(testStr.Length == 15);
	SASSERT(!testStr.IsAllocated());


	SLOG_INFO("[ Test ] String test passed!");

	return 1;
}
