#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "SUI.h"

#include "Game.h"
#include "ResourceManager.h"
#include "SMemory.h"
#include "Vector2i.h"
#include "CommandMgr.h"

#include <string>
#include <stdio.h>

const global_var struct nk_color BG_COLOR = ColorToNuklear({ 17, 17, 17, 155 });

internal void DrawFPS(struct nk_context* ctx);

bool InitializeUI(UIState* state, GameApplication* gameApp)
{
	InitializeNuklear(&state->Ctx, state, &gameApp->Game->Resources.FontSilver, 16.0f);
	
	// NOTE: generally you dont need to specify
	// an allocator, but since UIState is allocated
	// directly it is 0
	state->ConsoleEntries.Allocator = SMemAllocator();
	state->ConsoleEntries.Resize(CONSOLE_MAX_LENGTH);

	SLOG_INFO("[ UI ] Initialized");

	return true;
}


void UpdateUI(UIState* state)
{
	SASSERT_MSG(state->Ctx.memory.needed < state->Ctx.memory.size, 
		"UI needed memory is larger then memory allocated!");

	UpdateNuklear(&state->Ctx);

	state->IsMouseHoveringUI = IsMouseHoveringUI(&state->Ctx);

	if (state->IsDrawingFPS)
		DrawFPS(&state->Ctx);

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
	return SMemAlloc(size);
}

