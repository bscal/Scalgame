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
	SLinkedListEntry<T>* Last;
	uint64_t Size;
};

template<typename T>
internal SLinkedListEntry<T>* CreateEntry(T* value)
{
	assert(value);
	SLinkedListEntry<T>* entry = (SLinkedListEntry<T>*)
		Scal::MemAllocZero(sizeof(T*));
	entry->Value = value;
	return entry;
}

template<typename T>
internal SLinkedListEntry<T>* FreeEntry(SLinkedListEntry<T>* entry)
{
	if (entry)
	{
		auto next = entry->Next;
		Scal::MemFree(entry);
		return next;
	}
	return nullptr;
}

template<typename T>
void SLinkedFree(SLinkedList<T>* list)
{
	assert(list);
	SLinkedListEntry<T>* next = list->First;
	while (next)
	{
		next = FreeEntry(next);
	}
	list->First = nullptr;
	list->Last = nullptr;
	list->Size = 0;
}

template<typename T>
void SLinkedListPush(SLinkedList<T>* list, T* value)
{
	assert(list);
	if (!value)
	{
		S_LOG_ERR("value cannot be nullptr");
		return;
	}

	SLinkedListEntry<T>* newEntry = CreateEntry<T>(value);

	if (!list->First)
		list->First = newEntry;

	if (list->Last)
	{
		list->Last->Next = newEntry;
	}
	list->Last = newEntry;
	++list->Size;
}

template<typename T>
void SLinkedListPop(SLinkedList<T>* list)
{
	assert(list);
	if (!value)
	{
		S_LOG_ERR("value cannot be nullptr");
		return;
	}

	if (list->Size == 0)
		return;

	auto freedEntryNext = FreeEntry(list->First);

	if (freedEntryNext)
	{
		list->First = freedEntryNext;
	}
	else
	{
		list->First = nullptr;
		list->Last = nullptr;
	}

	--list->Size;
}