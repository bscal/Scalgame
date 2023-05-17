#include "ComponentTypes.h"

#include "Game.h"

void CreatureEntity::OnAdd(uint32_t entity, void* component)
{
	SLOG_INFO("HELLO");
}

void CreatureEntity::OnRemove(uint32_t entity, void* component)
{
	CreatureEntity* creature = (CreatureEntity*)component;

	Inventory* inventoryPtr = GetGame()->InventoryMgr.Inventories.Get(&creature->InventoryId);
	if (inventoryPtr)
		GetGame()->InventoryMgr.RemoveInventory(inventoryPtr);
}