#pragma once

#include "Core/Core.h"

#include "SList.h"
#include "SLinkedList.h"

struct EntityContainer
{
	template<typename EntityType>
	using EntityArray = SList<EntityType>;

	uint32_t* Entities;
	uint32_t* Indices;
	SList<void*> EntityArrays;
	SLinkedList<uint32_t> UnusedEntities;
};
