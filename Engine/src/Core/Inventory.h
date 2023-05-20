#pragma once

#include "Core.h"
#include "Sprite.h"
#include "Structures/SList.h"
#include "Structures/StaticArray.h"
#include "Structures/SHoodTable.h"

struct CreatureEntity;
struct ItemStack;

constexpr global_var uint32_t INV_EMPTY = UINT32_MAX;

#define INV_MAX_ITEMS 10
#define INV_MAX_INV_TYPES 10
#define EQUIPMENT_MAX_SLOTS 5

union UID
{
	struct
	{
		uint32_t Gen : 8;
		uint32_t Id : 24;
	};
	uint32_t Mask;
};

namespace Items
{
inline uint16_t AIR;
inline uint16_t TORCH;
inline uint16_t FIRE_STAFF;
};

struct Item
{
	typedef ItemStack(*CreateDefaultStack)();
	typedef void(*OnEquip)(CreatureEntity* creature, uint16_t slot, ItemStack* itemStack);
	typedef void(*OnUnequip)(CreatureEntity* creature, uint16_t slot, ItemStack* itemStack);
	typedef void(*OnUpdate)(CreatureEntity* creature, ItemStack* itemStack);
	typedef void(*OnUse)(CreatureEntity* creature, ItemStack* itemStack, int key);

	OnEquip OnEquipCallback;

	Sprite Sprite;
	uint16_t MaxStackSize;
	uint8_t Width;
	uint8_t Height;
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

struct InventorySlot
{
	uint16_t InventoryStackIndex : 12;
	uint16_t State : 4;
};

struct InventoryStack
{
	ItemStack Stack;
	uint16_t SlotX;
	uint16_t SlotY;
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
	ItemStack* GetStack(uint16_t x, uint16_t y);
	void SetStack(uint16_t x, uint16_t y, ItemStack* stack);
	bool RemoveStack(uint16_t x, uint16_t y);
	bool CanInsertStack(uint16_t x, uint16_t y, const Item* item) const;

};

enum class EquipmentSlots : uint8_t
{
	MAIN_HAND = 0,
	OFF_HAND,

	MAX_SLOTS
};

struct Equipment
{
	uint32_t EquipmentId;
	StaticArray<ItemStack, (uint8_t)EquipmentSlots::MAX_SLOTS> Slots;

	bool EquipItem(CreatureEntity* creature, ItemStack* stack, uint8_t slot);
	bool UnequipItem(CreatureEntity* creature, uint8_t slot);
};

struct InventoryMgr
{
	StaticArray<Item, INV_MAX_ITEMS> Items;

	SHoodTable<uint32_t, Inventory> Inventories;
	SHoodTable<uint32_t, Equipment> Equipments;
	
	uint32_t NextInventoryId;
	uint32_t NextEquipmentId;
	uint16_t NextItemId;

	void Initialize();

	uint16_t RegisterItem(Sprite sprite);
	uint16_t RegisterItemFunc(Sprite sprite, void(*RegisterCallback)(Item* item));

	Inventory* CreateInventory(uint32_t entity, uint16_t width, uint16_t height);
	Inventory* CreateInventoryLayout(uint32_t entity, uint16_t width, uint16_t height, const InventorySlotState* layoutArray);
	void RemoveInventory(Inventory* inventory);

	Equipment* CreateEquipment();
};

ItemStack ItemStackNew(uint16_t itemId, uint16_t itemCount);
