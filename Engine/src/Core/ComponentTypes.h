#pragma once

#include "Core.h"

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
	uint8_t x;
	uint8_t y;
	int8_t SrcWidth;
	int8_t SrcHeight;
	int8_t DstWidth;
	int8_t DstHeight;
};

struct UpdatingLightSource : Component<UpdatingLightSource>
{
	Color Colors[2];
	Color FinalColor;
	float Radius;
	float MaxRadius;
	float UpdateCounter;
};

struct AttachTransform : Component<AttachTransform>
{
	Entity Entity;
};

struct ChildSprites : Component<ChildSprites>
{
	SList<Renderable> Sprites;
};


