#pragma once

#include "Core.h"

#include "rlgl.h"

void DrawTextureProF(Texture2D texture, Rectangle source, Rectangle dest,
    Vector2 origin, float rotation, Vector4 tint);

void ScalDrawTextureProF(const Texture2D* texture,
    Rectangle source, Rectangle dest, Vector4 tint);

void SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color);

RenderTexture2D SLoadRenderTexture(int width, int height, PixelFormat format);
