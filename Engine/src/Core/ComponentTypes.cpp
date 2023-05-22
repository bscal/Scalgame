#include "ComponentTypes.h"

#include "Game.h"

void CreatureEntity::OnAdd(uint32_t entity, void* component)
{
	CreatureEntity* creature = (CreatureEntity*)component;

	creature->OwningEntity = entity;

	uint8_t layout[4 * 4] =
	{
		0, 1, 1, 0,
		0, 1, 1, 0,
		1, 1, 1, 1,
		1, 1, 1, 1,
	};
	Inventory* inv = GetGame()->InventoryMgr.CreateInventoryLayout(entity, { 4, 4 }, (InventorySlotState*)layout);

	//Inventory* inv = GetGame()->InventoryMgr.CreateInventory(entity, 10, 6);
	creature->InventoryId = inv->InventoryId;
	Equipment* equipment = GetGame()->InventoryMgr.CreateEquipment();
	creature->EquipmentId = equipment->EquipmentId;
}

void CreatureEntity::OnRemove(uint32_t entity, void* component)
{
	CreatureEntity* creature = (CreatureEntity*)component;

	Inventory* inventoryPtr = GetGame()->InventoryMgr.Inventories.Get(&creature->InventoryId);
	if (inventoryPtr)
		GetGame()->InventoryMgr.RemoveInventory(inventoryPtr);
}