#pragma once

#include "GameHeaders.h"
#include "core/GameClient.h"

namespace TheGame
{
	class Game
	{
	public:
		GameClient Client;
	public:
		Game();
		int Start();
	};
}