#include "Inventory.h"

#include "Game.h"
#include "ComponentTypes.h"
#include "Entity.h"

uint32_t Inventory::FindItem(uint32_t item) const
{
	for (uint32_t i = 0; i < Contents.Count; ++i)
	{
		if (Contents[i].Stack.ItemId == item)
			return i;
	}
	return NOT_FOUND;
}

ItemStack* Inventory::GetStack(uint16_t x, uint16_t y)
{
	uint32_t idx = x + y * Width;
	InventorySlot slot = Slots[idx];

	if (static_cast<InventorySlotState>(slot.State) == InventorySlotState::FILLED)
		return &Contents[slot.InventoryStackIndex].Stack;
	else
		return nullptr;
}

void Inventory::SetStack(uint16_t x, uint16_t y, ItemStack* stack)
{
	const Item* item = stack->GetItem(&GetGame()->InventoryMgr);
	if (CanInsertStack(x, y, item))
	{
		InventoryStack* invStack = Contents.PushNew();
		invStack->Stack = *stack;
		invStack->SlotX = x;
		invStack->SlotY = y;

		uint16_t invStackIndex = Contents.LastIndex();

		uint16_t xEnd = x + item->Width;
		uint16_t yEnd = y + item->Height;
		for (uint16_t yPos = y; yPos < yEnd; ++yPos)
		{
			for (uint16_t xPos = x; xPos < xEnd; ++xPos)
			{
				uint32_t idx = xPos + yPos * Width;
				Slots[idx].State = (uint32_t)InventorySlotState::FILLED;
				Slots[idx].InventoryStackIndex = invStackIndex;
			}
		}
	}
}

bool Inventory::RemoveStack(uint16_t x, uint16_t y)
{
	SASSERT(x < Width);
	SASSERT(y < Height);
	if (x >= Width || y >= Height)
		return false;

	uint16_t slotIdx = x + y * Width;
	InventorySlot slot = Slots[slotIdx];

	if (static_cast<InventorySlotState>(Slots[slotIdx].State) != InventorySlotState::FILLED)
		return false;

	SASSERT(slot.InventoryStackIndex != 0xfff)

	InventoryStack& invStack = Contents[slot.InventoryStackIndex];
	Item* item = invStack.Stack.GetItem(&GetGame()->InventoryMgr);

	for (uint16_t y = invStack.SlotY; y < invStack.SlotY + item->Height; ++y)
	{
		for (uint16_t x = invStack.SlotX; x < invStack.SlotX + item->Width; ++x)
		{
			uint32_t idx = x + y * Width;
			Slots[idx].InventoryStackIndex = UINT16_MAX;
			Slots[idx].State = (uint16_t)InventorySlotState::EMPTY;
		}
	}
	
	InventorySlot lastSlot = Slots[Contents.LastIndex()];
	Contents.RemoveAtFast(slot.InventoryStackIndex);
	if (Contents.Count > 0)
	{
		InventoryStack& lastInvStack = Contents[lastSlot.InventoryStackIndex];
		Item* lastItem = lastInvStack.Stack.GetItem(&GetGame()->InventoryMgr);

		for (uint16_t y = lastInvStack.SlotY; y < lastInvStack.SlotY + item->Height; ++y)
		{
			for (uint16_t x = lastInvStack.SlotX; x < lastInvStack.SlotX + item->Width; ++x)
			{
				uint32_t idx = x + y * Width;
				Slots[idx].InventoryStackIndex = lastSlot.InventoryStackIndex;
			}
		}
	}
}

bool Inventory::CanInsertStack(uint16_t x, uint16_t y, const Item* item) const
{
	uint16_t xEnd = x + item->Width;
	uint16_t yEnd = y + item->Height;
	SASSERT(xEnd >= 0);
	SASSERT(yEnd >= 0);

	for (uint16_t yPos = y; yPos < yEnd; ++yPos)
	{
		for (uint16_t xPos = x; xPos < xEnd; ++xPos)
		{
			if (xPos >= Width || yPos >= Height)
				return false;

			uint32_t idx = xPos + yPos * Width;
			if (static_cast<InventorySlotState>(Slots[idx].State) != InventorySlotState::EMPTY)
				return false;
		}
	}
	return true;
}

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

	SpriteRenderer* torchRenderable = GetGame()->ComponentMgr.AddComponent(torch, SpriteRenderer{});
	torchRenderable->Sprite.x = 0;
	torchRenderable->Sprite.y = 32;
	torchRenderable->Sprite.w = 4;
	torchRenderable->Sprite.h = 4;
	torchRenderable->DstWidth = 4;
	torchRenderable->DstHeight = 4;

	Attachable* torchAttachable = GetGame()->ComponentMgr.AddComponent(torch, Attachable{});
	torchAttachable->EntityId = GetGame()->EntityMgr.Player.EntityId;
	torchAttachable->EntityOrigin = { 8.0f, 8.0f };
	torchAttachable->Local.Origin.x = 2.0f;
	torchAttachable->Local.Origin.y = 2.0f;
	torchAttachable->Local.Position.x = 2.0f;
	torchAttachable->Local.Position.y = -5.0f;
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
	Items::TORCH = RegisterItem({ { 0, 32, 4, 4 }, 1, 1, 1, OnEquipTorch });
}

uint32_t InventoryMgr::RegisterItem(const Item& item)
{
	SASSERT(NextItemId < INV_MAX_ITEMS);

	if (NextItemId >= INV_MAX_ITEMS)
		return UINT32_MAX;

	uint32_t id = NextItemId++;
	Items.Data[id] = item;
	return id;
}

Inventory* InventoryMgr::CreateInventory(uint32_t entity, uint16_t width, uint16_t height)
{
	uint32_t id = NextInventoryId++;
	Inventory* inv = Inventories.InsertKey(&id);
	inv->Width = width;
	inv->Height = height;
	inv->InventoryId = id;
	inv->OwningEntity = entity;
	inv->Slots.EnsureSize(width * height);
	return inv;
}

void InventoryMgr::RemoveInventory(Inventory* inventory)
{
	inventory->Slots.Free();
	inventory->Contents.Free();
	inventory->InventoryId = UINT32_MAX;
}

Equipment* InventoryMgr::CreateEquipment()
{
	uint32_t id = NextEquipmentId++;
	Equipment* equipment = Equipments.InsertKey(&id);
	equipment->EquipmentId = id;
	return equipment;
}

ItemStack ItemStackNew(uint16_t itemId, uint16_t itemCount)
{
	ItemStack res = {};
	res.ItemId = itemId;
	res.ItemCount = itemCount;
	return res;
}
