#include "Game.h"

#include "GameClient.h"

namespace TheGame
{
	Game::Game()
	{
	}

	int Game::Start()
	{
		GameClient::Instance().Start();

		return 0;
	}
}

