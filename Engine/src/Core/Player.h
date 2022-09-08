#pragma once

#include "Engine.h"

struct GameApplication;

enum class Direction : uint8_t
{
	North,
	East,
	South,
	West
};

struct Player
{
	Vector2 Position;
	Vector2i TilePosition;
    Rectangle TexturePosition;
	int MaxEnergy;
	int Energy;
	Direction LookDirection;
};

bool InitializePlayer(GameApplication* gameApp, Player* outPlayer);

void UpdatePlayer(GameApplication* gameApp, Player* player);
void RenderPlayer(GameApplication* gameApp, Player* player);

float AngleFromDirection(Direction direction);

void MovePlayer(GameApplication* gameApp, Player* player, int tileX, int tileY);
bool ProcessEnergy(GameApplication* gameApp, Player* player, int cost);
