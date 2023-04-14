#pragma once

#include "Core.h"
#include "Structures/SList.h"
#include "Structures/StaticArray.h"
#include "Structures/SHoodTable.h"

struct InventoryMgr;

constexpr global_var uint32_t INV_EMPTY = UINT32_MAX;

#define INV_MAX_ITEMS 10
#define INV_MAX_INV_TYPES 10
#define EQUIPMENT_MAX_SLOTS 5

struct Item
{
	uint16_t StackSize;
	uint16_t MaxStackSize;
};

struct ItemStack
{
	uint16_t ItemId;
	uint16_t Durability;
	uint16_t MaxDurablity;

	inline const Item* GetItem(const InventoryMgr* invMgr) const
	{
		SASSERT(ItemId < INV_MAX_ITEMS);
		const Item* item = &invMgr->Items[ItemId];
		return item;
	}
};

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
	SList<ItemStack> Items;
	InventoryType::Type Type;
	uint32_t OwningEntity;
	uint32_t InventoryId;
};


struct Equipment
{
	uint32_t EquipmentId;
	StaticArray<bool, EQUIPMENT_MAX_SLOTS> ValidSlots;
	StaticArray<ItemStack, EQUIPMENT_MAX_SLOTS> EquipmentSlots;
};

struct InventoryMgr
{
	StaticArray<Item, INV_MAX_ITEMS> Items;

	SHoodTable<uint32_t, Inventory> Inventories;
	SHoodTable<uint32_t, Equipment> Equipments;
	uint32_t NextInventoryId;
	uint32_t NextEquipmentId;

	inline Inventory* CreateInvetory(uint32_t entity)
	{
		uint32_t id = NextInventoryId++;
		Inventory* inv = Inventories.InsertKey(&id);
		inv->InventoryId = id;
		inv->OwningEntity = entity;
		return inv;
	}

	inline Equipment* CreateEquipment()
	{
		uint32_t id = NextEquipmentId++;
		Equipment* equipment = Equipments.InsertKey(&id);
		equipment->EquipmentId = id;
		return equipment;
	}
};
