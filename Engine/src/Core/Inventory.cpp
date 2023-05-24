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

ItemStack* Inventory::GetStack(Vector2i16 slot)
{
	if (slot.x >= Width || slot.y >= Height)
		return nullptr;

	uint32_t idx = slot.x + slot.y * Width;
	InventorySlot invSlot = Slots[idx];
	SASSERT(invSlot.InventoryStackIndex < INVENTORY_SLOT_MAX);

	if (static_cast<InventorySlotState>(invSlot.State) == InventorySlotState::FILLED)
		return &Contents[invSlot.InventoryStackIndex].Stack;
	else
		return nullptr;
}

void Inventory::SetStack(Vector2i16 slot, ItemStack* stack)
{
	const Item* item = stack->GetItem();

	InventoryStack* invStack = Contents.PushNew();
	invStack->Slot = slot;
	invStack->Stack = *stack;

	uint16_t invStackIndex = Contents.LastIndex();

	uint16_t xEnd = slot.x + item->Width;
	uint16_t yEnd = slot.y + item->Height;
	for (uint16_t yPos = slot.y; yPos < yEnd; ++yPos)
	{
		for (uint16_t xPos = slot.x; xPos < xEnd; ++xPos)
		{
			uint32_t idx = xPos + yPos * Width;
			Slots[idx].State = (uint32_t)InventorySlotState::FILLED;
			Slots[idx].InventoryStackIndex = invStackIndex;
		}
	}
}

void Inventory::InsertStack(Vector2i16 slot, ItemStack* stack, bool flipped)
{
	SASSERT(slot.x >= 0);
	SASSERT(slot.y >= 0);
	SASSERT(stack);

	const Item* item = stack->GetItem();

	short xEnd;
	short yEnd;
	if (flipped)
	{
		xEnd = slot.x + item->Height;
		yEnd = slot.y + item->Width;
	}	
	else
	{
		xEnd = slot.x + item->Width;
		yEnd = slot.y + item->Height;
	}

	for (short yPos = slot.y; yPos < yEnd; ++yPos)
	{
		for (short xPos = slot.x; xPos < xEnd; ++xPos)
		{
			uint32_t idx = (uint32_t)(xPos + yPos * Width);
			if (idx >= Slots.Count)
				return;
			if (static_cast<InventorySlotState>(Slots[idx].State) != InventorySlotState::EMPTY)
				return;
		}
	}

	InventoryStack* invStack = Contents.PushNew();
	invStack->Slot = slot;
	invStack->Stack = *stack;

	uint32_t invStackIndex = Contents.LastIndex();
	SASSERT(invStackIndex < INVENTORY_SLOT_MAX);

	for (short yPos = slot.y; yPos < yEnd; ++yPos)
	{
		for (short xPos = slot.x; xPos < xEnd; ++xPos)
		{
			uint32_t idx = xPos + yPos * Width;
			Slots[idx].State = (uint32_t)InventorySlotState::FILLED;
			Slots[idx].InventoryStackIndex = (uint16_t)invStackIndex;
			Slots[idx].IsFlipped = flipped;
		}
	}
}

