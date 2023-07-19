#include "Entity.h"

#include "Game.h"
#include "Player.h"
#include "Renderer.h"
#include "Inventory.h"

#include <raylib/src/raymath.h>

global_var EntityManager EntityMgr;

void EntityMgrInitialize()
{
	EntityMgr.Entities.Reserve(256);
	EntityMgr.Monsters.Reserve(256);

	EntityMgr.CreatureDB[CreatureType::Human].Sprite = Sprites::PLAYER_SPRITE;
	EntityMgr.CreatureDB[CreatureType::Human].Skeleton = SKELETON_HUMAN;
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
	Texture2D* spriteSheet = &game->Resources.EntitySpriteSheet;

	for (uint32_t i = 0; i < EntityMgr.Monsters.Count; ++i)
	{
		Monster* monster = EntityMgr.Monsters[i];
		Sprite sprite = GetCreatureType(&monster->Creature)->Sprite;
		Vector2 worldPos = monster->AsPosition();
		SDrawSprite(spriteSheet, monster, worldPos, sprite);
	}

	DrawPlayer(&EntityMgr.Player, game);
}

internal void
NewEntity(uint32_t* uid, WorldEntity* entity, EntityTypes type)
{
	entity->Color = WHITE;
	entity->EntityType = type;

	if (type != EntityTypes::Player)
	{
		*uid = EntityMgr.NextUid++;
		EntityMgr.Entities.Insert(uid, &entity);
	}
}

internal void
InitializeCreature(WorldEntity* entity, Creature* creature, CreatureType type)
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

	switch (entity->EntityType)
	{
		case (EntityTypes::Player):
		{
			uint8_t layout[4 * 4] =
			{
				2, 0, 0, 2,
				2, 0, 0, 2,
				0, 0, 0, 0,
				0, 0, 0, 0,
			};
			Inventory* inv = CreateInventoryLayout({ 4, 4 }, (InventorySlotState*)layout);
			creature->InventoryId = inv->InventoryId;
		} break;

		case (EntityTypes::Monster):
		{
			creature->InventoryId = CreateInventory({ 8, 8 })->InventoryId;
		} break;

		default:
			break;
	}
}

void CreatePlayer(Player* player)
{
	NewEntity(&player->Uid, player, EntityTypes::Player);
	InitializeCreature(player, &player->Creature, CreatureType::Human);

	Inventory* inv = GetInventory(player->Creature.InventoryId);
	ItemStack stack = ItemStackNew(Items::FIRE_STAFF, 1);
	inv->InsertStack({ 2, 2 }, {}, &stack, false);
}

// TODO SpawnLocation, Type
Monster* SpawnMonster()
{
	Monster* res = EntityMgr.MonsterPool.allocate();
	SMemClear(res, sizeof(Monster));
	
	NewEntity(&res->Uid, res, EntityTypes::Monster);

	InitializeCreature(res, &res->Creature, CreatureType::Human);

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
		DeleteInventory(monster->Creature.InventoryId);

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
	return (EntityMgr.Entities.Get(&ent));
}

CreatureData* GetCreatureType(Player* player)
{
	SASSERT(player);
	uint16_t creatureId = player->Creature.CreatureType;
	SASSERT(creatureId < EntityMgr.CreatureDB.Count());
	return &EntityMgr.CreatureDB[creatureId];
}

CreatureData* GetCreatureType(Monster* monster)
{
	SASSERT(monster);
	uint16_t creatureId = monster->Creature.CreatureType;
	SASSERT(creatureId < EntityMgr.CreatureDB.Count());
	return &EntityMgr.CreatureDB[creatureId];
}

CreatureData* GetCreatureType(Creature* creature)
{
	SASSERT(creature);
	uint16_t creatureId = creature->CreatureType;
	SASSERT(creatureId < EntityMgr.CreatureDB.Count());
	return &EntityMgr.CreatureDB[creatureId];
}
