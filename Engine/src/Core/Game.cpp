#include "Game.h"

#include "ResourceManager.h"
#include "SMemory.h"
#include "raymath.h"
#include "SUI.h"
#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "SUtil.h"
#include "Creature.h"
#include "SpriteAtlas.h"

global_var GameApplication* GameAppPtr;

SAPI bool GameApplication::Start()
{
	if (IsInitialized)
	{
		TraceLog(LOG_ERROR, "GameApplication already initialized!");
		return false;
	}

	const int screenWidth = 1600;
	const int screenHeight = 900;
	InitWindow(screenWidth, screenHeight, "Some roguelike game");
	SetTargetFPS(60);
	SetTraceLogLevel(LOG_ALL);
	GameAppPtr = this;

	Resources = (struct Resources*)Scal::MemAllocZero(sizeof(struct Resources));
	InitializeResources(Resources);

	UIState = (struct UIState*)Scal::MemAllocZero(sizeof(struct UIState));
	InitializeUI(&Resources->FontSilver, 16.0f, UIState);
	InitiailizeDebugWindow(&Resources->MainFontM, 10, 30, DARKGREEN);

	Game = (struct Game*)Scal::MemAllocZero(sizeof(struct Game));
	if (!InitializeGame(Game, this))
	{
		S_CRASH("Game failed to initialized!");
		return false;
	}
	Game->Atlas.Load("assets/textures/atlas/tiles.atlas", 32);

	Scal::Creature::TestCreature(Game);
	Test();
	TestSTable();
	TestHashes();
	return IsInitialized = true;
}

SAPI void GameApplication::Shutdown()
{
	GameAppPtr = nullptr;
	CloseWindow();
}

internal void TestActionFunc(World* world, Action* action)
{
	TraceLog(LOG_INFO, "Testing!");
}

SAPI void GameApplication::Run()
{
	TraceLog(LOG_INFO, "Game Running...");
	IsRunning = true;

	while (!WindowShouldClose())
	{
		DeltaTime = GetFrameTime();

		if (IsSuspended) continue;

		// ***************
		// Update
		// ***************

		if (!UIState->IsMouseHoveringUI)
		{
			// Free Camera Controls
			if (IsKeyPressed(KEY_SLASH))
				Game->IsFreeCam = !Game->IsFreeCam;

			if (Game->IsFreeCam)
			{
				if (IsKeyDown(KEY_L))
					SetCameraPosition(Game,
						{ 512.0f * DeltaTime, 0, 0 });
				else if (IsKeyDown(KEY_J))
					SetCameraPosition(Game,
						{ -512.0f * DeltaTime, 0, 0 });
				if (IsKeyDown(KEY_K))
					SetCameraPosition(Game,
						{ 0, -512.0f * DeltaTime, 0 });
				else if (IsKeyDown(KEY_I))
					SetCameraPosition(Game,
						{ 0, 512.0f * DeltaTime, 0 });
			}

			// Camera zoom controls
			float mouseWheel = GetMouseWheelMove();
			if (mouseWheel != 0)
			{
				SetCameraDistance(Game, mouseWheel);
			}

			// Debug place tiles
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				// LocalToWorld
				Matrix invMatCamera =
					MatrixInvert(GetCameraMatrix2D(Game->Camera));
				Vector3 transform = Vector3Transform(
					{ (float)GetMouseX(), (float)GetMouseY(), 0.0f }, invMatCamera);

				int x = transform.x / 16;
				int y = transform.y / 16;
				if (IsInBounds(x, y, 64, 64))
				{
					//Tile tile = CreateTile(&Game->World.MainTileMap, 4);
					//SetTile(&Game->World.MainTileMap, x, y, &tile);
				}
			}

			if (IsKeyPressed(KEY_ONE))
			{
				Action testAction = {};
				testAction.Cost = 100;
				testAction.ActionFunction = TestActionFunc;
				AddAction(&Game->World, &testAction);
			}
		}

		UpdateUI(UIState);
		//Scal::ShowMemoryUsage(UIState);

		// ***************
		// Render
		// ***************
		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(Game->Camera);

		//BeginMode3D(Game->Camera3D);

		WorldUpdate(&Game->World, this);

		//EndMode3D();

		EndMode2D();

		// ***************
		// UI
		// ***************
		UpdateDebugWindow();
		//BeginShaderMode(Resources.SDFFont.Shader);

		//DisplayDebugText("Zoom = %.2f", Game->Camera.zoom);
		//DisplayDebugText("cX = %.1f, cY = %.1f",
		//    Game->Camera.target.x, Game->Camera.target.y);
		//const Scal::Creature::Player* p = Game->World.EntityMgr.GetPlayer(0);
		//DisplayDebugText("pX = %d, pY = %d",
		//    p->Transform.TilePos.x, p->Transform.TilePos.y);
		//DisplayDebugText("Time = %d", Game->Time);
		//DisplayDebugText("Energy = %d/%d", Game->Player.Energy, Game->Player.MaxEnergy);
		//EndShaderMode();

		RenderUI(UIState);

		const double drawStart = GetTime();
		EndDrawing();
		RenderTime = GetTime() - drawStart;

		// Handle Camera Move
		if (!Game->IsFreeCam)
		{
			Vector2 from = Game->Camera.target;
			Vector2 to = GetClientPlayer()->Transform.PosAsVec2();
			float t;
			if (Vector2Distance(from, to) < 2.5f)
				t = 10.0f * GetDeltaTime();
			else
				t = 4.0f * GetDeltaTime();
			Vector2 cameraPos = Vector2Lerp(from, to, t);
			Game->Camera.target = cameraPos;
			Game->Camera.offset = { GetScreenWidth() / 2.0f,
				GetScreenHeight() / 2.0f };
		}
	}
	IsRunning = false;
}