bool Inventory::RemoveStack(Vector2i16 slot)
{
	SASSERT(slot.x < Width);
	SASSERT(slot.y < Height);
	if (slot.x >= Width || slot.y >= Height)
		return false;

	uint16_t slotIdx = slot.x + slot.y * Width;
	InventorySlot invSlot = Slots[slotIdx];

	if (static_cast<InventorySlotState>(invSlot.State) != InventorySlotState::FILLED)
		return false;

	SASSERT(invSlot.InventoryStackIndex < INVENTORY_SLOT_MAX)

	InventoryStack& invStack = Contents[invSlot.InventoryStackIndex];
	Item* item = invStack.Stack.GetItem();
	{
		short xEnd;
		short yEnd;
		if (invSlot.IsFlipped)
		{
			xEnd = invStack.Slot.x + item->Height;
			yEnd = invStack.Slot.y + item->Width;
		}
		else
		{
			xEnd = invStack.Slot.x + item->Width;
			yEnd = invStack.Slot.y + item->Height;
		}

		// Sets all slots containing item to empty
		for (short slotY = invStack.Slot.y; slotY < yEnd; ++slotY)
		{
			for (short slotX = invStack.Slot.x; slotX < xEnd; ++slotX)
			{
				uint32_t idx = slotX + slotY * Width;
				Slots[idx].InventoryStackIndex = UINT16_MAX;
				Slots[idx].State = (uint16_t)InventorySlotState::EMPTY;
				Slots[idx].IsFlipped = 0;
			}
		}
	}
	
	// Removes InventoryStack
	SMemSet((Contents.Memory + invSlot.InventoryStackIndex), 0, sizeof(InventoryStack));
	bool wasLastSwapped = Contents.RemoveAtFast(invSlot.InventoryStackIndex);

	if (wasLastSwapped) // Updates swapped element's index
	{
		InventoryStack& lastInvStack = Contents[invSlot.InventoryStackIndex];
		Item* lastItem = lastInvStack.Stack.GetItem();
		uint16_t slotIdx = lastInvStack.Slot.x + lastInvStack.Slot.y * Width;
		InventorySlot lastInvSlot = Slots[slotIdx];
		{
			short xEnd;
			short yEnd;
			if (lastInvSlot.IsFlipped)
			{
				xEnd = lastInvStack.Slot.x + lastItem->Height;
				yEnd = lastInvStack.Slot.y + lastItem->Width;
			}
			else
			{
				xEnd = lastInvStack.Slot.x + lastItem->Width;
				yEnd = lastInvStack.Slot.y + lastItem->Height;
			}

			for (short slotY = lastInvStack.Slot.y; slotY < yEnd; ++slotY)
			{
				for (short slotX = lastInvStack.Slot.x; slotX < xEnd; ++slotX)
				{
					uint32_t idx = slotX + slotY * Width;
					Slots[idx].InventoryStackIndex = invSlot.InventoryStackIndex;
				}
			}
		}
	}
	return true;
}

bool Inventory::CanInsertStack(Vector2i16 slot, const Item* item, bool flipped) const
{
	short xEnd;
	short yEnd;
	if (flipped)
	{
		xEnd = slot.x + item->Height;
		yEnd = slot.y + item->Width;
	}
	else
	{
		xEnd = slot.x + item->Width;
		yEnd = slot.y + item->Height;
	}
	SASSERT(xEnd >= 0);
	SASSERT(yEnd >= 0);

	for (short yPos = slot.y; yPos < yEnd; ++yPos)
	{
		for (short xPos = slot.x; xPos < xEnd; ++xPos)
		{
			uint32_t idx = xPos + yPos * Width;
			if (idx >= Slots.Count)
				return false;

			if (static_cast<InventorySlotState>(Slots[idx].State) != InventorySlotState::EMPTY)
				return false;
		}
	}
	return true;
}

void ItemStack::Remove()
{
	SMemClear(this, sizeof(ItemStack));
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

Inventory* InventoryMgr::CreateInventory(uint32_t entity, Vector2i16 dimensions)
{
	SASSERT(dimensions.x > 0);
	SASSERT(dimensions.y > 0);

	uint32_t id = NextInventoryId++;
	Inventory* inv = Inventories.InsertKey(&id);
	inv->Width = dimensions.x;
	inv->Height = dimensions.y;
	inv->InventoryId = id;
	inv->OwningEntity = entity;
	inv->Slots.EnsureSize(inv->Width * inv->Height);
	return inv;
}

Inventory* InventoryMgr::CreateInventoryLayout(uint32_t entity, Vector2i16 dimensions, const InventorySlotState* layoutArray)
{
	Inventory* inv = CreateInventory(entity, dimensions);
	for (uint32_t i = 0; i < inv->Slots.Count; ++i)
	{
		inv->Slots[i].State = (uint16_t)layoutArray[i];
	}
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
