#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "SUI.h"

#include "Game.h"
#include "Inventory.h"
#include "Vector2i.h"
#include "ResourceManager.h"
#include "CommandMgr.h"
#include "Lighting.h"
#include "SMemory.h"

global_var const int CONSOLE_MAX_LENGTH = 128;
global_var const struct nk_color BG_COLOR = ColorToNuklear({ 17, 17, 17, 155 });

internal void DrawFPS(struct nk_context* ctx);
internal void DrawInventory(struct nk_context* ctx, const Inventory* inv);
internal struct nk_colorf Vec4ToColorf(Vector4 color);
internal Vector4 ColorFToVec4(struct nk_colorf color);
internal struct nk_color ColorFToColor(struct nk_colorf* color);
internal bool ColorFEqual(const struct nk_colorf& v0, const struct nk_colorf& v1);
internal float CalculateTextWidth(nk_handle handle, float height, const char* text, int len);
internal void InitializeNuklear(nk_context* nkCtxToInit, UIState* state, Font* font, float fontSize);
internal void DrawDebugPanel(UIState* state);
internal void DrawConsole(UIState* state);
internal void AppendMemoryUsage(UIState* state);

bool InitializeUI(UIState* state, GameApplication* gameApp)
{
	InitializeNuklear(&state->Ctx, state, &gameApp->Game->Resources.FontSilver, 16.0f);
	
	// Initialize ConsoleEntries
	state->ConsoleEntries.Allocator = SAllocator::Game;
	state->ConsoleEntries.Reserve(CONSOLE_MAX_LENGTH);

	SLOG_INFO("[ UI ] Initialized");

	return true;
}


void UpdateUI(UIState* state, Game* game)
{
	PROFILE_BEGIN();
	SASSERT_MSG(state->Ctx.memory.needed < state->Ctx.memory.size, 
		"UI needed memory is larger then memory allocated!");

	UpdateNuklear(&state->Ctx);

	state->IsMouseHoveringUI = (nk_window_is_any_hovered(&state->Ctx) != 0);

	if (state->IsDrawingFPS)
		DrawFPS(&state->Ctx);

	if (state->IsDebugPanelOpen)
		DrawDebugPanel(state);

	if (game->IsInventoryOpen)
	{
		Inventory* inv = game->InventoryMgr.CreateInventory(UINT32_MAX, 4, 4);
		ItemStack itemStack = ItemStackNew(Items::TORCH, 1);
		inv->SetStack(1, 1, &itemStack);
		DrawInventory(&state->Ctx, inv);
		game->InventoryMgr.RemoveInventory(inv);
	}

	DrawConsole(state);
	PROFILE_END();
}

void DrawUI(UIState* state)
{
	PROFILE_BEGIN();
	DrawNuklear(&state->Ctx);
	PROFILE_END();
}

internal nk_colorf 
Vec4ToColorF(const Vector4 color)
{
	return { color.x, color.y, color.z, color.w };
}

internal Vector4 
ColorFToVec4(struct nk_colorf color)
{
	return { color.r, color.g, color.b, color.a };
}

internal struct nk_color 
ColorFToColor(struct nk_colorf* color)
{
	struct nk_color result;
	result.r = (uint8_t)(color->r * 255.0f);
	result.g = (uint8_t)(color->g * 255.0f);
	result.b = (uint8_t)(color->b * 255.0f);
	result.a = (uint8_t)(color->a * 255.0f);
	return result;
}

