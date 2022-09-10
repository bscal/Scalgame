#include "DirectAccessTable.h"

#include "Core/SMemory.h"

namespace Scal
{
namespace DirectAccess
{

void DATCreate(size_t length, DirectAccessTable* outTable)
{
	outTable->Length = length;
	outTable->Memory = (bool*)Memory::AllocZero(length);
}

void DATFree(DirectAccessTable* table)
{
	Memory::Free(table->Memory);
}

void DATInsert(DirectAccessTable* table, size_t index)
{
	table->Memory[index] = true;
}

bool DATContains(DirectAccessTable* table, size_t index)
{
	return table->Memory[index];
}

void DATRemove(DirectAccessTable* table, size_t index)
{
	table->Memory[index] = false;
}

void DATClear(DirectAccessTable* table)
{
	Memory::Clear(table->Memory, table->Length);
}

}
}

