#pragma once

#include <cstdint>
#include "src/world/World.h"

class GameClient
{
public:
	uint32_t ScreenWidth, ScreenHeight;
	Font MainGameFont; // TODO new class?
	Texture2D Tilemap; // TODO new class?
	World World;
	Camera2D MainCamera;
public:
	GameClient();
	int Start();

private:
	void Init();
	void SetupGame();
	void Loop();
	void Cleanup();

	void Render();
	void RenderUI();
	void Update();
	void SetupCamera();

};
