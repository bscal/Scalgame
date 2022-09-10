#include "DirectAccessTable.h"

#include "Core/SMemory.h"

namespace Scal
{
namespace DirectAccess
{

void DASCreate(size_t length, DirectAccessSet* outTable)
{
	outTable->Length = length;
	outTable->Memory = (bool*)Memory::AllocZero(length);
}

void DASFree(DirectAccessSet* table)
{
	Memory::Free(table->Memory);
}

void DASInsert(DirectAccessSet* table, size_t index)
{
	table->Memory[index] = true;
}

bool DASContains(DirectAccessSet* table, size_t index)
{
	return table->Memory[index];
}

void DASRemove(DirectAccessSet* table, size_t index)
{
	table->Memory[index] = false;
}

void DASClear(DirectAccessSet* table)
{
	Memory::Clear(table->Memory, table->Length);
}

}
}

