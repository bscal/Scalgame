#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

#include <assert.h>

template<typename T>
struct SLinkedListEntry
{
	SLinkedListEntry<T>* Next;
	T* Value;
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
	SLinkedListEntry<T>* entry = (SLinkedListEntry<T>*)
		Scal::MemAllocZero(sizeof(T*));
	entry->Value = value;
	return entry;
}

template<typename T>
void SLinkedListFree(SLinkedList<T>* list)
{
	SLinkedListEntry<T>* nextEntry = list->First->Next;
	bool hasNext = list->First->Next != nullptr;
	while (hasNext)
	{
		Scal::MemFree(&nextEntry->Value);
		nextEntry = nextEntry->Next;
		hasNext = nextEntry->Next != nullptr;
	}
}

template<typename T>
void SLinkedListPush(SLinkedList<T>* list, const T* value)
{
	assert(list);
	if (!value)
	{
		S_LOG_ERR("value cannot be nullptr");
		return;
	}

	auto newEntry = CreateEntry(value);
	if (list->First)
	{
		list->Last->Next = newEntry;
		list->Last = newEntry;
	}
	else
	{
		list->First = newEntry;
	}

	++list->Size;
}

template<typename T>
T* SLinkedListPop(SLinkedList<T>* list)
{
	assert(list);
	if (!value)
	{
		S_LOG_ERR("value cannot be nullptr");
		return;
	}
	if (list->Size == 0)
	{
		return nullptr;
	}

	--list->Size;

	SLinkedListEntry<T>* firstEntry = list->First;

	if (firstEntry->Next)
		list->First = firstEntry->Next;
	else
		list->First = nullptr;

	return firstEntry->Value;
}