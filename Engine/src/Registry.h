#pragma once

#include "Core/Core.h"
#include "Core/SString.h"
#include "Core/SHash.hpp"

#include "Core/Structures/SList.h"
#include "Core/Structures/SHoodTable.h"

struct SStringViewHasher
{
	[[nodiscard]] constexpr uint64_t operator()(const SStringView* key) const noexcept
	{
		const uint8_t* data = (const uint8_t*)(key->Str);
		return FNVHash64(data, key->Length);
	}
};

template<typename T>
struct Registry
{
	SHoodTable<SStringView, int, SStringViewHasher> NameToIndex;
	SList<T> Entries;

	uint32_t NextId;

	int Add(SStringView name, const T* val)
	{
		SASSERT(name.Str);
		SASSERT(name.Length > 0);

		Entries.Push(val);

		int index = Entries.LastIndex(); 

		NameToIndex.Insert(&name, &index);
		return index;
	}

	T* Find(SStringView name) const
	{
		int* index = NameToIndex.Get(&name);
		if (index)
			return Entries.PeekAt(index);
		else
			return nullptr;
	}

};
