#pragma once

#include "Structures/StaticArray.h"

#include <stdint.h>

enum class AttributeType : uint8_t
{
	STRENGTH = 0,

	MAX_VALUE,
};

enum class AttributeModifierType : uint8_t
{
	ADD,
	MULTIPLY
};

struct AttributeModifier
{
	float Value;
	AttributeModifierType Type;
};

struct AttributesContainer
{
	StaticArray<float, (uint8_t)AttributeType::MAX_VALUE> BaseAttributes;
	StaticArray<float, (uint8_t)AttributeType::MAX_VALUE> PlusAttributes;

	void ResetAttributes();

	float GetAttribute(AttributeType attribute) const;

	void AddAttribute(AttributeType attribute, AttributeModifier modifier);

	void RemoveAttribute(AttributeType attribute, AttributeModifier modifier);
};
