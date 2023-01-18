#pragma once

#include "Core.h"

global_var uint32_t SNextComponentId;

template<typename T>
struct SComponent
{
	static const uint32_t ID;
	uint32_t EntityId;
};

template<typename T>
const uint32_t SComponent<T>::ID = SNextComponentId++;

// ****************
// Components
// ****************

struct Human : SComponent<Human>
{
	uint32_t Age;
};

struct Undead : SComponent<Undead>
{
};
