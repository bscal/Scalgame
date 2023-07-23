#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "SUI.h"

#include "Game.h"
#include "Inventory.h"
#include "Vector2i.h"
#include "ResourceManager.h"
#include "CommandMgr.h"
#include "Lighting.h"
#include "SMemory.h"

#include "raylib/src/raymath.h"

global_var const int CONSOLE_MAX_LENGTH = 128;
global_var const struct nk_color BG_COLOR = ColorToNuklear({ 17, 17, 17, 155 });

internal void DrawGameGUI(UIState* state, Game* game);
internal void DrawFPS(struct nk_context* ctx);
internal void DrawInventory(struct nk_context* ctx, Inventory* inv);
internal void DrawDebugPanel(UIState* state);
internal void AppendMemoryUsage(UIState* state);
internal void DrawConsole(UIState* state);
internal void DrawChatBox(UIState* state, Game* game);

internal Vector4 ColorFToVec4(struct nk_colorf color);
internal struct nk_color ColorFToColor(struct nk_colorf* color);
internal bool ColorFEqual(const struct nk_colorf& v0, const struct nk_colorf& v1);
internal float CalculateTextWidth(nk_handle handle, float height, const char* text, int len);
internal struct nk_rect GetSpriteRect(struct nk_rect rect, bool isRotated);
internal struct nk_sprite GetSprite(Texture2D* texture, Item* item, bool isRotated);

