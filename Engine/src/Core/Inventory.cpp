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

bool EquipItem(WorldEntity* entity, Creature* creature, const ItemStack* stack, uint8_t slot)
{
	SASSERT(entity);
	SASSERT(creature);
	SASSERT(stack);
	SASSERT(slot < EquipmentSlots::MAX_SLOTS);

	if (creature && creature->Equipment.Stacks[slot].IsEmpty())
	{
		// TODO check equip conditions?

		creature->Equipment.Stacks[slot] = *stack;

		Item* item = creature->Equipment.Stacks[slot].GetItem();
		if (item->OnEquipCallback)
			item->OnEquipCallback(entity, &creature->Equipment.Stacks[slot], slot);

		return true;
	}
	return false;
}

bool UnquipItem(WorldEntity* entity, Creature* creature, uint8_t slot)
{
	SASSERT(entity);
	SASSERT(creature);
	SASSERT(slot < EquipmentSlots::MAX_SLOTS);

	if (creature && !creature->Equipment.Stacks[slot].IsEmpty())
	{
		ItemStack* stack = &creature->Equipment.Stacks[slot];
		SMemClear(stack, sizeof(ItemStack));

		return true;
	}
	return false;
}

void OnEquipTorch(WorldEntity* entity, ItemStack* stack, uint8_t slot)
{
	SLOG_INFO("EQUIPED, %u", entity->TilePos);

	UpdatingLight light = {};
	light.EntityId = entity->Uid;
	light.Pos = entity->TilePos;
	light.MinIntensity = 8.0f;
	light.MaxIntensity = 10.0f;
	light.Colors[0] = WHITE;
	light.Colors[1] = GRAY;
	light.Colors[2] = WHITE;
	light.Colors[3] = BLUE;
	light.Color = light.Colors[0];
	light.Radius = light.MaxIntensity;
	LightAddUpdating(&GetGame()->LightingState, &light);
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
	Items::FIRE_STAFF = RegisterItemFunc(SpriteGet(Sprites::FIRE_STAFF), [](Item* item)
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

void DeleteInventory(uint32_t inventoryId)
{
	SHashMap<uint32_t, Inventory>* inventoryMap = &GetGame()->InventoryMgr.Inventories;
	Inventory* inv = inventoryMap->Get(&inventoryId);
	if (inv)
	{
		inv->Slots.Free();
		inv->Contents.Free();
		inv->InventoryId = UINT32_MAX;
		bool wasRemoved = inventoryMap->Remove(&inventoryId);
		SASSERT(wasRemoved);
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