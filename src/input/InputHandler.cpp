#include "InputHandler.h"

#include "core/GameClient.h"

namespace TheGame
{
	void InputHandler::ProcessInputs()
	{
		Vector2 mousePos = GetMousePosition();

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			HandleLeftMouseClicK(mousePos);
		}
	}


	bool InputHandler::HandleLeftMouseClicK(const Vector2& mousePos)
	{
		auto& world = GameClient::Instance().GameWorld->WorldGrid;
		uint32_t index = world.MousePosToIndex(mousePos);

		auto& temperatureTile = world.GetFromIndex(index)->TemperatureTile;
		temperatureTile.SetValue(temperatureTile.GetValue() + 1.0f);

		TraceLog(LOG_INFO, "Mouse pressed! Index: %d, New Value: %f.0", index, temperatureTile.GetValue());

		return true;
	}
}