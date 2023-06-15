#include "Inventory.h"

#include "Game.h"

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
	if (slot.x < 0 || slot.x >= Width || slot.y < 0 || slot.y >= Height)
		return nullptr;

	uint32_t idx = slot.x + slot.y * Width;
	SASSERT(idx < Slots.Count);
	InventorySlot invSlot = Slots[idx];
	SASSERT(invSlot.InventoryStackIndex < INV_MAX_SLOT_ID);

	if (static_cast<InventorySlotState>(invSlot.State) == InventorySlotState::FILLED)
		return &Contents[invSlot.InventoryStackIndex].Stack;
	else
		return nullptr;
}

bool Inventory::InsertStack(Vector2i16 slot, Vector2i16 offset, ItemStack* stack, bool rotated)
{
	SASSERT(slot.x >= 0);
	SASSERT(slot.y >= 0);
	SASSERT(stack);

	const Item* item = stack->GetItem();
	SASSERT(item);

	if (!CanInsertStack(slot, offset, item, rotated))
		return false;

	slot.x -= offset.x;
	slot.y -= offset.y;

	short xEnd;
	short yEnd;
	if (rotated)
	{
		xEnd = slot.x + item->Height;
		yEnd = slot.y + item->Width;
	}
	else
	{
		xEnd = slot.x + item->Width;
		yEnd = slot.y + item->Height;
	}

	InventoryStack* invStack = Contents.PushNew();
	invStack->Stack = *stack;
	invStack->Slot = slot;
	invStack->IsRotated = rotated;

	uint32_t invStackIndex = Contents.LastIndex();
	SASSERT(invStackIndex < INV_MAX_SLOT_ID);

	for (short yPos = slot.y; yPos < yEnd; ++yPos)
	{
		for (short xPos = slot.x; xPos < xEnd; ++xPos)
		{
			uint32_t idx = xPos + yPos * Width;
			Slots[idx].State = (uint32_t)InventorySlotState::FILLED;
			Slots[idx].InventoryStackIndex = (uint16_t)invStackIndex;
		}
	}

	return true;
}

