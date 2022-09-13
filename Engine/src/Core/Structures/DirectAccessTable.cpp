#include "DirectAccessTable.h"

#include "Core/SMemory.h"

void DATCreate(size_t length, size_t stride, DirectAccessTable* outTable)
{
	outTable->Length = length;
	outTable->Stride = stride;
	outTable->ContainsMemory = (bool*)Scal::MemAllocZero(length);
	outTable->ValuesMemory = Scal::MemAllocZero(length * stride);
}

void DATFree(DirectAccessTable* table)
{
	MemFree(table->ContainsMemory);
	MemFree(table->ValuesMemory);
}

void DATInsert(DirectAccessTable* table, size_t index, const void* src)
{
	table->ContainsMemory[index] = true;
	bool* dest = ((bool*)table->ValuesMemory) + (index * table->Stride);
	Scal::MemCopy(dest, src, table->Stride);
}

bool DATContains(DirectAccessTable* table, size_t index)
{
	return table->ContainsMemory[index];
}

void DATGet(DirectAccessTable* table, size_t index, void* outDest)
{
	if (!table->ContainsMemory[index])
		return;

	bool* src = ((bool*)table->ValuesMemory) + (index * table->Stride);
	Scal::MemCopy(outDest, src, table->Stride);
}

void DATRemove(DirectAccessTable* table, size_t index)
{
	table->ContainsMemory[index] = false;
}

void DATClear(DirectAccessTable* table)
{
	Scal::MemClear(table->ContainsMemory, table->Length);
}
