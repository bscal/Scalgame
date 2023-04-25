#include "Attributes.h"

#include "Core.h"

void AttributesContainer::ResetAttributes()
{
	PlusAttributes.Fill(0.0f);
}

float AttributesContainer::GetAttribute(AttributeType attribute) const
{
	return BaseAttributes[(uint8_t)attribute] + PlusAttributes[(uint8_t)attribute];
}

void AttributesContainer::AddAttribute(AttributeType attribute, AttributeModifier modifier)
{
	switch (modifier.Type)
	{
		case (AttributeModifierType::ADD):
		{
			PlusAttributes[(uint8_t)attribute] += modifier.Value;
		} break;
		case (AttributeModifierType::MULTIPLY):
		{
			PlusAttributes[(uint8_t)attribute] += BaseAttributes[(uint8_t)attribute] * modifier.Value;
		} break;

		default:
			SASSERT(false); break;
	}
}

void AttributesContainer::RemoveAttribute(AttributeType attribute, AttributeModifier modifier)
{
	switch (modifier.Type)
	{
		case (AttributeModifierType::ADD):
		{
			PlusAttributes[(uint8_t)attribute] -= modifier.Value;
		} break;
		case (AttributeModifierType::MULTIPLY):
		{
			PlusAttributes[(uint8_t)attribute] -= BaseAttributes[(uint8_t)attribute] * modifier.Value;
		} break;

		default:
			SASSERT(false); break;
	}
}