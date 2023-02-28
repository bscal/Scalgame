#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

template<typename T>
struct SLinkedListEntry
{
	SLinkedListEntry<T>* Next;
	T Value;
};

template<typename T>
struct SLinkedList
{
	SMemAllocator Allocator = SMEM_GAME_ALLOCATOR;
	SLinkedListEntry<T>* First;
	uint32_t Size;

	void Free();

	void Push(const T* value);
	void Pop();
	T* Peek() const;
	[[nodiscard]] T PopValue();

	inline bool HasNext() const { return (Size > 0); }

private:
	SLinkedListEntry<T>* CreateEntry(const T* value) const;
	SLinkedListEntry<T>* FreeEntry(SLinkedListEntry<T>* entry) const;
};

template<typename T>
SLinkedListEntry<T>*
SLinkedList<T>::CreateEntry(const T* value) const
{
	SASSERT(value);
	SASSERT(Allocator.Alloc);

	SLinkedListEntry<T>* entry = (SLinkedListEntry<T>*)Allocator.Alloc(sizeof(SLinkedListEntry<T>));
	SASSERT(entry);
	entry->Next = First;
	entry->Value = *value;
	return entry;
}

template<typename T>
SLinkedListEntry<T>* 
SLinkedList<T>::FreeEntry(SLinkedListEntry<T>* entry) const
{
	SASSERT(entry);
	SASSERT(Allocator.Free);

	SLinkedListEntry<T>* next = entry->Next;
	Allocator.Free(entry);
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
	First = CreateEntry(value);
	++Size;
}

template<typename T>
void SLinkedList<T>::Pop()
{
	if (HasNext())
	{
		First = FreeEntry(First);
		--Size;
	}
}

template<typename T>
T* SLinkedList<T>::Peek() const
{
	return (HasNext()) ? &First->Value : nullptr;
}

template<typename T>
[[nodiscard]] T SLinkedList<T>::PopValue()
{
	SASSERT(HasNext());
	SASSERT(First);
	T val = First->Value;
	Pop();
	return val;
}