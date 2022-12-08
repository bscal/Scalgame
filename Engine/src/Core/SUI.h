#pragma once

#include "Core.h"
#include "Nuklear/nuklear.h"

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

bool InitializeUI(UIState* state, GameApplication* gameApp);
void UpdateUI(UIState* state);
void RenderUI(UIState* state);
void RenderDebugInfo(UIState* state);

bool IsMouseHoveringUI(UIState* state);

void RenderMemoryUsage(UIState* state, uint64_t length,
	const uint32_t* usage, const char* const *usageName);