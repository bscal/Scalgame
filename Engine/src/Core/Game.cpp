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
#include "rlgl.h"

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
	InitializeUI(UIState, this);
	InitiailizeDebugWindow(&Resources->MainFontM, 10, 30, DARKGREEN);

	Game = (struct Game*)Scal::MemAllocZero(sizeof(struct Game));
	if (!InitializeGame(Game, this))
	{
		S_CRASH("Game failed to initialized!");
		return false;
	}
	Game->Atlas.Load("assets/textures/atlas/tiles.atlas", 32);

	Test();
	TestSTable();

	IsInitialized = true;
	return IsInitialized;
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

				//int x = (int)transform.x / 16;
				//int y = (int)transform.y / 16;
				//if (IsInBounds(x, y, 64, 64))
				//{
				//	//Tile tile = CreateTile(&Game->World.MainTileMap, 4);
				//	//SetTile(&Game->World.MainTileMap, x, y, &tile);
				//}
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

		// ***************
		// Render
		// ***************
		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode2D(Game->Camera);
		BeginShaderMode(Resources->TileShader);

		//BeginMode3D(Game->Camera3D);

		WorldUpdate(&Game->World, Game);
		LateWorldUpdate(&Game->World, Game);

		//EndMode3D();
		EndShaderMode();

		BeginShaderMode(Resources->LightRayShader);
		
		//auto id = rlGetLocationUniform(Resources->LightRayShader.id,
			//"Light");
		auto v = GetScreenToWorld2D(GetMousePosition(), Game->Camera);
		Vector3 light = { v.x, v.y, 5.0f };
		SetShaderValue(Resources->LightRayShader,
			0, &light, SHADER_UNIFORM_VEC3);

		DrawRectangle(0, 0, 500, 500, WHITE);
		
		EndShaderMode();

		EndMode2D();

		// ***************
		// UI
		// ***************
		UpdateDebugWindow();
		//BeginShaderMode(Resources.SDFFont.Shader);

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
	assert(GameAppPtr->IsInitialized);
	return GameAppPtr;
}

Scal::Creature::Player* GetClientPlayer()
{
	return &GetGameApp()->Game->World.EntityMgr.Players[0];
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

float GetDeltaTime()
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

	WorldInitialize(&game->World, gameApp);

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
