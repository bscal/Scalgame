#pragma once

#include "Core.h"
#include "Sprite.h"
#include "Structures/SList.h"
#include "Structures/StaticArray.h"
#include "Structures/SHashMap.h"

struct GameApplication;
struct Game;
struct SEntity;
struct Creature;
struct ItemStack;

constexpr global_var uint32_t INV_EMPTY = UINT32_MAX;
constexpr global_var uint32_t ITEM_DB_MAX_ITEMS = 32;
constexpr global_var uint16_t INV_MAX_SLOT_ID = 0xdfff;

namespace Items
{
inline uint16_t AIR;
inline uint16_t TORCH;
inline uint16_t FIRE_STAFF;
};

//enum class Items : uint16_t
//{
//	AIR = 0,
//	TORCH,
//	FIRE_STAFF,
//};

struct Item
{
	typedef ItemStack(*CreateDefaultStack)();
	typedef void(*OnEquip)(SEntity* entity, ItemStack* stack, uint8_t slot);
	typedef void(*OnUnequip)(SEntity* entity, ItemStack* stack, uint8_t slot);
	typedef void(*OnUpdate)(SEntity* entity, ItemStack* itemStack);
	typedef void(*OnUse)(SEntity* entity, ItemStack* itemStack, int key);

	OnEquip OnEquipCallback;

	uint16_t SpriteId;
	uint16_t MaxStackSize;
	short Width;
	short Height;
};

struct ItemStack
{
	uint16_t ItemId;
	uint16_t ItemCount;
	uint16_t Durability;
	uint16_t MaxDurablity;

	void Remove();

	Item* GetItem() const;

	bool Increment();
	bool Deincrement();

	inline bool IsEmpty() const { return ItemId == Items::AIR; }
};
global_var constexpr ItemStack AIR_ITEMSTACK = {};

enum class InventorySlotState : uint8_t
{
	EMPTY = 0,
	FILLED,
	NOT_USED,
};

struct InventorySlot
{
	uint16_t InventoryStackIndex : 14;
	uint16_t State : 2;
};

struct InventoryStack
{
	ItemStack Stack;
	Vector2i16 Slot;
	bool IsRotated;
};

struct Inventory
{
	inline static const uint32_t NOT_FOUND = UINT32_MAX;

	SList<InventorySlot> Slots;
	SList<InventoryStack> Contents;
	uint32_t OwningEntity;
	uint16_t Width;
	uint16_t Height;

	uint32_t FindItem(uint32_t item) const;
	ItemStack* GetStack(Vector2i16 slot);
	bool InsertStack(Vector2i16 slot, Vector2i16 offset, ItemStack* stack, bool rotated);
	bool CanInsertStack(Vector2i16 slot, Vector2i16 offset, const Item* item, bool rotated) const;
	bool RemoveStack(Vector2i16 slot);
	SList<Vector2i16> GetIntersectingSlots(Vector2i16 slot, Vector2i16 offset, const Item* item, bool rotated) const;
};

struct ItemDB
{
	StaticArray<Item, ITEM_DB_MAX_ITEMS> Items;
	uint16_t NextItemId;
};

struct InventoryMgr
{
	MemoryPool<Inventory> Inventories;
};

// Items
void InitializeItems(Game* game);

uint16_t RegisterItem(uint16_t spriteId);
uint16_t RegisterItemFunc(uint16_t spriteId, void(*RegisterCallback)(Item* item));

ItemStack ItemStackNew(uint16_t itemId, uint16_t itemCount);

// Inventories
Inventory* CreateInventory(Vector2i16 dimensions);
Inventory* CreateInventoryLayout(Vector2i16 dimensions, const InventorySlotState* layout);

void DeleteInventory(Inventory* inventory);
