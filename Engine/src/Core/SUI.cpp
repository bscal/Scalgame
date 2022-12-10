/*
*
* This is a modified source from https://github.com/RobLoach/raylib-nuklear
*
Copyright (c) 2020 Rob Loach (@RobLoach)

This software is provided "as-is", without any express or implied warranty. In no event
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you
  wrote the original software. If you use this software in a product, an acknowledgment
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

#define NK_IMPLEMENTATION
#include "SUI.h"

#include "Core.h"
#include "Game.h"
#include "ResourceManager.h"
#include "SMemory.h"
#include <string>
#include <stdio.h>
#include <rlgl.h>

#define RAYLIB_NUKLEAR_DEFAULT_ARC_SEGMENTS 20

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
	auto vec = MeasureTextEx(*(Font*)handle.ptr, text, height, 0.0f);
	return vec.x;
}

bool InitializeUI(UIState* state, GameApplication* gameApp)
{
	state->App = gameApp;
	state->Font.userdata.ptr = (void*)&gameApp->Resources->FontSilver;
	state->Font.height = 16.0f;
	state->Font.width = CalculateTextWidth;
	state->Allocator.alloc = MemAlloc;
	state->Allocator.free = MemFree;

	//&outState->Allocator
	size_t size = Megabytes(1);
	void* memory = Scal::MemAlloc(size);
	nk_bool res = nk_init_fixed(&state->Ctx, memory, size, &state->Font);
	if (!res)
		TraceLog(LOG_ERROR, "Nuklear could not initialize");
	else
		TraceLog(LOG_INFO, "Ui Initialized");
	return (res == 1);
}

internal Color ColorFromNuklear(struct nk_color color)
{
	Color rc;
	rc.a = color.a;
	rc.r = color.r;
	rc.g = color.g;
	rc.b = color.b;
	return rc;
}

internal nk_color ColorToNuklear(Color color)
{
	struct nk_color rc;
	rc.a = color.a;
	rc.r = color.r;
	rc.g = color.g;
	rc.b = color.b;
	return rc;
}

internal nk_colorf ColorToNuklearF(Color color)
{
	return nk_color_cf(ColorToNuklear(color));
}

void UpdateUI(UIState* state)
{
	nk_input_begin(&state->Ctx);
	nk_input_motion(&state->Ctx, GetMouseX(), GetMouseY());
	nk_input_button(&state->Ctx, NK_BUTTON_LEFT, GetMouseX(), GetMouseY(),
		IsMouseButtonDown(MOUSE_LEFT_BUTTON));
	nk_input_button(&state->Ctx, NK_BUTTON_RIGHT, GetMouseX(), GetMouseY(),
		IsMouseButtonDown(MOUSE_RIGHT_BUTTON));
	nk_input_button(&state->Ctx, NK_BUTTON_MIDDLE, GetMouseX(), GetMouseY(),
		IsMouseButtonDown(MOUSE_MIDDLE_BUTTON));

	// Mouse Wheel
	float mouseWheel = GetMouseWheelMove();
	if (mouseWheel != 0.0f)
	{
		struct nk_vec2 mouseWheelMove;
		mouseWheelMove.x = 0.0f;
		mouseWheelMove.y = mouseWheel;
		nk_input_scroll(&state->Ctx, mouseWheelMove);
	}
	nk_input_end(&state->Ctx);

	state->IsMouseHoveringUI = IsMouseHoveringUI(state);

	if (nk_begin(&state->Ctx, "Debug", { 5, 5, 230, 200 }, NK_WINDOW_NO_SCROLLBAR))
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
		nk_label(&state->Ctx, "LOSTime: ", NK_TEXT_LEFT);
		nk_label(&state->Ctx, 
			std::to_string(GetGameApp()->LOSTime * 1000).c_str(), NK_TEXT_LEFT);
	
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


	}
	nk_end(&state->Ctx);

	//nk_colorf bg = ColorToNuklearF(SKYBLUE);

	//if (nk_begin(&state->Ctx, "Demo", { 50, 50, 230, 250 },
	//	NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
	//	NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	//{
	//	enum { EASY, HARD };
	//	static int op = EASY;
	//	static int property = 20;
	//	nk_layout_row_static(&state->Ctx, 30, 80, 1);
	//	if (nk_button_label(&state->Ctx, "button"))
	//		TraceLog(LOG_INFO, "button pressed");

	//	nk_layout_row_dynamic(&state->Ctx, 30, 2);
	//	if (nk_option_label(&state->Ctx, "easy", op == EASY)) op = EASY;
	//	if (nk_option_label(&state->Ctx, "hard", op == HARD)) op = HARD;

	//	nk_layout_row_dynamic(&state->Ctx, 25, 1);
	//	nk_property_int(&state->Ctx, "Compression:", 0, &property, 100, 10, 1);

	//	nk_layout_row_dynamic(&state->Ctx, 20, 1);
	//	nk_label(&state->Ctx, "background:", NK_TEXT_LEFT);
	//	nk_layout_row_dynamic(&state->Ctx, 25, 1);
	//	if (nk_combo_begin_color(&state->Ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(&state->Ctx), 400)))
	//	{
	//		nk_layout_row_dynamic(&state->Ctx, 120, 1);
	//		bg = nk_color_picker(&state->Ctx, bg, NK_RGBA);
	//		nk_layout_row_dynamic(&state->Ctx, 25, 1);
	//		bg.r = nk_propertyf(&state->Ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
	//		bg.g = nk_propertyf(&state->Ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
	//		bg.b = nk_propertyf(&state->Ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
	//		bg.a = nk_propertyf(&state->Ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
	//		nk_combo_end(&state->Ctx);
	//	}
	//}

	//nk_end(&state->Ctx);
}

void RenderUI(UIState* state)
{
	const struct nk_command* cmd;

	nk_foreach(cmd, &state->Ctx)
	{
		switch (cmd->type)
		{
			case NK_COMMAND_NOP: {
				break;
			}

			case NK_COMMAND_SCISSOR: {
				// TODO(RobLoach): Verify if NK_COMMAND_SCISSOR works.
				const struct nk_command_scissor* s = (const struct nk_command_scissor*)cmd;
				BeginScissorMode(s->x, s->y, s->w, s->h);
			} break;

			case NK_COMMAND_LINE: {
				const struct nk_command_line* l = (const struct nk_command_line*)cmd;
				Color color = ColorFromNuklear(l->color);
				Vector2 startPos = { (float)l->begin.x, (float)l->begin.y };
				Vector2 endPos = { (float)l->end.x, (float)l->end.y };
				DrawLineEx(startPos, endPos, l->line_thickness, color);
			} break;

			case NK_COMMAND_CURVE: {
				const struct nk_command_curve* q = (const struct nk_command_curve*)cmd;
				Color color = ColorFromNuklear(q->color);
				Vector2 start = { (float)q->begin.x, (float)q->begin.y };
				// Vector2 controlPoint1 = (Vector2){q->ctrl[0].x, q->ctrl[0].y};
				// Vector2 controlPoint2 = (Vector2){q->ctrl[1].x, q->ctrl[1].y};
				Vector2 end = { (float)q->end.x, (float)q->end.y };
				// TODO: Encorporate segmented control point bezier curve?
				// DrawLineBezier(start, controlPoint1, (float)q->line_thickness, color);
				// DrawLineBezier(controlPoint1, controlPoint2, (float)q->line_thickness, color);
				// DrawLineBezier(controlPoint2, end, (float)q->line_thickness, color);
				DrawLineBezier(start, end, (float)q->line_thickness, color);
			} break;

			case NK_COMMAND_RECT: {
				const struct nk_command_rect* r = (const struct nk_command_rect*)cmd;
				Color color = ColorFromNuklear(r->color);
				Rectangle rect = { (float)r->x, (float)r->y, (float)r->w, (float)r->h };
				if (r->rounding > 0)
				{
					float roundness = (float)r->rounding * 4.0f / (rect.width + rect.height);
					DrawRectangleRoundedLines(rect, roundness, 1, r->line_thickness, color);
				}
				else
				{
					DrawRectangleLinesEx(rect, r->line_thickness, color);
				}
			} break;

			case NK_COMMAND_RECT_FILLED: {
				const struct nk_command_rect_filled* r = (const struct nk_command_rect_filled*)cmd;
				Color color = ColorFromNuklear(r->color);
				Rectangle rect = { (float)r->x, (float)r->y, (float)r->w, (float)r->h };
				if (r->rounding > 0)
				{
					float roundness = (float)r->rounding * 4.0f / (rect.width + rect.height);
					DrawRectangleRounded(rect, roundness, 1, color);
				}
				else
				{
					DrawRectangleRec(rect, color);
				}
			} break;

			case NK_COMMAND_RECT_MULTI_COLOR: {
				const struct nk_command_rect_multi_color* rectangle = (const struct nk_command_rect_multi_color*)cmd;
				Rectangle position = { (float)rectangle->x, (float)rectangle->y, (float)rectangle->w, (float)rectangle->h };
				Color left = ColorFromNuklear(rectangle->left);
				Color top = ColorFromNuklear(rectangle->top);
				Color bottom = ColorFromNuklear(rectangle->bottom);
				Color right = ColorFromNuklear(rectangle->right);
				DrawRectangleGradientEx(position, left, bottom, right, top);
			} break;

			case NK_COMMAND_CIRCLE: {
				const struct nk_command_circle* c = (const struct nk_command_circle*)cmd;
				Color color = ColorFromNuklear(c->color);
				DrawEllipseLines(c->x + c->w / 2, c->y + c->h / 2, c->w / 2, c->h / 2, color);
			} break;

			case NK_COMMAND_CIRCLE_FILLED: {
				const struct nk_command_circle_filled* c = (const struct nk_command_circle_filled*)cmd;
				Color color = ColorFromNuklear(c->color);
				DrawEllipse(c->x + c->w / 2, c->y + c->h / 2, c->w / 2, c->h / 2, color);
			} break;

			case NK_COMMAND_ARC: {
				const struct nk_command_arc* a = (const struct nk_command_arc*)cmd;
				Color color = ColorFromNuklear(a->color);
				Vector2 center = { (float)a->cx, (float)a->cy };
				DrawRingLines(center, 0, a->r, a->a[0] * RAD2DEG - 45, a->a[1] * RAD2DEG - 45, RAYLIB_NUKLEAR_DEFAULT_ARC_SEGMENTS, color);
			} break;

			case NK_COMMAND_ARC_FILLED: {
				const struct nk_command_arc_filled* a = (const struct nk_command_arc_filled*)cmd;
				Color color = ColorFromNuklear(a->color);
				Vector2 center = { (float)a->cx, (float)a->cy };
				DrawRing(center, 0, a->r, a->a[0] * RAD2DEG - 45, a->a[1] * RAD2DEG - 45, RAYLIB_NUKLEAR_DEFAULT_ARC_SEGMENTS, color);
			} break;

			case NK_COMMAND_TRIANGLE: {
				const struct nk_command_triangle* t = (const struct nk_command_triangle*)cmd;
				Color color = ColorFromNuklear(t->color);
				Vector2 point1 = { (float)t->b.x, (float)t->b.y };
				Vector2 point2 = { (float)t->a.x, (float)t->a.y };
				Vector2 point3 = { (float)t->c.x, (float)t->c.y };
				DrawTriangleLines(point1, point2, point3, color);
			} break;

			case NK_COMMAND_TRIANGLE_FILLED: {
				const struct nk_command_triangle_filled* t = (const struct nk_command_triangle_filled*)cmd;
				Color color = ColorFromNuklear(t->color);
				Vector2 point1 = { (float)t->b.x, (float)t->b.y };
				Vector2 point2 = { (float)t->a.x, (float)t->a.y };
				Vector2 point3 = { (float)t->c.x, (float)t->c.y };
				DrawTriangle(point1, point2, point3, color);
			} break;

			case NK_COMMAND_POLYGON: {
				// TODO: Confirm Polygon
				const struct nk_command_polygon* p = (const struct nk_command_polygon*)cmd;
				Color color = ColorFromNuklear(p->color);
				Vector2* points = (Vector2*)(p->point_count * (unsigned short)sizeof(Vector2));
				unsigned short i;
				for (i = 0; i < p->point_count; i++)
				{
					points[i].x = p->points[i].x;
					points[i].y = p->points[i].y;
				}
				DrawTriangleStrip(points, p->point_count, color);
				MemFree(points);
			} break;

			case NK_COMMAND_POLYGON_FILLED: {
				// TODO: Polygon filled expects counter clockwise order
				const struct nk_command_polygon_filled* p = (const struct nk_command_polygon_filled*)cmd;
				Color color = ColorFromNuklear(p->color);
				Vector2* points = (Vector2*)(p->point_count * (unsigned short)sizeof(Vector2));
				unsigned short i;
				for (i = 0; i < p->point_count; i++)
				{
					points[i].x = p->points[i].x;
					points[i].y = p->points[i].y;
				}
				DrawTriangleFan(points, p->point_count, color);
				MemFree(points);
			} break;

			case NK_COMMAND_POLYLINE: {
				// TODO: Polygon expects counter clockwise order
				const struct nk_command_polyline* p = (const struct nk_command_polyline*)cmd;
				Color color = ColorFromNuklear(p->color);
				Vector2* points = (Vector2*)MemAlloc(p->point_count * (unsigned short)sizeof(Vector2));
				unsigned short i;
				for (i = 0; i < p->point_count; i++)
				{
					points[i].x = p->points[i].x;
					points[i].y = p->points[i].y;
				}
				DrawTriangleStrip(points, p->point_count, color);
				MemFree(points);
			} break;

			case NK_COMMAND_TEXT: {
				const struct nk_command_text* text = (const struct nk_command_text*)cmd;
				Color color = ColorFromNuklear(text->foreground);
				Color background = ColorFromNuklear(text->background);
				float fontSize = text->font->height;
				Font* font = (Font*)text->font->userdata.ptr;
				DrawRectangle(text->x, text->y, text->w, text->h, background);
				if (font != NULL)
				{
					Vector2 position = { (float)text->x, (float)text->y };
					DrawTextEx(*font, (const char*)text->string, position, fontSize, fontSize / 10.0f, color);
				}
				else
				{
					DrawText((const char*)text->string, text->x, text->y, (int)fontSize, color);
				}
			} break;

			case NK_COMMAND_IMAGE: {
				const struct nk_command_image* i = (const struct nk_command_image*)cmd;
				Texture texture = *(Texture*)i->img.handle.ptr;
				Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
				Rectangle dest = { (float)i->x, (float)i->y, (float)i->w, (float)i->h };
				Vector2 origin = { 0, 0 };
				Color tint = ColorFromNuklear(i->col);
				DrawTexturePro(texture, source, dest, origin, 0, tint);
			} break;

			case NK_COMMAND_CUSTOM: {
				TraceLog(LOG_WARNING, "NUKLEAR: Unverified custom callback implementation NK_COMMAND_CUSTOM");
				const struct nk_command_custom* custom = (const struct nk_command_custom*)cmd;
				custom->callback(NULL, custom->x, custom->y, custom->w, custom->h, custom->callback_data);
			} break;

			default: {
				TraceLog(LOG_WARNING, "NUKLEAR: Missing implementation %i", cmd->type);
			} break;
		}
	}

	nk_clear(&state->Ctx);
}

bool IsMouseHoveringUI(UIState* state)
{
	return nk_window_is_any_hovered(&state->Ctx) != 0;
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
