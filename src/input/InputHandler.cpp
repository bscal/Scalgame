#include "InputHandler.h"

#include "core/GameClient.h"

namespace TheGame
{
	void InputHandler::ProcessInputs()
	{
		Vector2 mousePos = GetMousePosition();

		if (IsKeyDown(KEY_W)) GameClient::Instance().GetMainCamera().target.y -= CameraMoveSpeed;
		else if (IsKeyDown(KEY_S)) GameClient::Instance().GetMainCamera().target.y += CameraMoveSpeed;

		if (IsKeyDown(KEY_A)) GameClient::Instance().GetMainCamera().target.x -= CameraMoveSpeed;
		else if (IsKeyDown(KEY_D)) GameClient::Instance().GetMainCamera().target.x += CameraMoveSpeed;

		GameClient::Instance().GetMainCamera().zoom += GetMouseWheelMove() * CameraZoomSpeed;
		if (GameClient::Instance().GetMainCamera().zoom > MAX_ZOOM) GameClient::Instance().GetMainCamera().zoom = MAX_ZOOM;
		else if (GameClient::Instance().GetMainCamera().zoom < MIN_ZOOM) GameClient::Instance().GetMainCamera().zoom = MIN_ZOOM;

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