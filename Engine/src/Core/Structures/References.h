#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <atomic>

template<typename T>
struct RefBlock
{
	T Value;
	std::atomic<int> Counter;
	SAllocator Allocator;
};

template<typename T>
struct Ref
{
	RefBlock<T>* Data;

	//Ref() = default;

	//Ref(const Ref<T>& other)
	//{
	//	Data = other.Data;
	//	Allocator = other.Allocator;

	//	SLOG_DEBUG("Copied!");
	//	Data->Counter += 1;
	//}

	//~Ref()
	//{
	//	SASSERT(Data);
	//	Release();
	//}

	//Ref<T>& operator=(const Ref<T>& other)
	//{
	//	return other.Copy();
	//}

	Ref<T>& Inc()
	{
		SASSERT(Data);
		SLOG_DEBUG("Scoped!");

		Data->Counter++;

		return *this;
	}

	void Release()
	{
		SASSERT(Data);
		SLOG_DEBUG("Unscoped!");

		int counter = Data->Counter.fetch_sub(1);
		if (counter <= 0)
		{
			size_t size = sizeof(RefBlock<T>);
			SFree(Data->Allocator, Data, size, MemoryTag::Game);
		}

		Data = nullptr;
	}
};

template<typename T>
inline Ref<T> RefAlloc(SAllocator allocator, const T* ptr)
{
	SASSERT(ptr);
	SLOG_DEBUG("MakeRef called");

	Ref<T> ref;

	size_t size = sizeof(RefBlock<T>);
	ref.Data = (RefBlock<T>*)SAlloc(allocator, size, MemoryTag::Game);

	ref.Data->Allocator = allocator;

	memcpy(&ref.Data->Value, ptr, sizeof(T));
	
	CALL_CONSTRUCTOR(&ref.Data->Counter) std::atomic<int>(1);

	return ref;
}

inline int TestRef()
{
	size_t val = 100;
	Ref<size_t> ref = RefAlloc(SAllocator::Game, &val);
	SASSERT(ref.Data);
	SASSERT(ref.Data->Value == 100);
	SASSERT(ref.Data->Counter == 1);

	Ref<size_t> ref2 = ref.Inc();
	SASSERT(ref2.Data);
	SASSERT(ref2.Data->Value == 100);
	SASSERT(ref2.Data->Counter == 2);

	{
		Ref<size_t> ref3 = ref.Inc();

		ref3.Data->Value += 100;

		SASSERT(ref3.Data);
		SASSERT(ref3.Data->Value == 200);
		SASSERT(ref3.Data->Counter == 3);

		ref3.Release();
	}

	SASSERT(ref.Data);
	SASSERT(ref.Data->Value == 200);
	SASSERT(ref.Data->Counter == 2);

	ref.Release();

	SASSERT(!ref.Data);

	SASSERT(ref2.Data);
	SASSERT(ref2.Data->Value == 200);
	SASSERT(ref2.Data->Counter == 1);

	return 1;
}
