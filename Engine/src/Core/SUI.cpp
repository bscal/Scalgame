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
	return Scal::MemAlloc(size);
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
	Player* p = GetClientPlayer();

	UpdateNuklear(&state->Ctx);

	state->IsMouseHoveringUI = IsMouseHoveringUI(&state->Ctx);

	Color c = { 17, 17, 17, 155 };
	state->Ctx.style.window.fixed_background.data.color = ColorToNuklear(c);

	if (nk_begin(&state->Ctx, "Debug", { 5, 5, 380, 340 },
		NK_WINDOW_NO_SCROLLBAR))
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
		const char* xy = TextFormat("X: %.2f, Y: %.2f",
			p->Transform.Pos.x, p->Transform.Pos.y);
		nk_label(&state->Ctx, xy, NK_TEXT_LEFT);

		const char* tileXY = TextFormat("TX: %d, TY: %d",
			p->Transform.TilePos.x, p->Transform.TilePos.y);
		nk_label(&state->Ctx, tileXY, NK_TEXT_LEFT);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, "Zoom: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx, TextFormat("%.3f", GetGameApp()->Scale), NK_TEXT_LEFT);

		RenderMemoryUsage(state);
	}
	nk_end(&state->Ctx);
}

void DrawUI(UIState* state)
{
	DrawNuklear(&state->Ctx);
}

void RenderMemoryUsage(UIState* state)
{
	const size_t* memUsage = Scal::GetMemoryUsage();

	for (int i = 0; i < Scal::MaxTags; ++i)
	{
		const char* name = Scal::MemoryTagStrings[i];
		size_t size = memUsage[i];

		MemorySizeData memSize = FindMemSize(size);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, name, NK_TEXT_LEFT);
		const char* str = TextFormat("%.3f%cbs",
			memSize.Size, memSize.BytePrefix);
		nk_label(&state->Ctx, str, NK_TEXT_LEFT);
	}

	nk_layout_row_dynamic(&state->Ctx, 16, 2);
	nk_label(&state->Ctx, "UI Memory", NK_TEXT_LEFT);
	const auto& uiMem = state->Ctx.memory;
	MemorySizeData memSizeAlloc = FindMemSize(uiMem.allocated);
	MemorySizeData memSizeNeed = FindMemSize(uiMem.needed);
	const char* str = TextFormat("%.2f%cbs/%.2f%cbs",
		memSizeAlloc.Size, memSizeAlloc.BytePrefix,
		memSizeNeed.Size, memSizeNeed.BytePrefix);
	nk_label(&state->Ctx, str, NK_TEXT_LEFT);

	nk_layout_row_dynamic(&state->Ctx, 16, 2);
	nk_label(&state->Ctx, "UI Allocations", NK_TEXT_LEFT);
	nk_label(&state->Ctx, TextFormat("%d", uiMem.calls), NK_TEXT_LEFT);

}
