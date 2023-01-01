#pragma once

#include "Core.h"
#include "Nuklear/raylib-nuklear.h"

struct GameApplication;
struct Font;

struct UIState
{
	GameApplication* App;
	nk_context Ctx;
	nk_user_font Font;
	nk_allocator Allocator;
	bool IsMouseHoveringUI;
};

internal void* MemAlloc(nk_handle handle, void* old, nk_size size);
internal void MemFree(nk_handle handle, void* old);
internal float CalculateTextWidth(nk_handle handle, float height,
	const char* text, int len);
internal bool IsMouseHoveringUI(nk_context* ctx);
internal void InitializeNuklear(nk_context* nkCtxToInit, UIState* state,
	Font* font, float fontSize);

bool InitializeUI(UIState* state, GameApplication* gameApp);
void UpdateUI(UIState* state);
void DrawUI(UIState* state);

void RenderMemoryUsage(UIState* state);