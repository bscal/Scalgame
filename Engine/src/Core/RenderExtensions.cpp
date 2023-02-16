#include "RenderExtensions.h"

#include "raymath.h"

void DrawTextureProF(Texture2D texture, Rectangle source, Rectangle dest,
    Vector2 origin, float rotation, Vector4 tint)
{
    // Check if texture is valid
    if (texture.id > 0)
    {
        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (source.width < 0) { flipX = true; source.width *= -1; }
        if (source.height < 0) source.y -= source.height;

        Vector2 topLeft = { 0 };
        Vector2 topRight = { 0 };
        Vector2 bottomLeft = { 0 };
        Vector2 bottomRight = { 0 };

        // Only calculate rotation if needed
        if (rotation == 0.0f)
        {
            float x = dest.x - origin.x;
            float y = dest.y - origin.y;
            topLeft = Vector2{ x, y };
            topRight = Vector2{ x + dest.width, y };
            bottomLeft = Vector2{ x, y + dest.height };
            bottomRight = Vector2{ x + dest.width, y + dest.height };
        }
        else
        {
            float sinRotation = sinf(rotation * DEG2RAD);
            float cosRotation = cosf(rotation * DEG2RAD);
            float x = dest.x;
            float y = dest.y;
            float dx = -origin.x;
            float dy = -origin.y;

            topLeft.x = x + dx * cosRotation - dy * sinRotation;
            topLeft.y = y + dx * sinRotation + dy * cosRotation;

            topRight.x = x + (dx + dest.width) * cosRotation - dy * sinRotation;
            topRight.y = y + (dx + dest.width) * sinRotation + dy * cosRotation;

            bottomLeft.x = x + dx * cosRotation - (dy + dest.height) * sinRotation;
            bottomLeft.y = y + dx * sinRotation + (dy + dest.height) * cosRotation;

            bottomRight.x = x + (dx + dest.width) * cosRotation - (dy + dest.height) * sinRotation;
            bottomRight.y = y + (dx + dest.width) * sinRotation + (dy + dest.height) * cosRotation;
        }

        rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

        rlSetTexture(texture.id);
        rlBegin(RL_QUADS);

        rlColor4f(tint.x, tint.y, tint.z, tint.w);
        rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

        // Top-left corner for texture and quad
        if (flipX) rlTexCoord2f((source.x + source.width) / width, source.y / height);
        else rlTexCoord2f(source.x / width, source.y / height);
        rlVertex2f(topLeft.x, topLeft.y);

        // Bottom-left corner for texture and quad
        if (flipX) rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
        else rlTexCoord2f(source.x / width, (source.y + source.height) / height);
        rlVertex2f(bottomLeft.x, bottomLeft.y);

        // Bottom-right corner for texture and quad
        if (flipX) rlTexCoord2f(source.x / width, (source.y + source.height) / height);
        else rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
        rlVertex2f(bottomRight.x, bottomRight.y);

        // Top-right corner for texture and quad
        if (flipX) rlTexCoord2f(source.x / width, source.y / height);
        else rlTexCoord2f((source.x + source.width) / width, source.y / height);
        rlVertex2f(topRight.x, topRight.y);

        rlEnd();
        rlSetTexture(0);
    }
}

void ScalDrawTextureProF(const Texture2D* texture,
    Rectangle source, Rectangle dest, Vector4 tint)
{
    SASSERT(texture->id > 0);

    float width = (float)texture->width;
    float height = (float)texture->height;

    Vector2 topLeft = Vector2{ dest.x, dest.y };
    Vector2 topRight = Vector2{ dest.x + dest.width, dest.y };
    Vector2 bottomLeft = Vector2{ dest.x, dest.y + dest.height };
    Vector2 bottomRight = Vector2{ dest.x + dest.width, dest.y + dest.height };

    rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

    rlSetTexture(texture->id);
    rlBegin(RL_QUADS);

    rlColor4f(tint.x, tint.y, tint.z, tint.w);
    rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

    rlTexCoord2f(source.x / width, source.y / height);
    rlVertex2f(topLeft.x, topLeft.y);

    rlTexCoord2f(source.x / width, (source.y + source.height) / height);
    rlVertex2f(bottomLeft.x, bottomLeft.y);

    rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
    rlVertex2f(bottomRight.x, bottomRight.y);

    rlTexCoord2f((source.x + source.width) / width, source.y / height);
    rlVertex2f(topRight.x, topRight.y);

    rlEnd();
    rlSetTexture(0);
}

// Draw a color-filled rectangle with pro parameters
void SDrawRectangleProF(Rectangle rec, Vector2 origin, float rotation, Vector4 color)
{
    Vector2 topLeft = { 0 };
    Vector2 topRight = { 0 };
    Vector2 bottomLeft = { 0 };
    Vector2 bottomRight = { 0 };

    float x = rec.x - origin.x;
    float y = rec.y - origin.y;
    topLeft = (Vector2){ x, y };
    topRight = (Vector2){ x + rec.width, y };
    bottomLeft = (Vector2){ x, y + rec.height };
    bottomRight = (Vector2){ x + rec.width, y + rec.height };

    rlBegin(RL_TRIANGLES);

    rlColor4f(color.x, color.y, color.z, color.w);

    rlVertex2f(topLeft.x, topLeft.y);
    rlVertex2f(bottomLeft.x, bottomLeft.y);
    rlVertex2f(topRight.x, topRight.y);

    rlVertex2f(topRight.x, topRight.y);
    rlVertex2f(bottomLeft.x, bottomLeft.y);
    rlVertex2f(bottomRight.x, bottomRight.y);

    rlEnd();
}

RenderTexture2D SLoadRenderTexture(int width, int height, PixelFormat format)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(width, height);   // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, format, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = format;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}