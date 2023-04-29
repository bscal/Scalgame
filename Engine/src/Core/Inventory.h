#pragma once

#include "Core.h"
#include "Structures/SList.h"
#include "Structures/StaticArray.h"
#include "Structures/SHoodTable.h"

struct CreatureEntity;
struct InventoryMgr;
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

struct Item
{
	uint16_t MaxStackSize;
	uint8_t Width;
	uint8_t Height;

	void(*OnEquip)(uint32_t entity, CreatureEntity* creature, uint16_t slot, ItemStack* itemStack);
};

struct ItemStack
{
	uint16_t ItemId;
	uint16_t ItemCount;
	uint16_t Durability;
	uint16_t MaxDurablity;

	void Remove();

	Item* GetItem(InventoryMgr* invMgr);

	bool Increment();
	bool Deincrement();

	static ItemStack New(uint16_t itemId, uint16_t itemCount);
};
global_var constexpr ItemStack AIR_ITEMSTACK = {};

enum class InventorySlotState : uint8_t
{
	EMPTY = 0,
	NOT_USED,
	FILLED
};

struct InventorySlot
{
	uint32_t Index : 24;
	uint32_t State : 8;
};

struct Inventory
{
	inline static const uint32_t NOT_FOUND = UINT32_MAX;
	
	SList<InventorySlot> Slots;
	SList<ItemStack> Contents;
	uint32_t OwningEntity;
	uint32_t InventoryId;
	uint16_t Width;
	uint16_t Height;

	uint32_t FindItem(uint32_t item) const;
	ItemStack* GetStack(uint16_t x, uint16_t y);
	void SetStack(uint16_t x, uint16_t y, ItemStack* stack);
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

	bool EquipItem(uint32_t entity, CreatureEntity* creature, ItemStack* stack, uint8_t slot);
	bool UnequipItem(uint32_t entity, CreatureEntity* creature, uint8_t slot);
};

namespace Items
{
inline uint32_t AIR;
inline uint32_t TORCH;
};

struct InventoryMgr
{
	StaticArray<Item, INV_MAX_ITEMS> Items;

	SHoodTable<uint32_t, Inventory> Inventories;
	SHoodTable<uint32_t, Equipment> Equipments;
	
	uint32_t NextItemId;
	uint32_t NextInventoryId;
	uint32_t NextEquipmentId;

	void Initialize();

	uint32_t RegisterItem(const Item& item);

	Inventory* CreateInvetory(uint32_t entity);

	Equipment* CreateEquipment();
};