bool InitializeUI(UIState* state, GameApplication* gameApp)
{
	// Initialize Nuklear
	
	Font* font = &gameApp->Game->Resources.FontSilver;

	state->Font.userdata = nk_handle_ptr(font);
	state->Font.height = font->baseSize;
	state->Font.width = CalculateTextWidth;

	state->Ctx.clip.copy = nk_raylib_clipboard_copy;
	state->Ctx.clip.paste = nk_raylib_clipboard_paste;
	state->Ctx.clip.userdata = nk_handle_ptr(0);

	state->UIMemorySize = Megabytes(4);
	state->UIMemory = SAlloc(SAllocator::Game, state->UIMemorySize, MemoryTag::UI);
	if (!nk_init_fixed(&state->Ctx, state->UIMemory,
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
	nk_set_user_data(&state->Ctx, userDataHandle);

	SLOG_INFO("[ UI ] Initialized Nuklear");

	state->GUIRect.x = 0;
	state->GUIRect.y = 0;
	state->GUIRect.w = (float)GetScreenWidth();
	state->GUIRect.h = (float)GetScreenHeight();

	state->ChatBoxStrings.Initialize(64, 64);

	SLOG_INFO("[ UI ] Initialized");
	
	return true;
}

void UpdateUI(UIState* state, GameApplication* gameApp, Game* game)
{
	SASSERT_MSG(state->Ctx.memory.needed < state->Ctx.memory.size,
		"UI needed memory is larger then memory allocated!");

	DrawGameGUI(state, game);

	if (state->IsDrawingFPS)
		DrawFPS(&state->Ctx);

	if (state->IsDebugPanelOpen)
		DrawDebugPanel(state);

	if (game->IsInventoryOpen)
	{
		Inventory* inv = GetClientPlayer()->Inventory;
		DrawInventory(&state->Ctx, inv);
	}

	if (state->IsConsoleOpen)
		DrawConsole(state);
	else
		state->IsConsoleAlreadyOpen = false;
}

void HandleGUIInput(UIState* state, GameApplication* gameApp)
{
	constexpr int INV_FLIP_ITEM = KEY_R;

	SASSERT(state);
	SASSERT(gameApp);

	struct nk_context* ctx = &state->Ctx;
	SASSERT(ctx);

	UpdateNuklear(ctx);

	if (IsKeyPressed(KEY_GRAVE))
		state->IsDebugPanelOpen = !state->IsDebugPanelOpen;
	if (IsKeyPressed(KEY_EQUAL))
		state->IsConsoleOpen = !state->IsConsoleOpen;
	if (IsKeyPressed(KEY_BACKSLASH))
		state->IsDrawingFPS = !state->IsDrawingFPS;

	gameApp->IsGameInputDisabled = state->IsConsoleOpen;

	if (gameApp->Game->IsInventoryOpen)
	{
		struct nk_window* inventoryWindow = nk_window_find(ctx, "Inventory");
		if (!inventoryWindow || (inventoryWindow->flags & NK_WINDOW_CLOSED))
			return;

		if (nk_input_is_mouse_hovering_rect(&ctx->input, inventoryWindow->bounds))
		{
			PlayerClient* playerClient = &gameApp->Game->Client;
			SASSERT(playerClient);
			if (!playerClient->CursorStack.IsEmpty() && IsKeyPressed(INV_FLIP_ITEM))
			{
				playerClient->IsCursorStackFlipped = !playerClient->IsCursorStackFlipped;
			}
		}
	}
}

void DrawUI(UIState* state)
{
	DrawNuklear(&state->Ctx);
}

internal void DrawGameGUI(UIState* state, Game* game)
{
	struct nk_context* ctx = &state->Ctx;

	ctx->style.window.fixed_background.data.color = {};

	if (nk_begin(ctx, "GameGUI", state->GUIRect, NK_WINDOW_NO_SCROLLBAR))
	{
		DrawChatBox(state, game);
	}
	nk_end(ctx);
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
	if (len == 0) return
		0.0f;

	SASSERT(handle.ptr);
	SASSERT(height > 0.0f);
	SASSERT(text);
	SASSERT(len > 0);

	Font* font = (Font*)handle.ptr;
	
	float baseSize = (float)font->baseSize;
	if (height < baseSize)
		height = baseSize;

	int spacing = height / baseSize;

	const char* subtext = TextSubtext(text, 0, len);

	Vector2 textSize = MeasureTextEx(*font, subtext, height, spacing);
	return textSize.x;

	// Grab the text with the cropped length so that it only measures the desired string length.
	//const char* subtext = TextSubtext(text, 0, len);
	// Spacing is determined by the font size divided by 10.
	//return MeasureTextEx(*(Font*)handle.ptr, subtext, height, height / 10.0f).x;
}



internal void
DrawDebugPanel(UIState* state)
{
	struct nk_context* ctx = &state->Ctx;
	SEntity* p = GetClientPlayer();

	ctx->style.window.fixed_background.data.color = BG_COLOR;

	if (nk_begin(ctx, "Debug", { 4, 4, 500, (float)GetScreenHeight() },
		NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 16, 2);

		nk_label(ctx, "FrameTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetDeltaTime() * 1000.f), NK_TEXT_LEFT);

		nk_label(ctx, "RenderTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->RenderTime * 1000.0), NK_TEXT_LEFT);

		nk_label(ctx, "WorldUpdateTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->UpdateWorldTime * 1000.0), NK_TEXT_LEFT);

		nk_label(ctx, "DrawTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->DrawTime * 1000.0), NK_TEXT_LEFT);

		nk_label(ctx, "DebugLightTime:", NK_TEXT_LEFT);
		nk_label(ctx, TextFormat("% .3f", GetGameApp()->DebugLightTime * 1000.0), NK_TEXT_LEFT);

		nk_layout_row_dynamic(ctx, 16, 1);

		nk_label(ctx, TextFormat("Chunks(Updated/Total): %d/%d"
			, GetGameApp()->NumOfChunksUpdated, GetGameApp()->NumOfLoadedChunks), NK_TEXT_LEFT);

		const char* lightStr = TextFormat("Lights(Updated/Total): %d/%d"
			, GetGameApp()->NumOfLightsUpdated, GetNumOfLights());
		nk_label(ctx, lightStr, NK_TEXT_LEFT);

		const char* xy = TextFormat("Pos: %s", FMT_VEC2(p->TileToWorld()));
		nk_label(ctx, xy, NK_TEXT_LEFT);

		const char* tileXY = TextFormat("TilePos: %s", FMT_VEC2I(p->TilePos));
		nk_label(ctx, tileXY, NK_TEXT_LEFT);

		nk_layout_row_dynamic(ctx, 16, 2);

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
	MemorySizeData temp = FindMemSize(SMemGetLastFrameTempUsage());
	nk_label(&state->Ctx, TextFormat("Temp Memory: %.2f%cbs", temp.Size, temp.BytePrefix), NK_TEXT_LEFT); // last frames
	MemorySizeData memSizeNeed = FindMemSize(state->Ctx.memory.needed);
	nk_label(&state->Ctx, TextFormat("UI Memory Needed: %.2f%cbs", memSizeNeed.Size, memSizeNeed.BytePrefix), NK_TEXT_LEFT);

	// Start at 1, we dont allow allocatios to Unknown
	for (uint8_t i = 1; i < (uint8_t)MemoryTag::MaxTags; ++i)
	{
		const char* name = MemoryTagStrings[i];
		size_t size = SMemGetTaggedUsages()[i];
		MemorySizeData memSize = FindMemSize(size);

		nk_layout_row_dynamic(&state->Ctx, 16, 2);
		nk_label(&state->Ctx, name, NK_TEXT_LEFT);
		const char* str = TextFormat("%.3f%cbs", memSize.Size, memSize.BytePrefix);
		nk_label(&state->Ctx, str, NK_TEXT_LEFT);
	}
}

internal void
DrawConsole(UIState* state)
{
	local_var int heightAnimValue;
	local_var float SuggestionPanelSize;

	bool wasConsoleAlreadyOpen = state->IsConsoleAlreadyOpen;
	if (!wasConsoleAlreadyOpen)
		heightAnimValue = 0;
	state->IsConsoleAlreadyOpen = true;

	constexpr int CONSOLE_ANIM_SPEED = 900 * 6;
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
		CommandMgr* cmdMgr = &GetGame()->CommandMgr;
		
		if (!wasConsoleAlreadyOpen)
		{
			if (cmdMgr->ConsoleEntries.StringCount >= uint32_t(h / TEXT_ENTRY_HEIGHT_WITH_PADDING))
			{
				nk_group_set_scroll(ctx, "Messages", 0, cmdMgr->ConsoleEntries.StringCount * TEXT_ENTRY_HEIGHT_WITH_PADDING);
			}
		}

		nk_layout_row_static(ctx, h - INPUT_WIDTH - SuggestionPanelSize, (int)paddingW - SCROLL_BAR_OFFSET, 1);

		if (nk_group_begin(ctx, "Messages", 0))
		{
			nk_layout_row_dynamic(ctx, TEXT_ENTRY_HEIGHT, 1);
			for (int i = cmdMgr->ConsoleEntries.StringCount - 1; i >= 0; --i)
			{
				nk_label(ctx, cmdMgr->ConsoleEntries.StringsMemory[i], NK_TEXT_LEFT);
			}
			nk_group_end(ctx);
		}
		
		if (IsKeyPressed(KEY_TAB))
		{
			if (cmdMgr->Suggestions.Count > 0)
			{
				SMemCopy(cmdMgr->Input, cmdMgr->Suggestions[0].Data(), cmdMgr->Suggestions[0].Length);
				cmdMgr->Length = cmdMgr->Suggestions[0].Length;
				cmdMgr->Input[CONSOLE_STR_LENGTH - 1] = '\0';
			}
		}
		else if (IsKeyPressed(KEY_ENTER))
		{
			char* line = cmdMgr->ConsoleEntries.Push();
			SMemCopy(line, cmdMgr->Input, sizeof(cmdMgr->Input));
			SMemClear(cmdMgr->Input, sizeof(cmdMgr->Input));

			SString input = SString(line, cmdMgr->Length, SAllocator::Temp);

			cmdMgr->Length = 0;

			cmdMgr->TryExecuteCommand(input);

			if (cmdMgr->ConsoleEntries.StringCount >= uint32_t(h / TEXT_ENTRY_HEIGHT_WITH_PADDING))
			{
				nk_group_set_scroll(ctx, "Messages", 0, cmdMgr->ConsoleEntries.StringCount * TEXT_ENTRY_HEIGHT_WITH_PADDING);
			}
		}

		// *** Command Input ***
		//nk_layout_row_dynamic(ctx, INPUT_HEIGHT, 2);
		nk_layout_row_begin(ctx, NK_STATIC, INPUT_HEIGHT, 2);

		float width = CalculateTextWidth(ctx->style.font->userdata, 16.0f, cmdMgr->Input, cmdMgr->Length);
		width += 32.0f;
		nk_layout_row_push(ctx, width);

		nk_edit_string(ctx,
			NK_EDIT_FIELD,
			cmdMgr->Input,
			&cmdMgr->Length,
			cmdMgr->ConsoleEntries.StringCapacity,
			nk_filter_default);

		SString input = SString(cmdMgr->Input, cmdMgr->Length, SAllocator::Temp);

		nk_layout_row_push(ctx, 128.0f);

		const char* commandArgSuggestionString = cmdMgr->PopulateArgSuggestions(input);
		if (commandArgSuggestionString)
			nk_label_colored(ctx, commandArgSuggestionString, NK_TEXT_LEFT, { 255, 0, 0, 255 });

		nk_layout_row_end(ctx);

		SuggestionPanelSize = 0;
		if (cmdMgr->Length > 0)
		{
			cmdMgr->PopulateSuggestions(input);

			SuggestionPanelSize = (float)(cmdMgr->Suggestions.Count) * 24.0f;
			nk_layout_row_dynamic(ctx, TEXT_ENTRY_HEIGHT, 1);
			for (uint32_t i = 0; i < cmdMgr->Suggestions.Count; ++i)
			{
				nk_label(ctx, cmdMgr->Suggestions[i].Data(), NK_TEXT_LEFT);
			}
		}
	}
	nk_end(&state->Ctx);

	nk_window_set_focus(ctx, "Console");
}

internal void
DrawChatBox(UIState* state, Game* game)
{
	struct nk_context* ctx = &state->Ctx;

	if (true)
	{

		struct nk_rect chatBoxRect;
		chatBoxRect.x = (float)GetScreenWidth() - 256;
		chatBoxRect.y = (float)GetScreenHeight() - 256;
		chatBoxRect.w = 256;
		chatBoxRect.h = 256;

		ctx->style.window.fixed_background.data.color = { 155, 22, 22, 255 };

		nk_layout_space_begin(ctx, NK_STATIC, chatBoxRect.h, INT_MAX);
		nk_layout_space_push(ctx, chatBoxRect);

		

		if (nk_group_begin(ctx, "ChatBox", 0))
		{
			nk_layout_row_dynamic(ctx, 16, 1);

			for (uint32_t i = 0; i < state->ChatBoxStrings.StringCount; ++i)
			{
				//char* entryStr = state->ChatBoxStrings.Get(i);

				// TODO: Look into not drawing label if no entry


				nk_label(ctx, "Test Text!", NK_TEXT_ALIGN_LEFT);
			}
			nk_group_end(ctx);
		}
		nk_layout_space_end(ctx);
	}
}

internal void
DrawFPS(struct nk_context* ctx)
{
	float w = (float)GetScreenWidth();
	struct nk_rect bounds = { w - 96.0f, 0.0f, w, 24.0f };
	if (nk_begin(ctx, "FPS", bounds, NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_NO_INPUT))
	{
		ctx->style.window.fixed_background.data.color = { 55, 55, 55, 255 };
		ctx->style.window.fixed_background.data.color = {};
		nk_layout_row_dynamic(ctx, 24.0f, 1);
		nk_label(ctx, TextFormat("FPS: %d", GetFPS()), NK_TEXT_LEFT);
	}
	nk_end(ctx);
}

constexpr global_var struct nk_color BORDER_COLOR = { 88, 88, 88, 255 };
constexpr global_var struct nk_color INV_BG_COLOR = { 22, 22, 22, 255 };
constexpr global_var float SLOT_SIZE = 64.0f;
constexpr global_var float BORDER_SIZE = 2.0f;

internal void
DrawInventory(struct nk_context* ctx, Inventory* inv)
{
	Texture2D* spriteSheet = &GetGame()->Resources.EntitySpriteSheet;
	PlayerClient* playerClient = &GetGame()->Client;

	// Stops us from selecting an item and immediately placing it
	// back into the inventory.
	bool hasHandledInventoryActionThisFrame = false;

	float width = SLOT_SIZE * (float)inv->Width;
	float height = SLOT_SIZE * (float)inv->Height;

	ctx->style.window.padding = nk_vec2(0.0f, 0.0f);
	ctx->style.window.border = 0.0f;
	ctx->style.window.spacing.x = 0.0f;
	ctx->style.window.spacing.y = 0.0f;
	ctx->style.window.background = {};
	ctx->style.window.fixed_background.data.color = {};

	struct nk_rect inventoryRect;
	inventoryRect.x = 0.0f;
	inventoryRect.y = 0.0f;
	inventoryRect.w = GetScreenWidth() / GetNuklearScaling(ctx);
	inventoryRect.h = GetScreenHeight() / GetNuklearScaling(ctx);
	if (nk_begin(ctx, "Inventory", inventoryRect, NK_WINDOW_NO_SCROLLBAR))
	{
		struct nk_vec2 invPos = { 256.0f, 256.0f };
		struct nk_vec2 headerDimensions = { width + BORDER_SIZE, 16.0f };

		// Inventory Title bar
		nk_layout_space_begin(ctx, NK_STATIC, headerDimensions.y, INT_MAX);

		nk_layout_space_push(ctx, nk_rect(invPos.x, invPos.y, headerDimensions.x, headerDimensions.y));
		nk_rect_fill(ctx, 0.0, BORDER_COLOR);

		nk_layout_space_push(ctx, nk_rect(invPos.x + BORDER_SIZE, invPos.y, headerDimensions.x, headerDimensions.y));
		nk_label(ctx, TextFormat("%s's Inventory", "Players Name"), NK_TEXT_ALIGN_LEFT);

		nk_layout_space_end(ctx);

		// Inventory Contents Grid
		nk_layout_space_begin(ctx, NK_STATIC, height, INT_MAX);

		ctx->style.button.border = 0.0f;
		ctx->style.button.padding = { 0.0f, 0.0f };
		ctx->style.button.rounding = 0.0f;

		struct nk_vec2 invSlotsPos = { invPos.x, invPos.y };
		struct nk_vec2 btnScreenCoord = nk_layout_space_to_screen(ctx, invSlotsPos);
		Vector2 invClickedPos = Vector2Subtract(GetMousePosition(), { btnScreenCoord.x, btnScreenCoord.y });
		short btnX = (short)(invClickedPos.x / SLOT_SIZE);
		short btnY = (short)(invClickedPos.y / SLOT_SIZE);
		Vector2i16 invClickedBtn = { btnX, btnY };

		bool canInsert = false;
		SList<Vector2i16> intersectingTiles = {};
		if (!playerClient->CursorStack.IsEmpty())
		{
			intersectingTiles = inv->GetIntersectingSlots(
				invClickedBtn
				, playerClient->ItemSlotOffsetSlot
				, playerClient->CursorStack.GetItem()
				, playerClient->IsCursorStackFlipped);

			canInsert = inv->CanInsertStack(
				invClickedBtn
				, playerClient->ItemSlotOffsetSlot
				, playerClient->CursorStack.GetItem()
				, playerClient->IsCursorStackFlipped);
		}

		// Draws slots + empty slot insert buttons
		uint16_t idx = 0;
		for (short h = 0; h < inv->Height; ++h)
		{
			for (short w = 0; w < inv->Width; ++w)
			{
				InventorySlot slot = inv->Slots[idx++];
				if ((InventorySlotState)slot.State == InventorySlotState::NOT_USED)
					continue;

				Vector2i16 curSlot = { w, h };

				// Draws border
				struct nk_rect borderRect;
				borderRect.x = invPos.x + w * SLOT_SIZE;
				borderRect.y = invPos.y + h * SLOT_SIZE;
				borderRect.w = SLOT_SIZE + BORDER_SIZE;
				borderRect.h = SLOT_SIZE + BORDER_SIZE;
				nk_layout_space_push(ctx, borderRect);
				nk_rect_lines(ctx, 0.0f, BORDER_SIZE, BORDER_COLOR);

				// Draws button and handles press
				struct nk_rect buttonRect;
				buttonRect.x = invPos.x + w * SLOT_SIZE + BORDER_SIZE;
				buttonRect.y = invPos.y + h * SLOT_SIZE + BORDER_SIZE;
				buttonRect.w = SLOT_SIZE - (BORDER_SIZE);
				buttonRect.h = SLOT_SIZE - (BORDER_SIZE);
				nk_layout_space_push(ctx, buttonRect);

				struct nk_color btnColor;
				if (!playerClient->CursorStack.IsEmpty()
					&& intersectingTiles.Contains(&curSlot))
				{
					if (canInsert)
						btnColor = nk_color{ 0, 255, 0, 255 };
					else
						btnColor = nk_color{ 255, 0, 0, 255 };
				}
				else
				{
					btnColor = INV_BG_COLOR;
				}

				if (nk_button_color(ctx, btnColor) && !playerClient->CursorStack.IsEmpty())
				{
					if (inv->InsertStack(
						curSlot
						, playerClient->ItemSlotOffsetSlot
						, &playerClient->CursorStack
						, playerClient->IsCursorStackFlipped))
					{
						playerClient->CursorStack.Remove();
						playerClient->IsCursorStackFlipped = false;
						playerClient->ItemSlotOffset = {};
						playerClient->ItemSlotOffsetSlot = {};
						playerClient->CursorStackLastPos = {};
						hasHandledInventoryActionThisFrame = true;
					}
				}
			}
		}

		// Inventory border
		nk_layout_space_push(ctx, nk_rect(invPos.x, invPos.y, width + BORDER_SIZE, height + BORDER_SIZE));
		nk_rect_lines(ctx, 0.0f, BORDER_SIZE, BORDER_COLOR);

		// Draws inventory items in inventory + take item buttons
		for (uint32_t i = 0; i < inv->Contents.Count; ++i)
		{
			InventoryStack* invStack = &inv->Contents[i];
			Item* item = invStack->Stack.GetItem();

			// Draws button and sprite. They are draw seperate because of
			// ItemStack rotation. They use different rectangle layout spaces
			struct nk_rect baseRect;
			baseRect.x = (float)invStack->Slot.x * SLOT_SIZE + BORDER_SIZE + invPos.x;
			baseRect.y = (float)invStack->Slot.y * SLOT_SIZE + BORDER_SIZE + invPos.y;
			baseRect.w = (float)item->Width * SLOT_SIZE - BORDER_SIZE;
			baseRect.h = (float)item->Height * SLOT_SIZE - BORDER_SIZE;

			struct nk_rect itemRect = GetSpriteRect(baseRect, invStack->IsRotated);
			nk_layout_space_push(ctx, itemRect);

			if (nk_button_color(ctx, { 55, 0, 0, 255 })
				&& !hasHandledInventoryActionThisFrame
				&& playerClient->CursorStack.IsEmpty())
			{
				SLOG_DEBUG("InventoryStack Pressed! (%d, %d)", invClickedBtn.x, invClickedBtn.y);

				ItemStack* slotStack = inv->GetStack(invClickedBtn);
				if (slotStack)
				{
					// This gets the offset slot on the clicked itemstack
					struct nk_vec2 btnScreenCoord = nk_layout_space_to_screen(ctx, { baseRect.x, baseRect.y });
					Vector2 btnScreenOffset = Vector2Subtract(GetMousePosition(), { btnScreenCoord.x, btnScreenCoord.y });

					Vector2i16 slotOffset;
					slotOffset.x = invClickedBtn.x - invStack->Slot.x;
					slotOffset.y = invClickedBtn.y - invStack->Slot.y;

					playerClient->CursorStack = *slotStack;
					playerClient->CursorStackLastPos = invClickedBtn;
					playerClient->ItemSlotOffset = btnScreenOffset;
					playerClient->ItemSlotOffsetSlot = slotOffset;
					playerClient->IsCursorStackFlipped = invStack->IsRotated;
					inv->RemoveStack(invClickedBtn);
				}
			}
			else // Draws item sprite if not pressed
			{
				nk_layout_space_push(ctx, baseRect);
				struct nk_sprite sprite = GetSprite(spriteSheet, item, invStack->IsRotated);
				nk_sprite(ctx, sprite);
			}
		}

		{	// Cursor Item
			struct nk_rect cursorRect;
			cursorRect.x = GetMousePosition().x - (playerClient->ItemSlotOffset.x);
			cursorRect.y = GetMousePosition().y - (playerClient->ItemSlotOffset.y);
			cursorRect.w = playerClient->CursorStack.GetItem()->Width * SLOT_SIZE;
			cursorRect.h = playerClient->CursorStack.GetItem()->Height * SLOT_SIZE;
			struct nk_rect cursorStackRect = GetSpriteRect(cursorRect, playerClient->IsCursorStackFlipped);

			struct nk_vec2 pos = nk_layout_space_to_local(ctx, { cursorRect.x, cursorRect.y });

			nk_layout_space_push(ctx, { pos.x, pos.y, cursorStackRect.w, cursorStackRect.h });
			nk_rect_fill(ctx, 0.0, INV_BG_COLOR);

			nk_layout_space_push(ctx, { pos.x, pos.y, cursorRect.w, cursorRect.h });
			struct nk_sprite sprite = GetSprite(spriteSheet, playerClient->CursorStack.GetItem(), playerClient->IsCursorStackFlipped);
			nk_sprite(ctx, sprite);
		}

		nk_layout_space_end(ctx);
	}
	nk_end(ctx);
}

internal struct nk_rect
GetSpriteRect(struct nk_rect rect, bool rotate)
{
	struct nk_rect res;
	res.x = rect.x;
	res.y = rect.y;
	if (rotate)
	{
		res.w = rect.h;
		res.h = rect.w;
	}
	else
	{
		res.w = rect.w;
		res.h = rect.h;
	}
	return res;
}

internal struct nk_sprite
GetSprite(Texture2D* texture, Item* item, bool isRotated)
{
	struct nk_sprite res = {};
	res.handle.ptr = texture;
	
	Sprite sprite = SpriteGet(item->SpriteId);
	SMemCopy(res.region, &sprite, sizeof(res.region));
	if (isRotated)
	{
		// Note: Half slot size - 1, -1 because I guessing rotation offsets slightly?
		constexpr float offset = SLOT_SIZE / 2.0f - 1.0f;
		res.origin = { offset, offset };
		res.rotation = 270.0f;
	}
	return res;
}

NK_API nk_bool
nk_button_sprite(struct nk_context* ctx, struct nk_sprite sprite)
{
	NK_ASSERT(ctx);
	if (!ctx) return 0;

	struct nk_window* win;
	struct nk_panel* layout;
	const struct nk_input* in;

	struct nk_rect bounds;
	enum nk_widget_layout_states state;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout)
		return 0;

	win = ctx->current;
	layout = win->layout;

	state = nk_widget(&bounds, ctx);
	if (!state) return 0;
	in = (state == NK_WIDGET_ROM || layout->flags & NK_WINDOW_ROM) ? 0 : &ctx->input;
	return nk_do_button_sprite(&ctx->last_widget_state, &win->buffer, bounds,
		sprite, ctx->button_behavior, &ctx->style.button, in);
}

NK_LIB nk_bool
nk_do_button_sprite(nk_flags* state,
	struct nk_command_buffer* out, struct nk_rect bounds,
	struct nk_sprite sprite, enum nk_button_behavior b,
	const struct nk_style_button* style, const struct nk_input* in)
{
	int ret;
	struct nk_rect content;

	NK_ASSERT(state);
	NK_ASSERT(style);
	NK_ASSERT(out);
	if (!out || !style || !state)
		return nk_false;

	ret = nk_do_button(state, out, bounds, style, in, b, &content);
	content.x += style->image_padding.x;
	content.y += style->image_padding.y;
	content.w -= 2 * style->image_padding.x;
	content.h -= 2 * style->image_padding.y;

	if (style->draw_begin) style->draw_begin(out, style->userdata);
	nk_draw_button(out, &bounds, *state, style);
	nk_draw_sprite(out, content, &sprite, nk_white);
	if (style->draw_end) style->draw_end(out, style->userdata);
	return ret;
}

NK_API void
nk_sprite(struct nk_context* ctx, struct nk_sprite sprite)
{
	struct nk_window* win;
	struct nk_rect bounds;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout) return;

	win = ctx->current;
	if (!nk_widget(&bounds, ctx)) return;
	nk_draw_sprite(&win->buffer, bounds, &sprite, nk_white);
}

NK_API void
nk_draw_sprite(struct nk_command_buffer* b, struct nk_rect r,
	const struct nk_sprite* sprite, struct nk_color col)
{
	struct nk_command_scal_sprite* cmd;
	NK_ASSERT(b);
	if (!b) return;
	if (b->use_clipping)
	{
		const struct nk_rect* c = &b->clip;
		if (c->w == 0 || c->h == 0 || !NK_INTERSECT(r.x, r.y, r.w, r.h, c->x, c->y, c->w, c->h))
			return;
	}

	cmd = (struct nk_command_scal_sprite*)
		nk_command_buffer_push(b, NK_COMMAND_SCAL_SPRITE, sizeof(*cmd));
	if (!cmd) return;
	cmd->x = (short)r.x;
	cmd->y = (short)r.y;
	cmd->w = (unsigned short)NK_MAX(0, r.w);
	cmd->h = (unsigned short)NK_MAX(0, r.h);
	cmd->sprite = *sprite;
	cmd->col = col;
}

NK_API void
nk_rect_lines(struct nk_context* ctx, float roundness, float border, struct nk_color color)
{
	struct nk_window* win;
	struct nk_rect bounds;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout) return;

	win = ctx->current;
	if (!nk_widget(&bounds, ctx)) return;
	nk_stroke_rect(&win->buffer, bounds, roundness, border, color);
}

NK_API void
nk_rect_fill(struct nk_context* ctx, float roundness, struct nk_color color)
{
	struct nk_window* win;
	struct nk_rect bounds;

	NK_ASSERT(ctx);
	NK_ASSERT(ctx->current);
	NK_ASSERT(ctx->current->layout);
	if (!ctx || !ctx->current || !ctx->current->layout) return;

	win = ctx->current;
	if (!nk_widget(&bounds, ctx)) return;
	nk_fill_rect(&win->buffer, bounds, roundness, color);
}