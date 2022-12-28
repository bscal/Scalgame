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
internal SLinkedListEntry<T>* CreateEntry(T value)
{
	SLinkedListEntry<T>* entry = (SLinkedListEntry<T>*)
		Scal::MemAllocZero(sizeof(T*));
	assert(entry);
	entry->Value = value;
	return entry;
}

template<typename T>
internal SLinkedListEntry<T>* FreeEntry(SLinkedListEntry<T>* entry)
{
	auto next = entry->Next;
	Scal::MemFree(entry);
	return next;
}

template<typename T>
void SLinkedFree(SLinkedList<T>* list)
{
	SLinkedListEntry<T>* first = list->First;
	if (first != nullptr)
	{
		SLinkedListEntry<T>* next = first->Next;
		while (next)
		{
			next = FreeEntry(next);
		}
		FreeEntry(first);
	}
	list->First = NULL;
	list->Last = NULL;
	list->Size = 0;
}

template<typename T>
void SLinkedListPush(SLinkedList<T>* list, T value)
{
	SLinkedListEntry<T>* newEntry = CreateEntry<T>(value);
	assert(newEntry);

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