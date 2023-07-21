#include "SEntity.h"

#include "Game.h"
#include "SMemory.h"

#include "Structures/SLinkedList.h"
#include "Structures/StaticArray.h"

#include <raylib/src/raymath.h>

struct EntityMgr
{
	SEntity* Entities;
	uint32_t Capacity;
	uint32_t NextId;
	SLinkedList<uint32_t> UnusedIds;
	DynamicArray<CreatureTypeInfo> Creatures;
};

global_var struct EntityMgr EntityMgr;

internal void CreaturesInitialize();

void EntitiesInitialize()
{
	EntityMgr.Creatures.Initialize(CREATURE_MAX, ALLOC_GAME);
	CreaturesInitialize();

	EntityMgr.Capacity = 512;
	EntityMgr.Entities = (SEntity*)SAlloc(ALLOC_GAME, EntityMgr.Capacity * sizeof(SEntity), MemoryTag::Game);

	SEntity* player = GetPlayer();
	SMemClear(player, sizeof(SEntity));
	player->Flags.Set(EFLAG_PLAYER, true);
	player->CreatureType = CREATURE_HUMAN;
	player->Color = WHITE;
	player->Group = GROUP_PLAYER;
	player->SpriteId = Sprites::PLAYER;

	uint8_t layout[4 * 4] =
	{
		2, 0, 0, 2,
		2, 0, 0, 2,
		0, 0, 0, 0,
		0, 0, 0, 0,
	};
	player->Inventory = CreateInventoryLayout({ 4, 4 }, (InventorySlotState*)layout);

	EntityMgr.NextId += 1;
}

internal uint32_t 
EntityNewUid()
{
	if (EntityMgr.UnusedIds.HasNext())
		return EntityMgr.UnusedIds.PopValue();
	else
		return EntityMgr.NextId++;
}

SEntity* EntitySpawn(CreatureType creatureType)
{
	uint32_t id = EntityNewUid();

	if (id > EntityMgr.Capacity)
	{
		size_t oldSize = EntityMgr.Capacity * sizeof(SEntity);

		EntityMgr.Capacity *= 2;
		size_t newSize = EntityMgr.Capacity * sizeof(SEntity);
		
		EntityMgr.Entities = (SEntity*)SRealloc(ALLOC_GAME, EntityMgr.Entities, oldSize, newSize, MemoryTag::Game);
	}

	SEntity* ent = &EntityMgr.Entities[id];

	UID uid;
	uid.Id = id;
	uid.Gen = ent->Uid.Gen;

	SMemClear(ent, sizeof(SEntity));

	ent->Uid = uid;
	ent->CreatureType = creatureType;
	ent->CreatureType = creatureType;
	ent->Color = WHITE;

	CreatureTypeInfo* creature = &EntityMgr.Creatures[creatureType];
	if (creature->OnEnable)
		creature->OnEnable(ent, creature);

	return ent;
}

bool EntityAlive(UID uid)
{
	if (uid.Id > EntityMgr.Capacity)
		return false;

	SEntity* ent = &EntityMgr.Entities[uid.Id];
	
	return (ent->Uid.Gen == uid.Gen && !ent->Flags.Get(EFLAG_DEAD));
}

SEntity* EntityGet(UID uid)
{
	if (uid.Id > EntityMgr.Capacity)
		return nullptr;

	SEntity* ent = &EntityMgr.Entities[uid.Id];

	if (ent->Uid.Number == uid.Number)
		return ent;
	else
		return nullptr;
}

void EntityDestroy(UID uid)
{
	if (uid.Id > EntityMgr.Capacity)
		return;

	SEntity* ent = &EntityMgr.Entities[uid.Id];
	if (ent->Uid.Gen == uid.Gen && !ent->Flags.Get(EFLAG_DEAD))
	{
		ent->Flags.Set(EFLAG_DEAD, true);

		CreatureTypeInfo* creature = &EntityMgr.Creatures[ent->CreatureType];
		if (creature->OnDisable)
			creature->OnDisable(ent, creature);

		ent->Uid.Gen = (ent->Uid.Gen + 1) % UINT8_MAX;

		uint32_t id = uid.Id;
		EntityMgr.UnusedIds.Push(&id);
	}
}

const CreatureTypeInfo* CreatureGet(CreatureType type)
{
	SASSERT(type < CREATURE_MAX);
	return &EntityMgr.Creatures[type];
}

SEntity* GetPlayer()
{
	return &EntityMgr.Entities[UID_PLAYER];
}

