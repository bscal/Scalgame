#include "Entity.h"

#include "Game.h"
#include "Player.h"
#include "Renderer.h"
#include "Inventory.h"

#include <raylib/src/raymath.h>

global_var EntityManager EntityMgr;

void EntityMgrInitialize(GameApplication* gameApp)
{
	EntityMgr.Entities.Reserve(256);
	EntityMgr.Monsters.Reserve(256);
}

EntityManager* GetEntityMgr()
{
	return &EntityMgr;
}

internal void 
MonsterUpdate(Monster* monster, Game* game)
{

}

void UpdateEntities(Game* game)
{
	UpdatePlayer(&EntityMgr.Player, game);

	for (uint32_t i = 0; i < EntityMgr.Monsters.Count; ++i)
	{
		MonsterUpdate(EntityMgr.Monsters[i], game);
	}
}

void DrawEntities(Game* game)
{
	const Texture2D* spriteSheet = &game->Resources.EntitySpriteSheet;

	for (uint32_t i = 0; i < EntityMgr.Monsters.Count; ++i)
	{
		Monster* monster = EntityMgr.Monsters[i];
		Sprite sprite = GetCreatureType(&monster->Creature)->Sprite;
		SDrawSprite(spriteSheet, sprite, monster->TilePos, monster->Color);
	}

	DrawPlayer(&EntityMgr.Player, game);
}

void CreatePlayer(Player* player)
{

	
	

}

internal void 
NewEntity(uint32_t* uid, EntityTypes type, void* entityData)
{
	*uid = EntityMgr.NextUid++;
	EntityMgr.Entities.Insert(uid, &entityData);
}

internal void
InitializeCreature(Creature* creature, uint32_t uid, CreatureType type)
{
	SASSERT(creature);

	creature->CreatureType = type;

	CreatureData* creatureData = GetCreatureType(creature);

	creature->Health = creatureData->MaxHealth;
	creature->Energy = creatureData->MaxEnergy;
	creature->Morale = 100;
	creature->Stamina = 100;
	creature->Sanity = 100;
	creature->DisplayName = creatureData->DefaultDisplayName;
	creature->InventoryId = GetGame()->InventoryMgr.CreateInventory(uid, { 8, 8 })->InventoryId;
	creature->EquipmentId = GetGame()->InventoryMgr.CreateEquipment()->EquipmentId;
}

// TODO SpawnLocation, Type
Monster* SpawnMonster()
{
	Monster* res = EntityMgr.MonsterPool.allocate();
	SMemClear(res, sizeof(Monster));
	
	NewEntity(&res->Uid, EntityTypes::Monster, res);
	InitializeCreature(&res->Creature, res->Uid, CreatureType::Human);

	res->StorageIdx = EntityMgr.Monsters.Count;
	EntityMgr.Monsters.Set(res->StorageIdx, &res);

	SASSERT(res);
	return res;
}

void DeleteMonster(Monster* monster)
{
	SASSERT(monster);
	bool wasRemoved = EntityMgr.Entities.Remove(&monster->Uid);
	if (wasRemoved)
	{
		Inventory* inv = GetGame()->InventoryMgr.Inventories.Get(&monster->Creature.InventoryId);
		GetGame()->InventoryMgr.DeleteInventory(inv);
		GetGame()->InventoryMgr.DeleteEquipment(monster->Creature.EquipmentId);

		// Handle Removed
		uint32_t removedIndex = monster->StorageIdx;
		bool wasLast = removedIndex == EntityMgr.Monsters.LastIndex();
		EntityMgr.Monsters.RemoveAtFast(removedIndex);

		// Handle Last Spot moved
		if (!wasLast)
		{
			uint32_t movedId = EntityMgr.Monsters[removedIndex]->Uid;
			WorldEntity* movedEntity = (WorldEntity*)EntityMgr.Entities.Get(&movedId);
			SASSERT(movedEntity->EntityType == EntityTypes::Monster);
			movedEntity->StorageIdx = removedIndex;
		}

		EntityMgr.MonsterPool.destroy(monster);
	}
}

void* GetEntity(uint32_t ent)
{
	if (ent == 0)
		return &EntityMgr.Player;

	return EntityMgr.Entities.Get(&ent);
}

bool DoesEntityExist(uint32_t ent)
{
	return EntityMgr.Entities.Contains(&ent);
}

CreatureData* GetCreatureType(Player* player)
{
	SASSERT(player);
	uint16_t creatureId = player->Creature.CreatureType;
	SASSERT(creatureId < EntityMgr.CreatureDB.Size());
	return &EntityMgr.CreatureDB[creatureId];
}

CreatureData* GetCreatureType(Monster* monster)
{
	SASSERT(monster);
	uint16_t creatureId = monster->Creature.CreatureType;
	SASSERT(creatureId < EntityMgr.CreatureDB.Size());
	return &EntityMgr.CreatureDB[creatureId];
}

CreatureData* GetCreatureType(Creature* creature)
{
	SASSERT(creature);
	uint16_t creatureId = creature->CreatureType;
	SASSERT(creatureId < EntityMgr.CreatureDB.Size());
	return &EntityMgr.CreatureDB[creatureId];
}
