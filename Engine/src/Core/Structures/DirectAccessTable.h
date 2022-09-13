#pragma once

#include "Core/Core.h"

struct DirectAccessTable
{
	bool* ContainsMemory;
	void* ValuesMemory;
	size_t Length;
	size_t Stride;
};

void DATCreate(size_t length, size_t stride, DirectAccessTable* outTable);
void DATFree(DirectAccessTable* table);
void DATInsert(DirectAccessTable* table, size_t index, const void* src);
bool DATContains(DirectAccessTable* table, size_t index);
void DATGet(DirectAccessTable* table, size_t index, void* dest);
void DATRemove(DirectAccessTable* table, size_t index);
void DATClear(DirectAccessTable* table);

