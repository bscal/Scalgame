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
	const Item* item = stack->GetItem();
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

	if (static_cast<InventorySlotState>(slot.State) != InventorySlotState::FILLED)
		return false;

	SASSERT(slot.InventoryStackIndex != 0xfff)

	InventoryStack& invStack = Contents[slot.InventoryStackIndex];
	Item* item = invStack.Stack.GetItem();

	// Sets all slots containing item to empty
	for (uint16_t slotY = invStack.SlotY; slotY < invStack.SlotY + item->Height; ++slotY)
	{
		for (uint16_t slotX = invStack.SlotX; slotX < invStack.SlotX + item->Width; ++slotX)
		{
			uint32_t idx = slotX + slotY * Width;
			Slots[idx].InventoryStackIndex = UINT16_MAX;
			Slots[idx].State = (uint16_t)InventorySlotState::EMPTY;
		}
	}
	
	// Removes InventoryStack
	SMemSet((Contents.Memory + slot.InventoryStackIndex), 0, sizeof(InventoryStack));
	bool wasLastSwapped = Contents.RemoveAtFast(slot.InventoryStackIndex);

	if (wasLastSwapped) // Updates swapped element's index
	{
		InventoryStack& lastInvStack = Contents[slot.InventoryStackIndex];
		Item* lastItem = lastInvStack.Stack.GetItem();

		for (uint16_t slotY = lastInvStack.SlotY; slotY < lastInvStack.SlotY + lastItem->Height; ++slotY)
		{
			for (uint16_t slotX = lastInvStack.SlotX; slotX < lastInvStack.SlotX + lastItem->Width; ++slotX)
			{
				uint32_t idx = slotX + slotY * Width;
				Slots[idx].InventoryStackIndex = slot.InventoryStackIndex;
			}
		}
	}

	return true;
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

Item* ItemStack::GetItem() const
{
	SASSERT(ItemId < INV_MAX_ITEMS);
	InventoryMgr* invMgr = &GetGame()->InventoryMgr;
	SASSERT(invMgr);
	Item& item = invMgr->Items[ItemId];
	return &item;
}

bool ItemStack::Increment()
{
	if (ItemCount != GetItem()->MaxStackSize)
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

bool Equipment::EquipItem(CreatureEntity* creature, ItemStack* stack, uint8_t slot)
{
	SASSERT(creature);
	SASSERT(stack);
	SASSERT(slot < (uint8_t)EquipmentSlots::MAX_SLOTS);

	Slots[slot] = *stack;

	Item* item = stack->GetItem();
	if (item->OnEquipCallback)
		item->OnEquipCallback(creature, slot, stack);

	return true;
}

bool Equipment::UnequipItem(CreatureEntity* creature, uint8_t slot)
{
	SASSERT(creature);
	SASSERT(slot < (uint8_t)EquipmentSlots::MAX_SLOTS);

	ItemStack* stack = &Slots[slot];
	// TODO unequip callback
	
	bool hadStack = (stack);
	SMemClear(stack, sizeof(ItemStack));	
	return hadStack;
}

void OnEquipTorch(CreatureEntity* creature, uint16_t slot, ItemStack* itemStack)
{
	SLOG_INFO("EQUIPED");

	uint32_t torch = GetGame()->EntityMgr.CreateEntity();
	TransformComponent* transform = GetGame()->ComponentMgr.GetComponent<TransformComponent>(creature->OwningEntity);
	GetGame()->ComponentMgr.AddComponent<TransformComponent>(torch, *transform);

	SpriteRenderer* torchRenderable = GetGame()->ComponentMgr.AddComponent(torch, SpriteRenderer{});
	torchRenderable->Sprite.x = 0;
	torchRenderable->Sprite.y = 32;
	torchRenderable->Sprite.w = 4;
	torchRenderable->Sprite.h = 4;
	torchRenderable->DstWidth = 4;
	torchRenderable->DstHeight = 4;

	Attachable* torchAttachable = GetGame()->ComponentMgr.AddComponent(torch, Attachable{});
	torchAttachable->ParentEntity = creature->OwningEntity;
	torchAttachable->Target = { 8.0f, 8.0f };
	torchAttachable->Local.Position.x = 4.0f;
	torchAttachable->Local.Position.y = -4.0f;
	torchAttachable->Width = 4;

	UpdatingLightSource* torchLight = GetGame()->ComponentMgr.AddComponent(torch, UpdatingLightSource{});
	torchLight->MinRadius = 6;
	torchLight->MaxRadius = 7;
	torchLight->Colors[0] = { 250, 190, 200, 200 };
	torchLight->Colors[1] = { 255, 200, 210, 200 };
}

void InventoryMgr::Initialize()
{
	Items::AIR = RegisterItem({ 0 });
	Items::TORCH = RegisterItemFunc({ 0, 32, 4, 4 }, [](Item* item)
		{
			item->Width = 1;
			item->Height = 1;
			item->MaxStackSize = 1;
			item->OnEquipCallback = OnEquipTorch;
		});
	Items::FIRE_STAFF = RegisterItemFunc(Sprites::FIRE_STAFF, [](Item* item)
		{
			item->Width = 1;
			item->Height = 2;
			item->MaxStackSize = 1;
		});
}

uint16_t InventoryMgr::RegisterItem(Sprite sprite)
{
	SASSERT(NextItemId < INV_MAX_ITEMS);

	if (NextItemId >= INV_MAX_ITEMS)
		return UINT16_MAX;

	uint16_t id = NextItemId++;
	Items[id] = {};
	Items[id].Sprite = sprite;
	return id;
}

uint16_t InventoryMgr::RegisterItemFunc(Sprite sprite, void(*RegisterCallback)(Item* item))
{
	uint16_t id = RegisterItem(sprite);

	RegisterCallback(&Items[id]);

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