bool Inventory::CanInsertStack(Vector2i16 slot, Vector2i16 offset, const Item* item, bool flipped) const
{
	SASSERT(offset.x <= item->Width);
	SASSERT(offset.y <= item->Height);

	slot.x -= offset.x;
	slot.y -= offset.y;

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
	if (uint16_t(xEnd) > Width || uint16_t(yEnd) > Height)
		return false;

	for (short yPos = slot.y; yPos < yEnd; ++yPos)
	{
		for (short xPos = slot.x; xPos < xEnd; ++xPos)
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

	SASSERT(invSlot.InventoryStackIndex < INV_MAX_SLOT_ID)

	InventoryStack& invStack = Contents[invSlot.InventoryStackIndex];
	Item* item = invStack.Stack.GetItem();
	{
		short xEnd;
		short yEnd;
		if (invStack.IsRotated)
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
			if (lastInvStack.IsRotated)
			{
				xEnd = lastInvStack.Slot.x + lastItem->Height;
				yEnd = lastInvStack.Slot.y + lastItem->Width;
			}
			else
			{
				xEnd = lastInvStack.Slot.x + lastItem->Width;
				yEnd = lastInvStack.Slot.y + lastItem->Height;
			}
			SASSERT(xEnd >= 0);
			SASSERT(yEnd >= 0);

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

SList<Vector2i16> Inventory::GetIntersectingSlots(Vector2i16 slot, Vector2i16 offset, const Item* item, bool rotated) const
{
	SList<Vector2i16> res = {};
	res.Allocator = SAllocator::Temp;
	res.Reserve(item->Width * item->Height);

	slot.x -= offset.x;
	slot.y -= offset.y;

	short xEnd;
	short yEnd;
	if (rotated)
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

	for (short slotY = slot.y; slotY < yEnd; ++slotY)
	{
		for (short slotX = slot.x; slotX < xEnd; ++slotX)
		{
			uint32_t idx = slotX + slotY * Width;
			if (slotX >= Width || slotY >= Height)
				continue;

			if (static_cast<InventorySlotState>(Slots[idx].State) == InventorySlotState::EMPTY)
			{
				Vector2i16 curSlot = { slotX, slotY };
				res.Push(&curSlot);
			}
		}
	}
	return res;
}

void ItemStack::Remove()
{
	SMemClear(this, sizeof(ItemStack));
}

Item* ItemStack::GetItem() const
{
	SASSERT(ItemId < ITEM_DB_MAX_ITEMS);
	if (ItemId < ITEM_DB_MAX_ITEMS)
	{
		return &GetGame()->ItemDB.Items[ItemId];
	}
	return nullptr;
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

bool EquipItem(uint32_t entityId, Creature* creature, Equipment* equipment, const ItemStack* stack, uint16_t slot)
{
	SASSERT(creature);
	SASSERT(equipment);
	SASSERT(stack);
	SASSERT(slot < EQUIPMENT_MAX_SLOTS);

	if (equipment && equipment->Stacks[slot].IsEmpty())
	{
		// TODO check equip conditions?

		equipment->Stacks[slot] = *stack;

		Item* item = equipment->Stacks[slot].GetItem();
		if (item->OnEquipCallback)
			item->OnEquipCallback(entityId, creature, &equipment->Stacks[slot], slot);

		return true;
	}
	return false;
}

bool UnquipItem(uint32_t entityId, Creature* creature, Equipment* equipment, uint16_t slot)
{
	SASSERT(creature);
	SASSERT(equipment);
	SASSERT(slot < EQUIPMENT_MAX_SLOTS);

	if (equipment && !equipment->Stacks[slot].IsEmpty())
	{
		ItemStack* stack = &equipment->Stacks[slot];
		SMemClear(stack, sizeof(ItemStack));

		return true;
	}
	return false;
}

void OnEquipTorch(uint32_t entityId, Creature* creature, ItemStack* stack, uint16_t slot)
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

void InitializeItems(Game* game)
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

uint16_t RegisterItem(Sprite sprite)
{
	ItemDB* itemDB = &GetGame()->ItemDB;

	SASSERT(itemDB->NextItemId < ITEM_DB_MAX_ITEMS);

	if (itemDB->NextItemId >= ITEM_DB_MAX_ITEMS)
		return UINT16_MAX;

	uint16_t id = itemDB->NextItemId++;
	itemDB->Items[id] = {};
	itemDB->Items[id].Sprite = sprite;
	return id;
}

uint16_t RegisterItemFunc(Sprite sprite, void(*RegisterCallback)(Item* item))
{
	uint16_t id = RegisterItem(sprite);

	RegisterCallback(&GetGame()->ItemDB.Items[id]);

	return id;
}

Inventory* CreateInventory(Vector2i16 dimensions)
{
	SASSERT(dimensions.x > 0);
	SASSERT(dimensions.y > 0);

	InventoryMgr* invMgr = &GetGame()->InventoryMgr;

	uint32_t id = invMgr->NextInventoryId++;
	Inventory* inv = invMgr->Inventories.InsertKey(&id);
	inv->Width = dimensions.x;
	inv->Height = dimensions.y;
	inv->InventoryId = id;
	inv->OwningEntity = ENT_NOT_FOUND;
	inv->Slots.EnsureSize(inv->Width * inv->Height);
	return inv;
}

Inventory* CreateInventoryLayout(Vector2i16 dimensions, const InventorySlotState* layoutArray)
{
	Inventory* inv = CreateInventory(dimensions);
	for (uint32_t i = 0; i < inv->Slots.Count; ++i)
	{
		inv->Slots[i].State = (uint16_t)layoutArray[i];
	}
	return inv;
}

void DeleteInventory(Inventory** invPtr)
{
	SASSERT(invPtr);
	SASSERT(*invPtr);
	Inventory* inventory = *invPtr;
	if (inventory)
	{
		bool wasRemoved = GetGame()->InventoryMgr.Inventories.Remove(&inventory->InventoryId);
		SASSERT(wasRemoved);
		inventory->Slots.Free();
		inventory->Contents.Free();
		inventory->InventoryId = UINT32_MAX;
		*invPtr = nullptr;
	}
}

ItemStack ItemStackNew(uint16_t itemId, uint16_t itemCount)
{
	ItemStack res = {};
	res.ItemId = itemId;
	res.ItemCount = itemCount;
	return res;
}

Inventory* GetInventory(uint32_t inventoryId)
{
	InventoryMgr* invMgr = &GetGame()->InventoryMgr;
	SASSERT(invMgr);

	return invMgr->Inventories.Get(&inventoryId);
}