GameApplication* const GetGameApp()
{
	assert(GameAppPtr);
	return GameAppPtr;
}

Scal::Creature::Player* GetClientPlayer()
{
	assert(GameAppPtr);
	return &GameAppPtr->Game->World.EntityMgr.Players[0];
}

void SetCameraPosition(Game* game, Vector3 pos)
{
	#if Mode3D // 3D
	Camera3D& camera = game->Camera;
	camera->target.x = floorf(camera->target.x + pos.x);
	camera->target.y = floorf(camera->target.y + pos.y);
	camera->target.z = floorf(camera->target.z + pos.z);
	camera->position.x = floorf(camera->position.x + pos.x);
	camera->position.y = floorf(camera->position.y + pos.y);
	#else
	Camera2D& camera = game->Camera;
	camera.target.x += pos.x;
	camera.target.y += pos.y;
	#endif
}

void SetCameraDistance(Game* game, float zoom)
{
	#if Mode3D // 3D
	Camera3D& camera = game->Camera;
	zoom = camera->position.z + zoom * 15.0f;
	if (zoom > 256.0f) zoom = 250.0f;
	if (zoom < 16.0f) zoom = 15.0f;
	camera->position.z = floorf(zoom);
	#else // 2D
	Camera2D& camera2d = game->Camera;
	camera2d.zoom += zoom * 0.15f;
	if (camera2d.zoom > 3.0f)
		camera2d.zoom = 3.0f;
	else if (camera2d.zoom < 0.2f)
		camera2d.zoom = 0.2f;
	#endif
}

internal void HandleInput(GameApplication* gameApp) {}

inline float GetDeltaTime()
{
	return GetFrameTime();
}

//void UpdateTime(GameApplication* gameApp, int timeChange)
//{
//    gameApp->Game->Time += timeChange;
//    gameApp->Game->Player.Energy = gameApp->Game->Player.MaxEnergy;
//
//    ProcessActions(&gameApp->Game->World);
//    gameApp->Game->World.EntityActionsList.Clear();
//}

internal bool InitializeGame(Game* game, GameApplication* gameApp)
{
	CALL_CONSTRUCTOR(game) Game();

	WorldInitialize(&game->World, &gameApp->Resources->MainTileSet);

	#if Mode3D
	Camera3D camera = {};
	camera.position = { 0.0f, 0.0f, 100.0f };   // Camera position
	camera.target = { 0.0f, 0.0f, 0.0f };        // Camera looking at point
	camera.up = { 0.0f, 1.0f, 0.0f };            // Camera up vector (rotation towards target)
	camera.fovy = 90.0f;                         // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;      // Camera mode type
	game->Camera3D = camera;
	SetCameraMode(game->Camera3D,
		game->Camera3D.projection);
	#else
	game->Camera.zoom = 2.0f;
	#endif
	S_LOG_INFO("Game Initialized!");
	return true;
}
