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
void UpdatePlayer(Player* player, Game* game, TileMap* tileMap);
void RenderPlayer(Player* player, Game* game, TileMap* tileMap);

void MoveToTile(Player* player, Game* Game, int x, int y);
