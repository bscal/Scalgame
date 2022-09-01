#include "Player.h"

#include "Game.h"
#include "TileMap.h"

bool InitializePlayer(GameApplication* gameApp, Player* outPlayer)
{
	outPlayer->Position = { 0, 0 };
    outPlayer->TexturePosition = { 32.0f, 16.0f * 55.0f, 16, 16 };
    outPlayer->MaxEnergy = 2;
    outPlayer->Energy = outPlayer->MaxEnergy;
	return true;
}

void UpdatePlayer(GameApplication* gameApp, Player* player)
{
    if (IsKeyPressed(KEY_D))
        MovePlayer(gameApp, player, 1, 0);
    else if (IsKeyPressed(KEY_A))
        MovePlayer(gameApp, player, -1, 0);
    else if (IsKeyPressed(KEY_S))
        MovePlayer(gameApp, player, 0, 1);
    else if (IsKeyPressed(KEY_W))
        MovePlayer(gameApp, player, 0, -1);
}

void RenderPlayer(GameApplication* gameApp, Player* player)
{
    DrawTextureRec(
        gameApp->Resources->TileSheet,
        player->TexturePosition,
        player->Position,
        WHITE);
}

void MovePlayer(GameApplication* gameApp, Player* player, int tileX, int tileY)
{
    Vector2i playerTilePos = player->TilePosition;
    Vector2i newPlayerTilePos = { playerTilePos.x + tileX, playerTilePos.y + tileY };

    TileMap* tileMap = &gameApp->Game->World.MainTileMap;
    if (!IsInBounds(newPlayerTilePos.x, newPlayerTilePos.y,
        tileMap->MapWidth, tileMap->MapHeight))
    {
        return;
    }

    Tile* tile = GetTile(tileMap, playerTilePos.x, playerTilePos.y);
    Tile* tileMoveTo = GetTile(tileMap, newPlayerTilePos.x, newPlayerTilePos.y);
    
    TileType* tileType = GetTileInfo(tileMap, tile->TileId);
    TileType* tileTypeMoveTo = GetTileInfo(tileMap, tileMoveTo->TileId);

    if (ProcessEnergy(gameApp, player, tileTypeMoveTo->MovementCost))
    {
        player->TilePosition.x = newPlayerTilePos.x;
        player->TilePosition.y = newPlayerTilePos.y;
        player->Position.x += tileX * 16;
        player->Position.y += tileY * 16;
    }

}

bool ProcessEnergy(GameApplication* gameApp, Player* player, int cost)
{
    int newEnergy = player->Energy - cost;
    if (newEnergy < 0) return false;

    player->Energy = newEnergy;
    if (newEnergy == 0)
    {
        UpdateTime(gameApp, 1);
    }
    return true;
}
