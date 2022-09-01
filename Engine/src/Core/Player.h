#pragma once

#include "Engine.h"

struct Game;
struct TileMap;

struct Player
{
	Vector2 Position;

    Rectangle TexturePosition;

	bool IsInitialized;
};

bool InitializePlayer(Player* outPlayer, Game* Game);
void UpdatePlayer(Player* player, Game* game);
void RenderPlayer(Player* player, Game* game);

void MoveToTile(Player* player, Game* Game, int x, int y);
