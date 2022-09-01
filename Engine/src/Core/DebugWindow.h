#pragma once

struct Font;
struct Color;

void InitiailizeDebugWindow(Font* game, float x, float y, Color color);
void UpdateDebugWindow();
void DisplayDebugText(const char* text, ...);