#pragma once

#include "GameHeaders.h"

namespace TheGame
{
	class InputHandler
	{
	public:
		void ProcessInputs();
		
	private:
		bool HandleLeftMouseClicK(const Vector2& mousePos);
	};
}



