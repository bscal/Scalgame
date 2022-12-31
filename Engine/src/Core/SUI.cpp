#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "SUI.h"

#include "Game.h"
#include "ResourceManager.h"
#include "SMemory.h"

#include <string>
#include <stdio.h>
#include <rlgl.h>


internal void* MemAlloc(nk_handle handle, void* old, nk_size size)
{
	return Scal::MemRealloc(old, size);
}

internal void MemFree(nk_handle handle, void* old)
{
	Scal::MemFree(old);
}

internal float CalculateTextWidth(nk_handle handle,
	float height, const char* text, int len)
{
	if (len == 0) return 0.0f;
	return MeasureTextEx(*(Font*)handle.ptr, text, height, 0.0f).x;
}

internal bool IsMouseHoveringUI(nk_context* ctx)
{
	return nk_window_is_any_hovered(ctx) != 0;
}

internal void InitializeNuklear(nk_context* nkCtxToInit, UIState* state, Font* font, float fontSize)
{
	state->Font.userdata = nk_handle_ptr(font);
	state->Font.height = fontSize;
	state->Font.width = CalculateTextWidth;

	nkCtxToInit->clip.copy = nk_raylib_clipboard_copy;
	nkCtxToInit->clip.paste = nk_raylib_clipboard_paste;
	nkCtxToInit->clip.userdata = nk_handle_ptr(0);

	state->Allocator.alloc = MemAlloc;
	state->Allocator.free = MemFree;

	if (nk_init(nkCtxToInit, &state->Allocator, &state->Font) == 0)
	{
		S_LOG_ERR("[ UI ] Nuklear failed to initialized");
		return;
	}

	NuklearUserData* userData = (NuklearUserData*)Scal::MemAlloc(sizeof(NuklearUserData));

	// Set the internal user data.
	userData->scaling = 1.0f;
	nk_handle userDataHandle;
	userDataHandle.id = 1;
	userDataHandle.ptr = (void*)userData;
	nk_set_user_data(nkCtxToInit, userDataHandle);

	S_LOG_INFO("[ UI ] Initialized Nuklear");
}

bool InitializeUI(UIState* state, GameApplication* gameApp)
{
	state->App = gameApp;

	InitializeNuklear(&state->Ctx, state, &gameApp->Game->Resources.FontSilver, 16.0f);

	S_LOG_INFO("[ UI ] Initialized");
	return true;
}


void UpdateUI(UIState* state)
{
	UpdateNuklear(&state->Ctx);

	state->IsMouseHoveringUI = IsMouseHoveringUI(&state->Ctx);

	if (nk_begin(&state->Ctx, "Debug", { 5, 5, 400, 250 }, NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "Fps: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx, 
			std::to_string(GetFPS()).c_str(), NK_TEXT_LEFT);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "FrameTime: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx, 
			std::to_string(GetGameApp()->DeltaTime * 1000).c_str(), NK_TEXT_LEFT);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "RenderTime: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx, 
			std::to_string(GetGameApp()->RenderTime * 1000).c_str(), NK_TEXT_LEFT);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "#LoadedChunks: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx,
			std::to_string(GetGameApp()->NumOfLoadedChunks).c_str(), NK_TEXT_LEFT);
	
		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "#UpdatedChunks: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx,
			std::to_string(GetGameApp()->NumOfChunksUpdated).c_str(), NK_TEXT_LEFT);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "X, Y: ", NK_TEXT_LEFT);

		auto p = GetClientPlayer();
		std::string xy;
		xy.reserve(10);
		xy += std::to_string(p->Transform.TilePos.x);
		xy += ", ";
		xy += std::to_string(p->Transform.TilePos.y);
		nk_label(&state->Ctx, xy.c_str(), NK_TEXT_LEFT);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "Zoom: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx,
			std::to_string(GetGameApp()->Scale).c_str(), NK_TEXT_LEFT);
	}
	nk_end(&state->Ctx);
}

void DrawUI(UIState* state)
{
	DrawNuklear(&state->Ctx);
}

void RenderMemoryUsage(UIState* state, uint64_t length,
	const uint32_t* usage, const char* const* usageName)
{
	if (nk_begin(&state->Ctx, "Memory Usage", { 10, 10, 240, 240 },
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		for (int i = 0; i < length; ++i)
		{
			nk_layout_row_dynamic(&state->Ctx, 30, 1);
			auto memSize = FindMemSize(usage[i]);
			char str[32];
			sprintf(str, "%s: %f %cBs", usageName[i], memSize.Size, memSize.BytePrefix);
			nk_label(&state->Ctx, str, NK_TEXT_LEFT);
		}
	}
	nk_end(&state->Ctx);
}
