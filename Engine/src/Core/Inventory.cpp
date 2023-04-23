#include "Inventory.h"

#include "Game.h"
#include "ComponentTypes.h"

#include <utility>

Item* ItemStack::GetItem(InventoryMgr* invMgr)
{
	SASSERT(ItemId < INV_MAX_ITEMS);
	Item& item = invMgr->Items[ItemId];
	return &item;
}

bool Equipment::EquipItem(uint32_t entity, CreatureEntity* creature, ItemStack* stack, uint8_t slot)
{
	SASSERT(creature);
	SASSERT(stack);
	SASSERT(slot < (uint8_t)EquipmentSlots::MAX_SLOTS);

	Slots[slot] = *stack;

	Item* item = stack->GetItem(&GetGame()->InventoryMgr);
	if (item->OnEquip)
		item->OnEquip(entity, creature, slot, stack);

	return true;
}

bool Equipment::UnequipItem(uint32_t entity, CreatureEntity* creature, ItemStack* stack, uint8_t slot)
{
	SASSERT(creature);
	SASSERT(stack);
	SASSERT(slot < (uint8_t)EquipmentSlots::MAX_SLOTS);

	ItemStack* stack = &Slots[slot];
	// TODO unequip callback
	
	bool hadStack = (stack);
	SMemClear(stack, sizeof(ItemStack));	
	return hadStack;
}

void OnEquipTorch(uint32_t entityId, CreatureEntity* creature, uint16_t slot, ItemStack* itemStack)
{
	SLOG_INFO("EQUIPED");
}

void InventoryMgr::Initialize()
{
	Items::AIR = RegisterItem({ 0 });
	Items::TORCH = RegisterItem({ 1, OnEquipTorch });
}

uint32_t InventoryMgr::RegisterItem(Item&& item)
{
	uint32_t id = NextItemId++;
	Items.Data[id] = std::move(item);
	return id;
}

Inventory* InventoryMgr::CreateInvetory(uint32_t entity)
{
	uint32_t id = NextInventoryId++;
	Inventory* inv = Inventories.InsertKey(&id);
	inv->InventoryId = id;
	inv->OwningEntity = entity;
	return inv;
}

Equipment* InventoryMgr::CreateEquipment()
{
	uint32_t id = NextEquipmentId++;
	Equipment* equipment = Equipments.InsertKey(&id);
	equipment->EquipmentId = id;
	return equipment;
}