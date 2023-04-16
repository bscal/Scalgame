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

bool Equipment::EquipItem(uint32_t entity, CreatureEntity* creature, uint16_t slot, ItemStack* stack)
{
	SASSERT(slot < EQUIPMENT_MAX_SLOTS);
	SASSERT(stack);
	EquipmentSlots[slot] = *stack;
	stack->GetItem(&GetGame()->InventoryMgr)->OnEquip(entity, creature, slot, stack);
	return true;
}

void OnEquipTorch(uint32_t entityId, CreatureEntity* creature, uint16_t slot, ItemStack* itemStack)
{
	SLOG_INFO("EQUIPED");
}

void InventoryMgr::Initialize()
{
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