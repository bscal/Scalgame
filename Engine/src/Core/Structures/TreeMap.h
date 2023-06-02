#pragma once

#include "Core/Core.h"
#include "Core/SMemory.h"

template<typename K, typename V>
struct TreeMapEntry
{
	K Key;
	V Value;
	TreeMapEntry<K, V>* Left;
	TreeMapEntry<K, V>* Right;
};



template<typename K, typename V>
struct TreeMap
{
	typedef void(*TreeMapForEachCB)(TreeMapEntry<K, V>);

	TreeMapEntry<K, V>* Head;
	uint32_t Count;
	SAllocator Allocator;

	V* Insert(const K* key, const V* value)
	{
		TreeMapEntry<K, V>* entry = TryInsert(&Head, key, value);
		if (entry)
		{
			++Count;
			return &entry->Value;
		}
		else
			return nullptr;
	}

	V* Get(const K* key)
	{
		TreeMapEntry<K, V>* entry = FindKey(Head, key);
		if (entry)
			return &entry->Value;
		else
			return nullptr;
	}

	bool Contains(const K* key)
	{
		return (FindKey(Head, key) != nullptr);
	}

	bool Remove(const K* key)
	{
		TreeMapEntry<K, V>* entry = FindKey(Head, key);
		if (entry)
		{
			if (!entry->Left && !entry->Right)
			{
				--Count;
				FreeEntry(entry);
				return true;
			}

			TreeMapEntry<K, V>* tmp = nullptr;
			if (!entry->Left)
			{
				tmp = entry->Right;
			}
			else if (!entry->Right)
			{
				tmp = entry->Left;
			}
			else
			{
				tmp = entry->Right;
				SwapMin(tmp, entry->Left);
			}
			
			--Count;
			*entry = *tmp;
			FreeEntry(tmp);
			return true;
		}
		else
			return false;
	}

	void ForEach(TreeMapForEachCB callback)
	{
		SASSERT(callback);

		TreeMapEntry<K, V>* next = Head;
		while (next)
		{
			callback(next);
		}
	}

private:
	TreeMapEntry<K, V>* CreateEntry(const K* key, const V* value)
	{
		constexpr size_t size = sizeof(TreeMapEntry<K, V>);
		TreeMapEntry<K, V>* res = (TreeMapEntry<K, V>*)SAlloc(Allocator, size, MemoryTag::Trees);
		res->Key = *key;
		res->Value = *value;
		res->Left = nullptr;
		res->Right = nullptr;
		return res;
	}

	void FreeEntry(TreeMapEntry<K, V>* entry)
	{
		constexpr size_t size = sizeof(TreeMapEntry<K, V>);
		SFree(Allocator, entry, size, MemoryTag::Trees);
	}

	TreeMapEntry<K, V>* TryInsert(TreeMapEntry<K, V>** rootPtr, const K* key, const V* value)
	{
		SASSERT(rootPtr);
		SASSERT(key);
		SASSERT(value);

		TreeMapEntry<K, V>* root = *rootPtr;
		if (!root)
		{
			*rootPtr = CreateEntry(key, value);
			return *rootPtr;
		}
		
		if (root->Key == *key)
			return root;

		if (*key < root->Key)
			return TryInsert(&root->Left, key, value);
		else
			return TryInsert(&root->Right, key, value);
	}

	TreeMapEntry<K, V>* FindKey(TreeMapEntry<K, V>* rootPtr, const K* key)
	{
		if (!rootPtr)
			return nullptr;

		if (rootPtr->Key == *key)
			return rootPtr;

		if (*key < rootPtr->Key)
			return FindKey(rootPtr->Left, key);
		else
			return FindKey(rootPtr->Right, key);
	}

	void SwapMin(TreeMapEntry<K, V>* rootPtr, TreeMapEntry<K, V>* entry)
	{
		if (!rootPtr->Left)
			rootPtr->Left = entry;
		else
		{
			if (entry->Key < rootPtr->Left->Key)
			{
				TreeMapEntry<K, V>* tmp = rootPtr;
				rootPtr->Key = entry->Key;
				rootPtr->Value = entry->Value;
				entry->Key = tmp->Key;
				entry->Value = tmp->Value;
			}
			SwapMin(rootPtr->Left, entry);
		}

	}
};

inline int TreeTest()
{
	TreeMap<int, int> tree = {};

	int a = 4;
	int b = 8;
	tree.Insert(&a, &b);

	int c = 64;
	int d = -64;
	tree.Insert(&c, &d);

	int x = 1;
	int y = 1;
	tree.Insert(&x, &y);

	tree.Insert(&x, &d);

	int w = 2;
	int z = 0;
	tree.Insert(&w, &z);

	SASSERT(tree.Count == 5);

	int* v1 = tree.Get(&w);
	SASSERT(*v1 == z);
	int* v2 = tree.Get(&c);
	SASSERT(*v2 == d);

	bool contains = tree.Contains(&a);
	SASSERT(contains);

	bool r = tree.Remove(&a);
	SASSERT(r);
	bool r2 = tree.Remove(&a);
	SASSERT(!r2);

	bool contains2 = tree.Contains(&a);
	SASSERT(!contains2);

	bool r3 = tree.Remove(&w);
	SASSERT(r3);

	bool contains3 = tree.Contains(&x);
	SASSERT(contains3);

	return 0;
}
