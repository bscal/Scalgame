#pragma once

#include "Core.h"

struct Game;

struct Light
{
    Vector2 Pos;
    Color Color;
    float Intensity;
};

void InitLights();
void AddLight(const Light& light);

void LightingUpdate(Game* game);
