#pragma once

#include "Core.h"
#include "Creature.h"

struct GameApplication;
struct Game;

namespace Scal
{
namespace Creature
{

enum class Direction : uint8_t
{
	North,
	East,
	South,
	West,

	MaxDirections
};

struct Player : public SCreature
{
	void InitializePlayer(::World* world);

	void HandleInput(::Game* game, float dt);
	void UpdatePlayer(::Game* game, float dt);
};

//bool InitializePlayer(GameApplication* gameApp, Player* outPlayer);
//
//void UpdatePlayer(GameApplication* gameApp, Player* player);
//void RenderPlayer(GameApplication* gameApp, Player* player);
//
//float AngleFromDirection(Direction direction);
//Vector2i PlayerFoward(Player* player);
//
//void PlayerMove(GameApplication* gameApp, Player* player, Direction direction);
//void PlayerOnNewTurn(GameApplication* gameApp, Player* player);
//bool PlayerProcessEnergy(GameApplication* gameApp, Player* player, int cost);
//
//constexpr Direction Vector2iToDirection(Vector2i v)
//{
//	if (v.x < 0)
//		return Direction::West;
//	else if (v.x > 0)
//		return Direction::East;
//	else if (v.y < 0)
//		return Direction::North;
//	else
//		return Direction::South;
//}
}
}