internal void
PlayerUpdate(SEntity* player, Game* game)
{
	SASSERT(player);
	SASSERT(game);

	SASSERT(player->Inventory);

	// Get move input & direction
	bool hasMoved = false;
	TileDirection inputMoveDir;
	if (IsKeyPressed(KEY_D))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::East;
	}
	else if (IsKeyPressed(KEY_A))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::West;
	}
	else if (IsKeyPressed(KEY_S))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::South;
	}
	else if (IsKeyPressed(KEY_W))
	{
		hasMoved = true;
		inputMoveDir = TileDirection::North;
	}

	// Handle movement
	if (hasMoved)
	{
		Vector2i tileToMoveTo = player->TilePos + Vec2i_NEIGHTBORS[(uint8_t)inputMoveDir];
		player->LookDir = inputMoveDir;
		if (CanMoveToTile(&game->Universe.World, tileToMoveTo))
		{
			player->TilePos = tileToMoveTo;
			game->CameraLerpTime = 0.0f;
		}
	}

	PlayerClient* playerClient = &game->Client;

	if (IsKeyPressed(KEY_EIGHT))
	{
		if (game->IsInventoryOpen && !playerClient->CursorStack.IsEmpty())
		{
			if (player->Inventory->InsertStack(playerClient->CursorStackLastPos
				, playerClient->ItemSlotOffsetSlot
				, &playerClient->CursorStack
				, playerClient->IsCursorStackFlipped))
			{
				playerClient->CursorStack.Remove();
				playerClient->IsCursorStackFlipped = false;
				playerClient->CursorStackLastPos = {};
				playerClient->ItemSlotOffset = {};
			}
		}
		game->IsInventoryOpen = !game->IsInventoryOpen;
	}

	if (IsKeyPressed(KEY_NINE))
	{
		SASSERT(player->Inventory);

		ItemStack itemStack = ItemStackNew(Items::TORCH, 1);
		player->Inventory->InsertStack({ 2, 2 }, { 0, 0 }, &itemStack, false);
		ItemStack itemStack2 = ItemStackNew(Items::FIRE_STAFF, 1);
		player->Inventory->InsertStack({ 0, 2 }, { 0, 0 }, &itemStack2, false);

		ItemStack stack = ItemStackNew(Items::TORCH, 1);
		EquipItem(player, &stack, 0); // TODO
	}
}

void EntitiesUpdate(Game* game)
{
	PlayerUpdate(GetPlayer(), game);

	for (uint32_t i = 0; i < EntityMgr.NextId; ++i)
	{
		SEntity* ent = &EntityMgr.Entities[i];
		if (!ent->Flags.Get(EFLAG_DEAD))
		{
			SASSERT(ent->CreatureType < CREATURE_MAX);
			CreatureTypeInfo* creature = &EntityMgr.Creatures[ent->CreatureType];
			if (creature->OnUpdate)
				creature->OnUpdate(ent, creature);
		}
	}
}

void EntitiesDraw(Game* game)
{
	Texture2D* entitySheet = &game->Resources.EntitySpriteSheet;
	Vector2 topLeft = GetScreenToWorld2D({}, game->WorldCamera);
	Rectangle screen =
	{
		topLeft.x,
		topLeft.y,
		(float)GetGameApp()->View.Resolution.x,
		(float)GetGameApp()->View.Resolution.y
	};
	
	// 0 Is always player.
	for (uint32_t i = 1; i < EntityMgr.NextId; ++i)
	{
		SEntity* ent = &EntityMgr.Entities[i];
		if (!ent->Flags.Get(EFLAG_DEAD))
		{
			if (CheckCollisionPointRec({ (float)ent->TilePos.x, (float)ent->TilePos.y }, screen))
			{
				Sprite sprite = SpriteGet(ent->SpriteId);
				Vector2 worldPos = ent->TileToWorld();
				bool flip = (ent->LookDir == TileDirection::South || ent->LookDir == TileDirection::West) ? true : false;
				SDrawSprite(entitySheet, sprite, worldPos, ent->Color, flip);
			}
		}
	}

	SEntity* player = GetPlayer();
	const CreatureTypeInfo* type = CreatureGet(player->CreatureType);

	Sprite sprite = SpriteGet(player->SpriteId);
	Vector2 worldPos = player->TileToWorld();
	bool flip = (player->LookDir == TileDirection::South || player->LookDir == TileDirection::West) ? true : false;
	SDrawSprite(entitySheet, sprite, worldPos, player->Color, flip);
	if (!player->Equipment.MainHand.IsEmpty())
	{
		Sprite sprite = player->Equipment.MainHand.GetItem()->Sprite;
		Vector2 offset = Vector2Add(worldPos, { 8, 8 });
		SDrawSubSprite(entitySheet, sprite, offset, type->Skeleton.RHand, player->Color, flip);
	}
	if (!player->Equipment.OffHand.IsEmpty())
	{
		Sprite sprite = player->Equipment.OffHand.GetItem()->Sprite;
		Vector2 offset = Vector2Add(worldPos, { 8, 8 });
		SDrawSubSprite(entitySheet, sprite, offset, type->Skeleton.LHand, player->Color, flip);
	}
}

bool EquipItem(SEntity* entity, const ItemStack* stack, uint8_t slot)
{
	SASSERT(entity);
	SASSERT(stack);
	SASSERT(slot < EQUIPMENT_MAX_SLOTS);

	if (entity && entity->Equipment.Stacks[slot].IsEmpty())
	{
		// TODO check equip conditions?

		entity->Equipment.Stacks[slot] = *stack;

		Item* item = entity->Equipment.Stacks[slot].GetItem();
		if (item->OnEquipCallback)
			item->OnEquipCallback(entity, &entity->Equipment.Stacks[slot], slot);

		return true;
	}
	return false;
}

bool UnquipItem(SEntity* entity, uint8_t slot)
{
	SASSERT(entity);
	SASSERT(slot < EQUIPMENT_MAX_SLOTS);

	if (entity && !entity->Equipment.Stacks[slot].IsEmpty())
	{
		ItemStack* stack = &entity->Equipment.Stacks[slot];
		SMemClear(stack, sizeof(ItemStack));

		return true;
	}
	return false;
}

internal void 
CreaturesInitialize()
{

}
