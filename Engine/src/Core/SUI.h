#pragma once

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