internal void 
MemFree(nk_handle handle, void* old)
{
	SMemFree(old);
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

	state->UIMemorySize = Megabytes(1);
	state->UIMemory = SMemAllocTag(state->UIMemorySize, MemoryTag::UI);
	if (!nk_init_fixed(nkCtxToInit, state->UIMemory,
		state->UIMemorySize, &state->Font))
	{
		SLOG_ERR("[ UI ] Nuklear failed to initialized");
		assert(false);
	}

	NuklearUserData* userData = (NuklearUserData*)SMemAllocTag(
		sizeof(NuklearUserData), MemoryTag::UI);

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

	state->Ctx.style.window.fixed_background.data.color = BG_COLOR;

	if (nk_begin(&state->Ctx, "Debug", { 5, 5, 380, 420 },
		NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(&state->Ctx, 16, 2);

		nk_label(&state->Ctx, "FrameTime:", NK_TEXT_LEFT);
		nk_label(&state->Ctx, TextFormat("% .2f", GetGameApp()->DeltaTime * 1000), NK_TEXT_LEFT);

		nk_label(&state->Ctx, "RenderTime:", NK_TEXT_LEFT);
		nk_label(&state->Ctx, TextFormat("% .2f", GetGameApp()->RenderTime * 1000), NK_TEXT_LEFT);

		nk_label(&state->Ctx, "Chunks(Load/Up):", NK_TEXT_LEFT);
		nk_label(&state->Ctx, TextFormat("%d/%d",
			GetGameApp()->NumOfLoadedChunks, GetGameApp()->NumOfChunksUpdated), NK_TEXT_LEFT);

		const char* xy = TextFormat("X: %.1f, Y: %.1f",
			p->Transform.Pos.x, p->Transform.Pos.y);
		nk_label(&state->Ctx, xy, NK_TEXT_LEFT);

		const char* tileXY = TextFormat("TX: %d, TY: %d",
			p->Transform.TilePos.x, p->Transform.TilePos.y);
		nk_label(&state->Ctx, tileXY, NK_TEXT_LEFT);

		nk_label(&state->Ctx, "Zoom: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx, TextFormat("%.3f", GetGameApp()->Scale), NK_TEXT_LEFT);

		AppendMemoryUsage(state);
	}
	nk_end(&state->Ctx);
}

internal void AppendMemoryUsage(UIState* state)
{
	const size_t* memUsage = SMemGetTaggedUsages();


	nk_layout_row_dynamic(&state->Ctx, 16, 1);
	nk_label(&state->Ctx, "--- Memory Usage ---", NK_TEXT_LEFT);
	nk_layout_row_dynamic(&state->Ctx, 16, 1);
	MemorySizeData usage = FindMemSize(SMemGetUsage());
	nk_label(&state->Ctx, TextFormat("Tagged: %.2f%cbs", usage.Size, usage.BytePrefix), NK_TEXT_LEFT);
	size_t freeMem = GetMemPoolFreeMemory(GetGameApp()->GameMemory);
	MemorySizeData alloced = FindMemSize(SMemGetAllocated());
	nk_label(&state->Ctx, TextFormat("Allocated: %.2f%cbs", alloced.Size, alloced.BytePrefix), NK_TEXT_LEFT);
	MemorySizeData game = FindMemSize(GetGameApp()->GameMemory.arena.size - freeMem);
	nk_label(&state->Ctx, TextFormat("GameAlloc: %.2f%cbs", game.Size, game.BytePrefix), NK_TEXT_LEFT);
	MemorySizeData temp = FindMemSize(SMemGetTempAllocated());
	nk_label(&state->Ctx, TextFormat("TempAlloc: %.2f%cbs", temp.Size, temp.BytePrefix), NK_TEXT_LEFT);

	nk_layout_row_dynamic(&state->Ctx, 16, 1);
	nk_label(&state->Ctx, "--- UI Memory ---", NK_TEXT_LEFT);
	const auto& uiMem = state->Ctx.memory;
	MemorySizeData memSizeAlloc = FindMemSize(uiMem.allocated);
	MemorySizeData memSizeNeed = FindMemSize(uiMem.needed);
	MemorySizeData memSizeSize = FindMemSize(uiMem.size);
	const char* str = TextFormat("A%.2f%cbs/N%.2f%cbs/S%.2f%cbs",
		memSizeAlloc.Size, memSizeAlloc.BytePrefix,
		memSizeNeed.Size, memSizeNeed.BytePrefix,
		memSizeSize.Size, memSizeSize.BytePrefix);
	nk_label(&state->Ctx, str, NK_TEXT_LEFT);

	// Start at 1, we dont allow allocatios to Unknown
	for (int i = 1; i < (int)MemoryTag::MaxTags; ++i)
	{
		const char* name = MemoryTagStrings[i];
		size_t size = memUsage[i];

		MemorySizeData memSize = FindMemSize(size);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, name, NK_TEXT_LEFT);
		const char* str = TextFormat("%.3f%cbs",
			memSize.Size, memSize.BytePrefix);
		nk_label(&state->Ctx, str, NK_TEXT_LEFT);
	}

	nk_layout_row_dynamic(&state->Ctx, 16, 1);


	nk_label(&state->Ctx, TextFormat("NewCalls: %d", GetNewCalls()), NK_TEXT_LEFT);
}

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
		constexpr int SCROLL_BAR_OFFSET = 16;
		constexpr float TEXT_ENTRY_HEIGHT = 16.0f;
		constexpr int TEXT_ENTRY_HEIGHT_WITH_PADDING = (int)TEXT_ENTRY_HEIGHT + 4;

		int w = GetScreenWidth();
		int h = (float)GetScreenHeight() * .75f + SuggestionPanelSize;
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
				for (int i = 0; i < state->ConsoleEntries.Count; ++i)
				{
					nk_label(ctx, state->ConsoleEntries[i].c_str(), NK_TEXT_LEFT);
				}
				nk_group_end(ctx);
			}

			auto& cmdMgr = GetGame()->CommandMgr;
			if (IsKeyPressed(KEY_TAB) && cmdMgr.Suggestions.size() > 0)
			{
				const auto& sug = cmdMgr.Suggestions[0];
				cmdMgr.Length = (int)sug.size();
				SMemCopy(cmdMgr.TextInputMemory, sug.data(), cmdMgr.Length);
			}
			if (IsKeyPressed(KEY_ENTER))
			{
				if (state->ConsoleEntries.Count > MAX_CONSOLE_HISTORY)
				{
					state->ConsoleEntries.RemoveAt(0);
				}
				std::string* str = state->ConsoleEntries.PushZero();
				str->assign(cmdMgr.TextInputMemory, cmdMgr.Length);

				cmdMgr.TryExecuteCommand(std::string_view(cmdMgr.TextInputMemory, cmdMgr.Length));

				if (state->ConsoleEntries.Count >= h / TEXT_ENTRY_HEIGHT_WITH_PADDING)
				{
					nk_group_set_scroll(ctx, "Messages", 0, state->ConsoleEntries.Count * TEXT_ENTRY_HEIGHT_WITH_PADDING);
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

internal void DrawFPS(struct nk_context* ctx)
{
	float w = (float)GetScreenWidth();
	struct nk_rect bounds = { w - 96.0f, 0.0f, w, 24.0f };
	if (nk_begin(ctx, "FPS", bounds, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT))
	{
		ctx->style.window.fixed_background.data.color = {};
		nk_layout_row_dynamic(ctx, 24.0f, 1);
		nk_label(ctx, TextFormat("FPS: %d", GetFPS()), NK_TEXT_LEFT);
	}
	nk_end(ctx);
}