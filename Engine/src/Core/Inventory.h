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

	void(*OnEquip)(uint32_t entity, CreatureEntity* creature, uint16_t slot, ItemStack* itemStack);
};

struct ItemStack
{
	uint16_t ItemId;
	uint16_t ItemCount;
	uint16_t Durability;
	uint16_t MaxDurablity;

	Item* GetItem(InventoryMgr* invMgr);

	bool Increment();
	bool Deincrement();

	static ItemStack New(uint16_t itemId, uint16_t itemCount);
};
global_var constexpr ItemStack AIR_ITEMSTACK = {};

namespace InventoryType
{
enum Type
{
	BASIC,
	MAX_TYPES
};
}

struct Inventory
{
	inline static const uint32_t NOT_FOUND = UINT32_MAX;
	
	SList<ItemStack> Contents;
	InventoryType::Type Type;
	uint32_t OwningEntity;
	uint32_t InventoryId;

	inline uint32_t FindItem(uint32_t item) const
	{
		for (uint32_t i = 0; i < Contents.Count; ++i)
		{
			if (Contents[i].ItemId == item)
				return i;
		}
		return NOT_FOUND;
	}

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

	uint32_t RegisterItem(Item&& item);

	Inventory* CreateInvetory(uint32_t entity);

	Equipment* CreateEquipment();
};
