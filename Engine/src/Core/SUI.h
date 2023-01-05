#pragma once

#include "Core.h"
#include "Nuklear/raylib-nuklear.h"

struct GameApplication;
struct Font;

struct UIState
{
	nk_context Ctx;
	nk_user_font Font;
	nk_allocator Allocator;
	bool IsMouseHoveringUI;
	bool IsDebugPanelOpen;
    bool IsConsoleOpen;
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