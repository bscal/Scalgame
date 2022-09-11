#pragma once

#include "Engine.h"

struct GameApplication;

enum class Direction : uint8_t
{
	North,
	East,
	South,
	West,

	MaxDirections
};

struct Player
{
	Vector2 Position;
	Vector2i TilePosition;
    Rectangle TexturePosition;
	int MaxHealth;
	int Health;
	int MaxMana;
	int Mana;
	int MaxEnergy;
	int Energy;
	Direction LookDirection;
};

bool InitializePlayer(GameApplication* gameApp, Player* outPlayer);

void UpdatePlayer(GameApplication* gameApp, Player* player);
void RenderPlayer(GameApplication* gameApp, Player* player);

float AngleFromDirection(Direction direction);
Vector2i PlayerFoward(Player* player);

void PlayerMove(GameApplication* gameApp, Player* player, Direction direction);
bool PlayerProcessEnergy(GameApplication* gameApp, Player* player, int cost);

constexpr Direction Vector2iToDirection(Vector2i v)
{
	if (v.x < 0)
		return Direction::West;
	else if (v.x > 0)
		return Direction::East;
	else if (v.y < 0)
		return Direction::North;
	else
		return Direction::South;
}
