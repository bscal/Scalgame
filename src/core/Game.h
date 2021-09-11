#pragma once

extern "C" {
	#include <raylib.h>
}

#include "src/core/GameClient.h"

class Game
{
public:
	static Game& GetInstance()
	{
		static Game instance;
		return instance;
	}
	GameClient Client;
public:
	Game();
	int Start();
};

