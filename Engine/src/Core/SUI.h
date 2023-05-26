#pragma once

#include "Core.h"
#include "SString.h"
#include "Structures/SList.h"

#define NK_SINGLE_FILE 0
#include <Nuklear/raylib-nuklear.h>

struct GameApplication;
struct Game;
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
void UpdateUI(UIState* state, Game* game);
void DrawUI(UIState* state);

void HandleGUIInput(UIState* state, GameApplication* gameApp);

NK_API nk_bool
nk_button_sprite(struct nk_context* ctx, struct nk_sprite sprite);

NK_LIB nk_bool
nk_do_button_sprite(nk_flags* state,
    struct nk_command_buffer* out, struct nk_rect bounds,
    struct nk_sprite sprite, enum nk_button_behavior b,
    const struct nk_style_button* style, const struct nk_input* in);

NK_API void
nk_sprite(struct nk_context* ctx, struct nk_sprite sprite);

NK_API void
nk_draw_sprite(struct nk_command_buffer* b, struct nk_rect r,
    const struct nk_sprite* sprite, struct nk_color col);


NK_API void
nk_rect_lines(struct nk_context* ctx, float roundness, float border, struct nk_color color);