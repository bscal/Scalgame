#pragma once

#include "Core.h"
#include "Nuklear/nuklear.h"

struct Font;

struct UIState
{
	nk_context Ctx;
	nk_user_font Font;
	nk_allocator Allocator;
	float FontSize;
	bool IsMouseHoveringUI;
};

bool InitializeUI(Font* font, float fontSize, UIState* outState);
void UpdateUI(UIState* state);
void RenderUI(UIState* state);

bool IsMouseHoveringUI(UIState* state);

void RenderMemoryUsage(UIState* state, uint64_t length,
	const uint32_t* usage, const char* const *usageName);