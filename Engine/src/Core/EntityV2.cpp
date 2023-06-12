#include "EntityV2.h"

#include "Game.h"
#include "Renderer.h"

global_var EntityManager EntityMgr;

void EntityMgrInitialize(GameApplication* gameApp)
{
	EntityMgr.Entities.Reserve(256);
}

EntityManager* GetEntityMgr()
{
	return &EntityMgr;
}

internal void
PlayerUpdate(Player* player, Game* game)
{

}

internal void 
MonsterUpdate(Monster* monster, Game* game)
{

}

void UpdateEntities(Game* game)
{
	PlayerUpdate(&EntityMgr.Player, game);

	for (uint32_t i = 0; i < EntityMgr.Entities.Capacity; ++i)
	{
		if (EntityMgr.Entities.Buckets[i].Occupied)
		{
			Monster* monster = (Monster*)&EntityMgr.Entities.Buckets[i].Value;
			MonsterUpdate(monster, game);
		}
	}
}

void DrawEntities(Game* game)
{
	const Texture2D* spriteSheet = &game->Resources.EntitySpriteSheet;

	for (uint32_t i = 0; i < EntityMgr.Entities.Capacity; ++i)
	{
		if (EntityMgr.Entities.Buckets[i].Occupied)
		{
			Monster* monster = (Monster*)&EntityMgr.Entities.Buckets[i].Value;
			Sprite sprite = EntityMgr.CreatureDB[(uint16_t)monster->Creature.CreatureType].Sprite;
			SDrawSprite(spriteSheet, sprite, monster->TilePos, monster->Color);
		}
	}

	// Draws player
	// TODO should players have custom sprites?
	Sprite sprite = EntityMgr.CreatureDB[(uint16_t)EntityMgr.Player.Creature.CreatureType].Sprite;
	SDrawSprite(spriteSheet, sprite, EntityMgr.Player.TilePos, EntityMgr.Player.Color);
}

internal void 
NewEntity(EntityId* uid, EntityTypes type, void* entityData)
{
	uid->Id = EntityMgr.NextUid++;
	uid->Type = static_cast<uint32_t>(type);
	EntityMgr.Entities.Insert(&uid->Mask, &entityData);
}

internal void
InitializeCreature(Creature* creature, CreatureTypes type)
{
	SASSERT(creature);
}


// TODO SpawnLocation, Type
Monster* SpawnMonster()
{
	Monster* res = EntityMgr.Monsters.allocate();
	NewEntity(&res->Uid, EntityTypes::Monster, res);
	InitializeCreature(&res->Creature, CreatureTypes::Human);
	SASSERT(res);
	return res;
}

void DeleteMonster(Monster* monster)
{
	SASSERT(monster);
	bool wasRemoved = EntityMgr.Entities.Remove(&monster->Uid.Mask);
	if (wasRemoved)
	{
		EntityMgr.Monsters.destroy(monster);
	}
}

void* GetEntity(EntityId ent)
{
	if (ent.Mask == 0)
		return &EntityMgr.Player;

	return EntityMgr.Entities.Get(&ent.Mask);
}

bool DoesEntityExist(EntityId ent)
{
	return EntityMgr.Entities.Contains(&ent.Mask);
}

TileEntity* SpawnTileEntity(Vector2i pos)
{
	return EntityMgr.TileEntities.InsertKey(&pos);
}