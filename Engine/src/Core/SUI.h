#pragma once

#include "Nuklear/raylib-nuklear.h"

#include "Core.h"
#include "SString.h"
#include "Structures/SList.h"

struct GameApplication;
struct Font;

struct UIUserData
{
	float Scale;
};

struct UIState
{
	nk_context Ctx;
	nk_user_font Font;
	void* UIMemory;
	size_t UIMemorySize;
	bool IsMouseHoveringUI;
	bool IsDrawingFPS;
	bool IsDebugPanelOpen;
    bool IsConsoleOpen;
	SList<SString> ConsoleEntries;
	UIUserData UserData;
};

bool InitializeUI(UIState* state, GameApplication* gameApp);
void UpdateUI(UIState* state);
void DrawUI(UIState* state);
