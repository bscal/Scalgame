#include "DebugWindow.h"

#include "Game.h"

#include <cassert>
#include <cstring>
#include <cstdio>

struct DebugWindowState
{
    Color Color;
    Font* Font;
    Vector2 StartPos;
    Vector2 CurrentDrawPos;
    float YStep;
    bool ShouldShow;
    bool IsInitialized;
};

global_var DebugWindowState WindowState;

void InitiailizeDebugWindow(Font* font, float x, float y, Color color)
{
    assert(!WindowState.IsInitialized);
    WindowState.Color = color;
    WindowState.Font = font;
    WindowState.StartPos = { x, y };
    WindowState.CurrentDrawPos = WindowState.StartPos;
    WindowState.YStep = 20.0f;
    WindowState.ShouldShow = true;
    WindowState.IsInitialized = true;
}

void UpdateDebugWindow()
{
    WindowState.CurrentDrawPos = WindowState.StartPos;

    if (IsKeyPressed(KEY_COMMA))
        WindowState.ShouldShow = !WindowState.ShouldShow;

    if (WindowState.ShouldShow)
        DrawFPS(10, 10);
}

void DisplayDebugText(const char* text, ...)
{
    assert(WindowState.IsInitialized);

    if (!WindowState.ShouldShow)
        return;

    char buffer[64] = { 0 };

    va_list argPtr;
    va_start(argPtr, text);
    vsnprintf(buffer, sizeof(buffer), text, argPtr);
    va_end(argPtr);

    //float fontSize = WindowState.Font->baseSize * 4.0f;
    DrawTextEx(*WindowState.Font, buffer, WindowState.CurrentDrawPos, 16.0f, 0.0f, WindowState.Color);
    WindowState.CurrentDrawPos.y += WindowState.YStep;
}
