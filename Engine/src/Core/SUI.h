#pragma once

#include "Nuklear/raylib-nuklear.h"

#include "Core.h"
#include "SString.h"
#include "Structures/SList.h"

struct GameApplication;
struct Font;

#define CONSOLE_MAX_LENGTH 128

struct UIUserData
{
	float Scale;
};

struct UIState
{
	nk_context Ctx;
	nk_user_font Font;
	nk_allocator Allocator;
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

internal void* MemAlloc(nk_handle handle, void* old, nk_size size);
internal void MemFree(nk_handle handle, void* old);
internal bool IsMouseHoveringUI(nk_context* ctx);
internal float 
CalculateTextWidth(nk_handle handle, float height, const char* text, int len);

internal void 
InitializeNuklear(nk_context* nkCtxToInit, UIState* state, Font* font, float fontSize);

internal void DrawDebugPanel(UIState* state);
internal void DrawConsole(UIState* state);

internal void AppendMemoryUsage(UIState* state);