#pragma once

#include "Game.h"

int main(int argc, char** argv)
{
	Game* game = CreateGame();
	game->Start();
	game->Run();
	game->Shutdown();
	return 0;
}