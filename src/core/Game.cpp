#include "Game.h"

Game::Game()
	: Client({})
{
}

int Game::Start()
{
	Client.Start();
	return 0;
}