#pragma once

#include "Core.h"
#include "Attributes.h"

#include "Structures/SList.h"

typedef uint32_t Entity;

global_var inline uint32_t NextId = 0;

template<typename T>
struct Component
{
	static const uint32_t Id;
};

template<typename T>
const uint32_t Component<T>::Id = NextId++;

struct TransformComponent : Component<TransformComponent>
{
	Vector2 Position;
	Vector2 Origin;
	Vector2 Scale = { 1.0f, 1.0f };
	float Rotation;
	TileDirection LookDir;

	inline Vector2i TilePos() const
	{
		Vector2i v;
		v.x = (int)floorf(Position.x * INVERSE_TILE_SIZE);
		v.y = (int)floorf(Position.y * INVERSE_TILE_SIZE);
		return v;
	}
};

struct Renderable : Component<Renderable>
{
	float x;
	float y;
	int8_t SrcWidth;
	int8_t SrcHeight;
	int8_t DstWidth;
	int8_t DstHeight;
};

struct Attachable : Component<Attachable>
{
	TransformComponent Local;
	Vector2 EntityOrigin;
	uint32_t EntityId;
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

struct SpriteRenderer : Component<SpriteRenderer>
{
	SList<Renderable> Renderables;
};

struct CreatureEntity : Component<CreatureEntity>
{
	AttributesContainer Attributes;
	uint32_t InventoryId;
	uint32_t EquipmentId;
};


