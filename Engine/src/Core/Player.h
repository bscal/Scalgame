#pragma once

#include "Engine.h"

struct GameApplication;

struct Player
{
	Vector2 Position;
	Vector2i TilePosition;
    Rectangle TexturePosition;
	int MaxEnergy;
	int Energy;
};

bool InitializePlayer(GameApplication* gameApp, Player* outPlayer);

void UpdatePlayer(GameApplication* gameApp, Player* player);
void RenderPlayer(GameApplication* gameApp, Player* player);

void MovePlayer(GameApplication* gameApp, Player* player, int tileX, int tileY);
bool ProcessEnergy(GameApplication* gameApp, Player* player, int cost);
