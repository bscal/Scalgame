#pragma once

#include "Core/Core.h"

namespace Scal
{

struct DirectAccessTable
{
	bool* Memory;
	size_t Length;
};

namespace DirectAccess
{

void DATCreate(size_t length, DirectAccessTable* outTable);
void DATFree(DirectAccessTable* table);
void DATInsert(DirectAccessTable* table, size_t index);
bool DATContains(DirectAccessTable* table, size_t index);
void DATRemove(DirectAccessTable* table, size_t index);
void DATClear(DirectAccessTable* table);

}




}
