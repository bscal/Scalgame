#pragma once

#include "Core.h"
#include "Sprite.h"
#include "Structures/SList.h"
#include "Structures/StaticArray.h"
#include "Structures/SHashMap.h"

struct GameApplication;
struct Game;
struct CreatureEntity;
struct ItemStack;

constexpr global_var uint32_t INV_EMPTY = UINT32_MAX;
constexpr global_var uint32_t ITEM_DB_MAX_ITEMS = 32;
constexpr global_var uint16_t INV_MAX_SLOT_ID = 0xdfff;
constexpr global_var uint16_t EQUIPMENT_MAX_SLOTS = 6;

namespace Items
{
inline uint16_t AIR;
inline uint16_t TORCH;
inline uint16_t FIRE_STAFF;
};

struct Item
{
	typedef ItemStack(*CreateDefaultStack)();
	typedef void(*OnEquip)(uint32_t uid, Creature* creature, ItemStack* stack, uint16_t slot);
	typedef void(*OnUnequip)(uint32_t uid, Creature* creature, ItemStack* stack, uint16_t slot);
	typedef void(*OnUpdate)(CreatureEntity* creature, ItemStack* itemStack);
	typedef void(*OnUse)(CreatureEntity* creature, ItemStack* itemStack, int key);

	OnEquip OnEquipCallback;

	Sprite Sprite;
	uint16_t MaxStackSize;
	short Width;
	short Height;
};

struct ItemStack
{
	uint16_t ItemId;
	uint16_t ItemCount;
	uint16_t Durability;
	uint16_t MaxDurablity;

	void Remove();

	Item* GetItem() const;

	bool Increment();
	bool Deincrement();

	inline bool IsEmpty() const { return ItemId == Items::AIR; }
};
global_var constexpr ItemStack AIR_ITEMSTACK = {};

enum class InventorySlotState : uint8_t
{
	NOT_USED = 0,
	EMPTY,
	FILLED
};

union Equipment
{
	struct
	{
		ItemStack MainHand;
		ItemStack OffHand;
		ItemStack Head;
		ItemStack Body;
		ItemStack Legs;
		ItemStack Feet;
	};
	ItemStack Stacks[EQUIPMENT_MAX_SLOTS];
};

struct InventorySlot
{
	uint16_t InventoryStackIndex : 14;
	uint16_t State : 2;
};

struct InventoryStack
{
	ItemStack Stack;
	Vector2i16 Slot;
	bool IsRotated;
};

struct Inventory
{
	inline static const uint32_t NOT_FOUND = UINT32_MAX;

	SList<InventorySlot> Slots;
	SList<InventoryStack> Contents;
	uint32_t OwningEntity;
	uint32_t InventoryId;
	uint16_t Width;
	uint16_t Height;

	uint32_t FindItem(uint32_t item) const;
	ItemStack* GetStack(Vector2i16 slot);
	bool InsertStack(Vector2i16 slot, Vector2i16 offset, ItemStack* stack, bool rotated);
	bool CanInsertStack(Vector2i16 slot, Vector2i16 offset, const Item* item, bool rotated) const;
	bool RemoveStack(Vector2i16 slot);
	SList<Vector2i16> GetIntersectingSlots(Vector2i16 slot, Vector2i16 offset, const Item* item, bool rotated) const;
};

struct ItemDB
{
	StaticArray<Item, ITEM_DB_MAX_ITEMS> Items;
	uint16_t NextItemId;
};

struct InventoryMgr
{
	SHashMap<uint32_t, Inventory> Inventories;	
	uint32_t NextInventoryId;
};

// Items
void InitializeItems(Game* game);

uint16_t RegisterItem(Sprite sprite);
uint16_t RegisterItemFunc(Sprite sprite, void(*RegisterCallback)(Item* item));

ItemStack ItemStackNew(uint16_t itemId, uint16_t itemCount);

// Inventories
Inventory* CreateInventory(Vector2i16 dimensions);
Inventory* CreateInventoryLayout(Vector2i16 dimensions, const InventorySlotState* layout);

void DeleteInventory(Inventory** inventory);

Inventory* GetInventory(uint32_t inventoryId);

bool EquipItem(uint32_t entityId, Creature* creature, Equipment* equipment, const ItemStack* stack, uint16_t slot);
bool UnquipItem(uint32_t entityId, Creature* creature, Equipment* equipment, uint16_t slot);
