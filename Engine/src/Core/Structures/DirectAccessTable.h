#pragma once

#include "Core/Core.h"

namespace Scal
{

struct DirectAccessSet
{
	bool* Memory;
	size_t Length;
};

namespace DirectAccess
{

void DASCreate(size_t length, DirectAccessSet* outTable);
void DASFree(DirectAccessSet* table);
void DASInsert(DirectAccessSet* table, size_t index);
bool DASContains(DirectAccessSet* table, size_t index);
void DASRemove(DirectAccessSet* table, size_t index);
void DASClear(DirectAccessSet* table);

}




}
