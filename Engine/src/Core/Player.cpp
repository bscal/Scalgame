#include "Player.h"

#include "Game.h"
#include "TileMap.h"

bool InitializePlayer(Player* outPlayer, Game* game)
{
	outPlayer->Position = { 0, 0 };
    float yPos = 16.0f * 55.0f;
    outPlayer->TexturePosition = { 0, yPos, 16, 16 };

	return outPlayer->IsInitialized = true;
}

void UpdatePlayer(Player* player, Game* game)
{
    Vector2 position = player->Position;
    auto tileSize = game->MainTileMap.MapTileSize;
    if (IsKeyPressed(KEY_D))
        MoveToTile(player, game, position.x + tileSize, position.y);
    else if (IsKeyPressed(KEY_A))
        MoveToTile(player, game, position.x - tileSize, position.y);
    else if (IsKeyPressed(KEY_S))
        MoveToTile(player, game, position.x, position.y + tileSize);
    else if (IsKeyPressed(KEY_W))
        MoveToTile(player, game, position.x, position.y - tileSize);
}

void RenderPlayer(Player* player, Game* game)
{
    Texture2D spriteSheet = game->Resources.TileSheet;
    DrawTextureRec(spriteSheet,
        player->TexturePosition,
        player->Position,
        WHITE);
}


void MoveToTile(Player* player, Game* game, int x, int y)
{
	player->Position.x = (float)x;
	player->Position.y = (float)y;
}
