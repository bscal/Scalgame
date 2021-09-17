#pragma once

#include <unordered_map>
#include <string>

#include "GameHeaders.h"

namespace TheGame
{

	enum class ModifierType
	{
		None = 0,
		Shift = KEY_LEFT_SHIFT,
		Alt = KEY_LEFT_ALT,
		Ctrl = KEY_LEFT_CONTROL
	};

	enum class ButtonType
	{
		Press = 0,
		Release,
		Held,
		DoubleTap,
	};

	class KeyBind;

	struct KeyMap
	{
		// TODO 
		static const KeyBind MoveUp;
		static const KeyBind MoveDown;
		static const KeyBind MoveLeft;
		static const KeyBind MoveRight;
	};

	class KeyBinds
	{
	private:
		static std::unordered_map<std::string, KeyBind> s_BindsMap;

		KeyBinds() = default;
		KeyBinds(const KeyBinds& copy) = default;
	public:
		static void Process();
		static void Defaults();
		static void Serialize();
		//static void Deserialize();
		static void SetBind(const std::string& name, const KeyBind& keyBind);
		static inline KeyBind& GetBind(const std::string& name) { return s_BindsMap[name]; }
	};

	class KeyBind
	{
	public:
		static const float HOLD_DURATION;
		static const float TIMEOUT;
		static const float DELAY;

		int Key;
		ModifierType ModifierType;
		ButtonType ButtonType;

	private:
		float m_Duration = 0.0f;
		bool m_InProgress = false;
		bool m_Activated = false;

	public:
		KeyBind() = default;
		KeyBind(int key, TheGame::ModifierType modifierType, TheGame::ButtonType buttonType);
		void Process();

		/// <summary>
		/// Is the bind active for this frame.
		/// </summary>
		inline bool IsActive() const { return m_Activated; }

		void Deserialize();
		void Serialize();

	private:
		void ProcessHeld();
		void ProcessDoubleTap();
		void Cleanup();
	};
}