#include "Inventory.h"

#include "Game.h"
#include "ComponentTypes.h"
#include "Entity.h"

#include <utility>

void ItemStack::Remove()
{
	ItemId = 0;
	ItemCount = 0;
}

Item* ItemStack::GetItem(InventoryMgr* invMgr)
{
	SASSERT(ItemId < INV_MAX_ITEMS);
	Item& item = invMgr->Items[ItemId];
	return &item;
}

bool ItemStack::Increment()
{
	if (ItemCount != GetItem(&GetGame()->InventoryMgr)->MaxStackSize)
	{
		++ItemCount;
		return true;
	}
	return false;
}

bool ItemStack::Deincrement()
{
	--ItemCount;
	if (ItemCount == 0)
	{
		Remove();
		return false;
	}
	return true;
}

ItemStack ItemStack::New(uint16_t itemId, uint16_t itemCount)
{
	ItemStack res = {};
	res.ItemId = itemId;
	res.ItemCount = itemCount;
	return res;
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

bool Equipment::UnequipItem(uint32_t entity, CreatureEntity* creature, uint8_t slot)
{
	SASSERT(creature);
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

	uint32_t torch = GetGame()->EntityMgr.CreateEntity();
	GetGame()->ComponentMgr.AddComponent(torch, TransformComponent{});

	Renderable* torchRenderable = GetGame()->ComponentMgr.AddComponent(torch, Renderable{});
	torchRenderable->x = 0;
	torchRenderable->y = 32;
	torchRenderable->SrcWidth = 4;
	torchRenderable->SrcHeight = 4;
	torchRenderable->DstWidth = 4;
	torchRenderable->DstHeight = 4;

	Attachable* torchAttachable = GetGame()->ComponentMgr.AddComponent(torch, Attachable{});
	torchAttachable->EntityId = GetGame()->EntityMgr.Player.EntityId;
	torchAttachable->EntityOrigin = { 8.0f, 8.0f };
	torchAttachable->Local.Origin.x = 2.0f;
	torchAttachable->Local.Origin.y = 2.0f;
	torchAttachable->Local.Position.x = 4.0f;
	torchAttachable->Local.Position.y = -3.0f;
	torchAttachable->Local.Rotation = 0.0f;

	UpdatingLightSource* torchLight = GetGame()->ComponentMgr.AddComponent(torch, UpdatingLightSource{});
	torchLight->MinRadius = 6;
	torchLight->MaxRadius = 7;
	torchLight->Colors[0] = { 250, 190, 200, 200 };
	torchLight->Colors[1] = { 255, 200, 210, 200 };

}

void InventoryMgr::Initialize()
{
	Items::AIR = RegisterItem({ 0 });
	Items::TORCH = RegisterItem({ 1, 16, 16, OnEquipTorch });
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