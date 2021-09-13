#pragma once

#include "GameHeaders.h"

namespace TheGame
{
	class Tile
	{
	public:
		virtual void Render(const GameClient& client) = 0;
		virtual void Update() = 0;

		virtual const char* ToString() const { return "Tile"; }
	};
}