internal bool ColorFEqual(const struct nk_colorf& v0, const struct nk_colorf& v1)
{
	return (v0.r == v1.r && v0.g == v1.g && v0.b == v1.b && v0.a == v1.a);
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

internal void 
InitializeNuklear(nk_context* nkCtxToInit, UIState* state, Font* font, float fontSize)
{
	state->Font.userdata = nk_handle_ptr(font);
	state->Font.height = fontSize;
	state->Font.width = CalculateTextWidth;

	nkCtxToInit->clip.copy = nk_raylib_clipboard_copy;
	nkCtxToInit->clip.paste = nk_raylib_clipboard_paste;
	nkCtxToInit->clip.userdata = nk_handle_ptr(0);

	state->UIMemorySize = Megabytes(4);
	state->UIMemory = SAlloc(SAllocator::Game, state->UIMemorySize, MemoryTag::UI);
	if (!nk_init_fixed(nkCtxToInit, state->UIMemory,
		state->UIMemorySize, &state->Font))
	{
		SLOG_ERR("[ UI ] Nuklear failed to initialized");
		SASSERT(false);
	}

	// Set the internal user data.
	state->UserData.Scale = 1.0f;

	nk_handle userDataHandle;
	userDataHandle.id = 1;
	userDataHandle.ptr = &state->UserData;
	nk_set_user_data(nkCtxToInit, userDataHandle);

	SLOG_INFO("[ UI ] Initialized Nuklear");
}

internal void 
DrawDebugPanel(UIState* state)
{
	struct nk_context* ctx = &state->Ctx;
	const PlayerEntity* p = GetClientPlayer();
	
	ctx->style.window.fixed_background.data.color = BG_COLOR;
	
	if (nk_begin(ctx, "Debug", { 4, 4, 500, (float)GetScreenHeight()},
		NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 16, 2);

		nk_label(ctx, "FrameTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetDeltaTime() * 1000.f), NK_TEXT_LEFT);

		nk_label(ctx, "RenderTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->RenderTime * 1000.0), NK_TEXT_LEFT);

		nk_label(ctx, "WorldUpdateTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->UpdateWorldTime * 1000.0), NK_TEXT_LEFT);

		nk_label(ctx, "DebugLightTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->DebugLightTime * 1000.0), NK_TEXT_LEFT);

		nk_layout_row_dynamic(ctx, 16, 1);

		nk_label(ctx, TextFormat("Chunks(Updated/Total): %d/%d"
			, GetGameApp()->NumOfChunksUpdated, GetGameApp()->NumOfLoadedChunks), NK_TEXT_LEFT);

		const char* lightStr = TextFormat("Lights(Updated/Total): %d/%d"
			, GetGameApp()->NumOfLightsUpdated, GetNumOfLights());
		nk_label(ctx, lightStr, NK_TEXT_LEFT);

		nk_layout_row_dynamic(ctx, 16, 2);
		const char* xy = TextFormat("%s", FMT_VEC2(p->Transform.Position));
		nk_label(ctx, xy, NK_TEXT_LEFT);

		Vector2i v = Vector2i::FromVec2(p->Transform.Position);
		v = v.Divide({ TILE_SIZE, TILE_SIZE });
		const char* tileXY = TextFormat("%s", FMT_VEC2I(v));
		nk_label(ctx, tileXY, NK_TEXT_LEFT);

		nk_label(ctx, "Zoom: ", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("%.3f", GetGameApp()->Scale), NK_TEXT_LEFT);

		AppendMemoryUsage(state);

		// Lighting
		nk_spacer(ctx);

		nk_layout_row_dynamic(ctx, 16, 1);
		nk_label(ctx, "--- Lighting ---", NK_TEXT_LEFT);
		{
			nk_layout_row_begin(ctx, NK_DYNAMIC, 16, 2);
			{
				nk_layout_row_push(ctx, .4f);
				float val = GetGame()->LightingRenderer.LightIntensity;
				nk_label(ctx, TextFormat("Light Intensity: %.2f", val), NK_TEXT_LEFT);
				nk_layout_row_push(ctx, .6f);
				if (nk_slider_float(ctx, 0.0f, &val, 2.0f, 0.02f))
					GetGame()->LightingRenderer.LightIntensity = val;
			}
			nk_layout_row_end(ctx);

			//nk_layout_row_begin(ctx, NK_DYNAMIC, 16, 2);
			//{
			//	nk_layout_row_push(ctx, .4f);
			//	float val = GetGame()->Renderer.BloomIntensity;
			//	nk_label(ctx, TextFormat("Bloom Intensity: %.2f", val), NK_TEXT_LEFT);
			//	nk_layout_row_push(ctx, .6f);
			//	if (nk_slider_float(ctx, 0.0f, &val, 2.0f, 0.02f))
			//		GetGame()->Renderer.SetValueAndUniformBloomIntensity(val);
			//}
			//nk_layout_row_end(ctx);

			{
				nk_layout_row_dynamic(ctx, 16, 1);
				Vector3 color = GetGame()->LightingRenderer.AmbientLightColor;
				struct nk_colorf ambientColor = { color.x, color.y, color.z, 1.0f };
				nk_value_color_hex(ctx, "Ambient Color", ColorFToColor(&ambientColor));

				nk_layout_row_dynamic(ctx, 150, 1);
				struct nk_colorf newAmbientColor = nk_color_picker(ctx, ambientColor, NK_RGB);
				if (!ColorFEqual(ambientColor, newAmbientColor))
				{
					GetGame()->LightingRenderer.AmbientLightColor.x = newAmbientColor.r;
					GetGame()->LightingRenderer.AmbientLightColor.y = newAmbientColor.g;
					GetGame()->LightingRenderer.AmbientLightColor.z = newAmbientColor.b;
				}
			}

			{
				nk_layout_row_dynamic(ctx, 16, 1);
				Vector3 color = GetGame()->LightingRenderer.SunlightColor;
				struct nk_colorf skyColor = { color.x, color.y, color.z, 1.0f };
				nk_value_color_hex(ctx, "Sunlight Color", ColorFToColor(&skyColor));

				nk_layout_row_dynamic(ctx, 150, 1);
				struct nk_colorf newSkyColor = nk_color_picker(ctx, skyColor, NK_RGB);
				if (!ColorFEqual(skyColor, newSkyColor))
				{
					GetGame()->LightingRenderer.SunlightColor.x = newSkyColor.r;
					GetGame()->LightingRenderer.SunlightColor.y = newSkyColor.g;
					GetGame()->LightingRenderer.SunlightColor.z = newSkyColor.b;
				}
			}
		}
	}
	nk_end(&state->Ctx);
}

internal void 
AppendMemoryUsage(UIState* state)
{
	nk_layout_row_dynamic(&state->Ctx, 16, 1);

	nk_spacer(&state->Ctx);
	nk_label(&state->Ctx, "--- Memory Usage ---", NK_TEXT_LEFT);
	size_t freeMem = GetMemPoolFreeMemory(GetGameApp()->GameMemory);
	MemorySizeData alloced = FindMemSize(SMemGetAllocated());
	nk_label(&state->Ctx, TextFormat("Total Allocated Memory: %.2f%cbs", alloced.Size, alloced.BytePrefix), NK_TEXT_LEFT);
	MemorySizeData game = FindMemSize(GetGameApp()->GameMemory.arena.size - freeMem);
	nk_label(&state->Ctx, TextFormat("Game Memory: %.2f%cbs", game.Size, game.BytePrefix), NK_TEXT_LEFT);
	MemorySizeData temp = FindMemSize(GetGameApp()->LastFrameTempMemoryUsage);
	nk_label(&state->Ctx, TextFormat("Temp Memory: %.2f%cbs", temp.Size, temp.BytePrefix), NK_TEXT_LEFT); // last frames
	MemorySizeData memSizeNeed = FindMemSize(state->Ctx.memory.needed);
	nk_label(&state->Ctx, TextFormat("UI Memory Needed: %.2f%cbs", memSizeNeed.Size, memSizeNeed.BytePrefix), NK_TEXT_LEFT);

	// Start at 1, we dont allow allocatios to Unknown
	for (int i = 1; i < (int)MemoryTag::MaxTags; ++i)
	{
		const char* name = MemoryTagStrings[i];
		size_t size = SMemGetTaggedUsages()[i];
		MemorySizeData memSize = FindMemSize(size);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, name, NK_TEXT_LEFT);
		const char* str = TextFormat("%.3f%cbs",
			memSize.Size, memSize.BytePrefix);
		nk_label(&state->Ctx, str, NK_TEXT_LEFT);
	}
}

internal void 
DrawConsole(UIState* state)
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
		int h = int((float)GetScreenHeight() * .75f + SuggestionPanelSize);
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
				for (uint32_t i = 0; i < state->ConsoleEntries.Count; ++i)
				{
					nk_label(ctx, state->ConsoleEntries[i].Data(), NK_TEXT_LEFT);
				}
				nk_group_end(ctx);
			}

			CommandMgr* cmdMgr = &GetGame()->CommandMgr;
			if (IsKeyPressed(KEY_TAB) && cmdMgr->Suggestions.Count > 0)
			{
				SStringView sug = cmdMgr->Suggestions[0];
				cmdMgr->Length = (int)sug.Length;
				SMemCopy(cmdMgr->TextInputMemory, sug.Str, cmdMgr->Length);
			}

			// *** Command Input ***
			nk_layout_row_static(ctx, INPUT_HEIGHT, (int)paddingW, 1);
			nk_edit_string(&state->Ctx,
				NK_EDIT_SIMPLE,
				cmdMgr->TextInputMemory,
				&cmdMgr->Length,
				sizeof(cmdMgr->TextInputMemory) - 1,
				nk_filter_default);

			if (IsKeyPressed(KEY_ENTER))
			{
				if (state->ConsoleEntries.Count > MAX_CONSOLE_HISTORY)
				{
					state->ConsoleEntries.RemoveAt(0);
				}

				SString* string = state->ConsoleEntries.PushNew();
				string->Assign(cmdMgr->TextInputMemory);

				cmdMgr->TryExecuteCommand(SStringView(cmdMgr->TextInputMemory, cmdMgr->Length));

				if (state->ConsoleEntries.Count >= uint32_t(h / TEXT_ENTRY_HEIGHT_WITH_PADDING))
				{
					nk_group_set_scroll(ctx, "Messages", 0, state->ConsoleEntries.Count * TEXT_ENTRY_HEIGHT_WITH_PADDING);
				}
			}

			if (cmdMgr->Length != cmdMgr->LastLength)
			{
				cmdMgr->LastLength = cmdMgr->Length;
				cmdMgr->PopulateSuggestions(SStringView(cmdMgr->TextInputMemory, cmdMgr->Length));
			}
			if (cmdMgr->Length > 0)
			{
				SuggestionPanelSize = (float)(24.0f * cmdMgr->Suggestions.Count);
				nk_layout_row_dynamic(ctx, TEXT_ENTRY_HEIGHT, 1);
				for (uint32_t i = 0; i < cmdMgr->Suggestions.Count; ++i)
				{
					nk_label(ctx, cmdMgr->Suggestions[i].Str, NK_TEXT_LEFT);
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

internal void 
DrawFPS(struct nk_context* ctx)
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

internal void
DrawInventory(struct nk_context* ctx, const Inventory* inv)
{
	constexpr float SLOT_SIZE = 32.0f;

	float width = SLOT_SIZE * (inv->Width);
	float height = SLOT_SIZE * (inv->Height);

	struct nk_rect windowRect;
	windowRect.x = 256.0f;
	windowRect.y = 256.0f;
	windowRect.w = width;
	windowRect.h = height;

	ctx->style.window.padding = nk_vec2(0.0f, 0.0f);
	ctx->style.window.border = 0.0f;
	ctx->style.window.spacing.x = 0.0f;
	ctx->style.window.spacing.y = 0.0f;

	if (nk_begin(ctx, "Inventory", windowRect, NK_WINDOW_NO_SCROLLBAR))
	{
		struct nk_image img;
		img.handle.ptr = &GetGame()->Resources.EntitySpriteSheet;
		img.w = 0;
		img.h = 0;
		img.region[0] = 0;
		img.region[1] = 32;
		img.region[2] = 4;
		img.region[3] = 4;

		ctx->style.window.spacing.x = 0.0f;
		ctx->style.window.spacing.y = 0.0f;

		nk_layout_space_begin(ctx, NK_STATIC, height, INT_MAX);

		// Slots
		uint16_t idx = 0;
		for (uint16_t h = 0; h < inv->Height; ++h)
		{
			for (uint16_t w = 0; w < inv->Width; ++w)
			{
				InventorySlot slot = inv->Slots[idx++];

				struct nk_rect rect;
				rect.x = w * SLOT_SIZE;
				rect.y = h * SLOT_SIZE;
				rect.w = SLOT_SIZE;
				rect.h = SLOT_SIZE;
				nk_layout_space_push(ctx, rect);

				switch ((InventorySlotState)slot.State)
				{
					case InventorySlotState::EMPTY:
					{
						nk_window* win = ctx->current;
						SASSERT(win);

						struct nk_rect bounds;
						enum nk_widget_layout_states state;
						state = nk_widget(&bounds, ctx);

						struct nk_color fillColor = { 22, 22, 22, 255 };
						nk_fill_rect(&win->buffer, bounds, 0.0f, fillColor);

						struct nk_color strokeColor = { 88, 88, 88, 255 };
						nk_stroke_rect(&win->buffer, bounds, 0.0f, 1.0f, strokeColor);
					} break;
					case InventorySlotState::FILLED:
					{
						nk_window* win = ctx->current;
						SASSERT(win);

						struct nk_rect bounds;
						enum nk_widget_layout_states state;
						state = nk_widget(&bounds, ctx);

						struct nk_color fillColor = { 22, 22, 22, 255 };
						nk_fill_rect(&win->buffer, bounds, 0.0f, fillColor);

						struct nk_color strokeColor = { 88, 88, 88, 255 };
						nk_stroke_rect(&win->buffer, bounds, 0.0f, 1.0f, strokeColor);

						struct nk_rect imgRect;
						imgRect.x = rect.x + 4;
						imgRect.y = rect.y + 4;
						imgRect.w = SLOT_SIZE - 8;
						imgRect.h = SLOT_SIZE - 8;
						nk_layout_space_push(ctx, imgRect);
						nk_image(ctx, img);
					} break;
					default:
						break;
				}
			}
		}

		// Border
		nk_layout_space_push(ctx, nk_rect(0, 0, width, height));
		{
			nk_window* win = ctx->current;
			SASSERT(win);

			struct nk_rect bounds;
			enum nk_widget_layout_states state;
			state = nk_widget(&bounds, ctx);

			struct nk_color strokeColor = { 88, 88, 88, 255 };
			nk_stroke_rect(&win->buffer, bounds, 0.0f, 2.0f, strokeColor);
		}

		nk_layout_space_end(ctx);
	}
	nk_end(ctx);
}
