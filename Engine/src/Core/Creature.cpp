#include "Creature.h"

#include "ResourceManager.h"

bool CreatureInitialize(Creature* creature, const CreatureType* type)
{
	creature->SpriteTextureRect = type->SpriteTextureRect;
	creature->EntityId = type->EntityId;
	creature->Level = type->Level;
	creature->MaxHealth = type->MaxHealth;
	creature->MaxMana = type->MaxMana;

	creature->Position = { 16, 16 };
	creature->Health = creature->MaxHealth;
	creature->Mana = creature->MaxMana;
	creature->Difficulty = 1;

	return true;
}

void CreatureUpdate(Creature* creature, Game* game)
{

}

void CreatureRender(Resources* resources, Creature* creature)
{
	DrawTextureRec(resources->EntitySpriteSheet,
		creature->SpriteTextureRect,
		creature->Position,
		WHITE);
}