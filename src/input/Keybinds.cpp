#include "Keybinds.h"

namespace TheGame
{
	const float KeyBind::HOLD_DURATION = 0.5f;
	const float KeyBind::TIMEOUT = 0.7f;
	const float KeyBind::DELAY = 0.1f;

	KeyBind::KeyBind(int Key, TheGame::ModifierType modifierType, TheGame::ButtonType buttonType)
		: Key(Key), ModifierType(modifierType), ButtonType(buttonType)
	{
	}

	void KeyBind::Serialize()
	{

	}

	void KeyBind::Process()
	{
		m_Activated = false;
		bool activate = false;

		// Handles checks for DoubleTap and Held ButtonType's
		if (m_InProgress)
		{
			m_Duration += GetFrameTime();

			if (ButtonType == ButtonType::DoubleTap)
				ProcessDoubleTap();
			else if (ButtonType == ButtonType::Held)
				ProcessHeld();
			return;
		}

		if (ButtonType == ButtonType::Release)
			activate = IsKeyReleased(Key);
		else
			activate = IsKeyPressed(Key);		

		if (activate)
		{
			if (ModifierType != ModifierType::None)
				activate = IsKeyDown((int)ModifierType);

			if (ButtonType == ButtonType::DoubleTap || ButtonType == ButtonType::Held)
				m_InProgress = true;
			else
				m_Activated = activate;
		}
	}

	void KeyBind::ProcessHeld()
	{
		if (IsKeyUp(Key))
			Cleanup();

		if (m_Duration > HOLD_DURATION)
		{
			m_Activated = true;
			Cleanup();
		}
	}

	void KeyBind::ProcessDoubleTap()
	{
		if (m_Duration > DELAY && IsKeyPressed(Key))
		{
			if (ModifierType != ModifierType::None && IsKeyUp((int)ModifierType))
				return;

			m_Activated = true;
			Cleanup();
			return;
		}

		if (m_Duration > TIMEOUT)
			Cleanup();
	}

	void KeyBind::Cleanup()
	{
		m_Duration = 0.0f;
		m_InProgress = false;
	}

	class KeyBind;

	std::unordered_map<std::string, KeyBind> KeyBinds::s_BindsMap;

	void KeyBinds::Defaults()
	{
		s_BindsMap.emplace("MoveUp", KeyBind(KEY_W, ModifierType::None, ButtonType::Press));
		s_BindsMap.emplace("MoveDown", KeyBind(KEY_S, ModifierType::Shift, ButtonType::Release));
		s_BindsMap.emplace("MoveLeft", KeyBind(KEY_A, ModifierType::None, ButtonType::Held));
		s_BindsMap.emplace("MoveRight", KeyBind(KEY_D, ModifierType::Alt, ButtonType::DoubleTap));
	}

	void KeyBinds::Process()
	{
		for (auto& [_, val] : s_BindsMap)
		{
			val.Process();
		}
	}

	void KeyBinds::SetBind(const std::string& name, const KeyBind& keyBind)
	{
		if (s_BindsMap.contains(name))
		{
			TraceLog(LOG_ERROR, "Trying to create a bind that already exists. Name: %s", name);
			return;
		}
		s_BindsMap.insert(std::make_pair(name, keyBind));
	}

	void KeyBinds::Serialize()
	{
		
		SaveFileData();
	}
}

