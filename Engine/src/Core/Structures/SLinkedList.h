#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

template<typename T>
struct SLinkedListEntry
{
	SLinkedListEntry<T>* Next;
	T Value;
};

template<typename T>
struct SLinkedList
{
	SLinkedListEntry<T>* First;
	uint64_t Size;

	void Free();
	void Push(const T* value);
	void Pop();
	[[nodiscard]] T PopValue();
};

template<typename T>
internal SLinkedListEntry<T>* FreeEntry(SLinkedListEntry<T>* entry)
{
	auto next = entry->Next;
	Scal::MemFree(entry);
	return next;
}

template<typename T>
void SLinkedList<T>::Free()
{
	SLinkedListEntry<T>* first = First;
	if (first)
	{
		SLinkedListEntry<T>* next = first->Next;
		while (next)
		{
			next = FreeEntry(next);
		}
		FreeEntry(first);
	}
	First = NULL;
	Size = 0;
}

template<typename T>
void SLinkedList<T>::Push(const T* value)
{
	assert(value);

	SLinkedListEntry<T>* entry = (SLinkedListEntry<T>*)Scal::MemAllocZero(sizeof(T));
	assert(entry);

	entry->Value = *value;
	entry->Next = First;
	First = entry;
	++Size;
}

template<typename T>
void SLinkedList<T>::Pop()
{
	if (Size == 0) return;
	First = FreeEntry(First);
	--Size;
}

template<typename T>
[[nodiscard]] T SLinkedList<T>::PopValue()
{
	assert(Size > 0);
	T val = First->Value;
	First = FreeEntry(First);
	--Size;
	return val;
}