#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "SUI.h"

#include "Game.h"
#include "ResourceManager.h"
#include "SMemory.h"
#include "Vector2i.h"
#include "CommandMgr.h"

#include <vector>
#include <string>
#include <stdio.h>

bool InitializeUI(UIState* state, GameApplication* gameApp)
{
	InitializeNuklear(&state->Ctx, state, &gameApp->Game->Resources.FontSilver, 16.0f);

	SLOG_INFO("[ UI ] Initialized");

	return true;
}


void UpdateUI(UIState* state)
{
	UpdateNuklear(&state->Ctx);

	state->IsMouseHoveringUI = IsMouseHoveringUI(&state->Ctx);

	if (state->IsDebugPanelOpen)
		DrawDebugPanel(state);

	DrawConsole(state);
}

void DrawUI(UIState* state)
{
	DrawNuklear(&state->Ctx);
}

internal void*
MemAlloc(nk_handle handle, void* old, nk_size size)
{
	return Scal::MemAlloc(size);
}

internal void 
MemFree(nk_handle handle, void* old)
{
	Scal::MemFree(old);
}

internal float 
CalculateTextWidth(nk_handle handle, float height, const char* text, int len)
{
	if (len == 0) return 0.0f;
	// Grab the text with the cropped length so that it only measures the desired string length.
	const char* subtext = TextSubtext(text, 0, len);
	// Spacing is determined by the font size divided by 10.
	return MeasureTextEx(*(Font*)handle.ptr, subtext, height, height / 10.0f).x;
}

internal bool 
IsMouseHoveringUI(nk_context* ctx)
{
	return false;// nk_window_is_any_hovered(ctx) != 0;
}

internal void 
InitializeNuklear(nk_context* nkCtxToInit, UIState* state, Font* font, float fontSize)
{
	state->Font.userdata = nk_handle_ptr(font);
	state->Font.height = fontSize;
	state->Font.width = CalculateTextWidth;

	nkCtxToInit->clip.copy = nk_raylib_clipboard_copy;
	nkCtxToInit->clip.paste = nk_raylib_clipboard_paste;
	nkCtxToInit->clip.userdata = nk_handle_ptr(0);

	state->Allocator.alloc = MemAlloc;
	state->Allocator.free = MemFree;

	if (!nk_init(nkCtxToInit, &state->Allocator, &state->Font))
	{
		SLOG_ERR("[ UI ] Nuklear failed to initialized");
		assert(false);
	}

	NuklearUserData* userData = (NuklearUserData*)Scal::MemAlloc(sizeof(NuklearUserData));

	// Set the internal user data.
	userData->scaling = 1.0f;
	nk_handle userDataHandle;
	userDataHandle.id = 1;
	userDataHandle.ptr = (void*)userData;
	nk_set_user_data(nkCtxToInit, userDataHandle);

	SLOG_INFO("[ UI ] Initialized Nuklear");
}

internal void DrawDebugPanel(UIState* state)
{
	const Player* p = GetClientPlayer();

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

		AppendMemoryUsage(state);
	}
	nk_end(&state->Ctx);
}

internal void AppendMemoryUsage(UIState* state)
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

global_var std::vector<std::string> ConsoleEntries;

internal void DrawConsole(UIState* state)
{
	local_var int heightAnimValue;
	local_var float SuggestionPanelSize;

	if (state->IsConsoleOpen)
	{
		constexpr int CONSOLE_ANIM_SPEED = 900 * 6;
		constexpr int MAX_CONSOLE_HISTORY = 128;
		constexpr float INPUT_HEIGHT = 36.0f;
		constexpr float INPUT_WIDTH = INPUT_HEIGHT + 12.0f;
		constexpr float SCROLL_BAR_OFFSET = 16.0f;
		constexpr float TEXT_ENTRY_HEIGHT = 16.0f;
		constexpr int TEXT_ENTRY_HEIGHT_WITH_PADDING = (int)TEXT_ENTRY_HEIGHT + 4;

		int w = GetScreenWidth();
		int h = GetScreenHeight() * .75f + SuggestionPanelSize;
		if (heightAnimValue < h)
		{
			h = heightAnimValue;
			heightAnimValue += (int)(GetDeltaTime() * CONSOLE_ANIM_SPEED);
		}

		float paddingX = 32.0f;
		float paddingW = (float)w - (paddingX * 2);

		nk_context* ctx = &state->Ctx;

		struct nk_color c = { 117, 117, 117, 200 };
		ctx->style.window.fixed_background.data.color = c;

		struct nk_rect bounds = { paddingX, 0.0f, paddingW, (float)h };
		if (nk_begin(ctx, "Console", bounds, NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_static(ctx, h - INPUT_WIDTH - SuggestionPanelSize, (int)paddingW - SCROLL_BAR_OFFSET, 1);

			if (nk_group_begin(ctx, "Messages", 0))
			{
				nk_layout_row_dynamic(ctx, TEXT_ENTRY_HEIGHT, 1);
				for (int i = 0; i < ConsoleEntries.size(); ++i)
				{
					nk_label(ctx, ConsoleEntries[i].c_str(), NK_TEXT_LEFT);
				}
				nk_group_end(ctx);
			}

			auto& cmdMgr = GetGame()->CommandMgr;
			if (IsKeyPressed(KEY_TAB) && cmdMgr.Suggestions.size() > 0)
			{
				const auto& sug = cmdMgr.Suggestions[0];
				cmdMgr.Length = sug.size();
				Scal::MemCopy(cmdMgr.TextInputMemory, sug.data(), cmdMgr.Length);
			}
			if (IsKeyPressed(KEY_ENTER))
			{
				if (ConsoleEntries.size() > MAX_CONSOLE_HISTORY)
				{
					ConsoleEntries.erase(ConsoleEntries.begin());
				}

				ConsoleEntries.emplace_back(cmdMgr.TextInputMemory, cmdMgr.Length);
				cmdMgr.TryExecuteCommand(std::string_view(cmdMgr.TextInputMemory, cmdMgr.Length));

				if (ConsoleEntries.size() >= h / TEXT_ENTRY_HEIGHT_WITH_PADDING)
				{
					nk_group_set_scroll(ctx, "Messages", 0, ConsoleEntries.size() * TEXT_ENTRY_HEIGHT_WITH_PADDING);
				}
			}

			// *** Command Input ***
			nk_layout_row_static(ctx, INPUT_HEIGHT, (int)paddingW, 1);
			nk_edit_string(&state->Ctx,
				NK_EDIT_SIMPLE,
				cmdMgr.TextInputMemory,
				&cmdMgr.Length,
				sizeof(cmdMgr.TextInputMemory) - 1,
				nk_filter_default);

			if (cmdMgr.Length != cmdMgr.LastLength)
			{
				cmdMgr.LastLength = cmdMgr.Length;
				cmdMgr.PopulateSuggestions(std::string_view(cmdMgr.TextInputMemory, cmdMgr.Length));
			}
			if (cmdMgr.Length > 0)
			{
				SuggestionPanelSize = (float)(24.0f * cmdMgr.Suggestions.size());
				nk_layout_row_dynamic(ctx, TEXT_ENTRY_HEIGHT, 1);
				for (int i = 0; i < cmdMgr.Suggestions.size(); ++i)
				{
					nk_label(ctx, cmdMgr.Suggestions[i].data(), NK_TEXT_LEFT);
				}
			}
			else
			{
				SuggestionPanelSize = 0.0f;
			}
		}
		nk_end(&state->Ctx);
	}
	else
		heightAnimValue = 0;
}
