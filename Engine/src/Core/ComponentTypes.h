#pragma once

#include "Core.h"
#include "Attributes.h"
#include "Sprite.h"

#include "Structures/SList.h"

typedef uint32_t Entity;

inline uint32_t NextId = 0;

template<typename T>
struct Component
{
	static const uint32_t Id;
};

template<typename T>
const uint32_t Component<T>::Id = NextId++;

#define Component(name) struct name : Component<name>

struct TransformComponent
{
	Vector2 Position;
	float Rotation;
	float Scale = 1.0f;
	TileDirection LookDir;

	inline Vector2i TilePos() const
	{
		Vector2i v;
		v.x = (int)floorf(Position.x * INVERSE_TILE_SIZE);
		v.y = (int)floorf(Position.y * INVERSE_TILE_SIZE);
		return v;
	}
};

struct SpriteRenderer : Component<SpriteRenderer>
{
	Sprite Sprite;
	Vector2 Origin;
	uint16_t DstWidth;
	uint16_t DstHeight;
};

struct Attachable : Component<Attachable>
{
	TransformComponent Local;
	Vector2 Target;
	uint32_t ParentEntity;
	uint16_t Width;
};

struct UpdatingLightSource : Component<UpdatingLightSource>
{
	Color Colors[2];
	Color FinalColor;
	float Radius;
	float MinRadius;
	float MaxRadius;
	float UpdateCounter;
};

struct Skeleton
{
	Vector2 Head;
	Vector2 Body;
	Vector2 HandLeft;
	Vector2 HandRight;
};

struct CreatureEntity : Component<CreatureEntity>
{
	Skeleton Skeleton;
	AttributesContainer Attributes;
	uint32_t OwningEntity;
	uint32_t InventoryId;
	uint32_t EquipmentId;
	int MaxEnergy;
	int Energy;

	static void OnAdd(uint32_t entity, void* component);
	static void OnRemove(uint32_t entity, void* component);
};

struct Character
{

};

struct Monster : Component<Monster>
{
	uint16_t MonsterType;
};
