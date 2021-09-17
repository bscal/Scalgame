#pragma once

#include "GameHeaders.h"

namespace TheGame
{
	class InputHandler
	{
	public:
		const float MIN_ZOOM = 0.1f;
		const float MAX_ZOOM = 3.0f;

		float CameraMoveSpeed = 2.0f;
		float CameraZoomSpeed = 0.05f;
	public:
		void ProcessInputs();
		
	private:
		bool HandleLeftMouseClicK(const Vector2& mousePos);
	};
}



