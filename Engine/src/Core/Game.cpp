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
#include "LightSource.h"

global_var GameApplication* GameAppPtr;

// TODO
global_var int Scale = -1;
global_var int Size = -1;
global_var int Tiles = -1;

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
	Scale = GetShaderLocation(Resources->LightShader, "Scale");
	Size = GetShaderLocation(Resources->LightShader, "Size");
	Tiles = GetShaderLocation(Resources->LightShader, "Tiles");


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
	Game->Atlas.Unload();
	UnloadFont(Resources->FontSilver);
	UnloadFont(Resources->MainFontM);
	UnloadTexture(Resources->EntitySpriteSheet);
	UnloadRenderTexture(Game->World.LightMap.Texture);
	UnloadRenderTexture(Game->ScreenTexture);
	UnloadShader(Resources->LightShader);
	UnloadShader(Resources->TileShader);
	UnloadShader(Resources->LightRayShader);
	CloseWindow();
}

internal void TestActionFunc(World* world, Action* action)
{
	TraceLog(LOG_INFO, "Testing!");
}

SAPI void GameApplication::Run()
{
	// TODO remove
	WorldLoad(&Game->World, Game);

	
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
						{ 0, 512.0f * DeltaTime, 0 });
				else if (IsKeyDown(KEY_I))
					SetCameraPosition(Game,
						{ 0, -512.0f * DeltaTime, 0 });
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
		UpdateGame(Game, this);
		WorldUpdate(&Game->World, Game);
		LateWorldUpdate(&Game->World, Game);
		
		//EndMode3D();
		EndShaderMode();
		EndMode2D();

		Game->World.LightMap.BuildLightMap();
		Game->World.LightMap.UpdatePositions({});

		BeginMode2D(Game->Camera);
		auto shader = Resources->LightShader;
		BeginShaderMode(shader);
		
		//int Light = GetShaderLocation(shader, "Light");
		//int TileCount = GetShaderLocation(shader, "TileCount");
		//int TilesWidth = GetShaderLocation(shader, "TilesWidth");
		//auto v = GetScreenToWorld2D(GetMousePosition(), Game->Camera);
		//Vector3 light = { v.x, v.y, 5.0f };
		//SetShaderValue(Resources->LightRayShader,
			//Light, &light, SHADER_UNIFORM_VEC3);
		Vector2 scale =
		{
			 Game->Camera.zoom,
			 Game->Camera.zoom
		};
		SetShaderValue(shader, Scale, &scale, SHADER_UNIFORM_VEC2);
		Vector2 size =
		{
			 (float)GetScreenWidth(),
			 (float)GetScreenHeight()
		};
		SetShaderValue(shader, Size, &size, SHADER_UNIFORM_VEC2);
		//int size = lm.Solids.size();
		//SetShaderValue(shader, TileCount, &size, SHADER_UNIFORM_INT);
		//int width = lm.EndPos.x;
		//SetShaderValue(shader, TilesWidth, &width, SHADER_UNIFORM_INT);
		//GenTextureMipmaps(&text.texture
		SetShaderValueTexture(shader, Tiles, Game->World.LightMap.Texture.texture);
		//DrawTexture(text.texture, lm.StartPos.x * 16, lm.StartPos.y * 16, WHITE);

		Rectangle r = Game->CurScreenRect;
		r.x = 0.0f;
		r.y = 0.0f;
		r.width = Game->World.LightMap.Width * 16.0f;
		r.height = Game->World.LightMap.Height * 16.0f;
		auto v = GetClientPlayer()->Transform.PosAsVec2();

		DrawRectanglePro(r, {}, 0.0f, {});
		EndShaderMode();
		DrawRectangleLinesEx(r, 4, PINK);
		EndMode2D();

		// ***************
		// UI
		// ***************
		//UpdateDebugWindow();
		//BeginShaderMode(Resources.SDFFont.Shader);

		RenderUI(UIState);

		const double drawStart = GetTime();
		EndDrawing();
		RenderTime = GetTime() - drawStart;
		//UnloadRenderTexture(text);

		
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
	camera2d.zoom += zoom * 0.25f;
	if (camera2d.zoom > 5.0)
		camera2d.zoom = 5.0;
	else if (camera2d.zoom < 0.25f)
		camera2d.zoom = 0.25f;
	#endif
}

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

internal void UpdateGame(Game* game, GameApplication* gameApp)
{
	Camera2D& camera = game->Camera;

	// Handle Camera Move
	if (!game->IsFreeCam)
	{
		Vector2 from = camera.target;
		Vector2 to = GetClientPlayer()->Transform.PosAsVec2();
		float t;
		if (Vector2Distance(from, to) < 2.5f)
			t = 10.0f * GetDeltaTime();
		else
			t = 4.0f * GetDeltaTime();
		Vector2 cameraPos = Vector2Lerp(from, to, t);
		camera.target.x = floorf(cameraPos.x);
		camera.target.y = floorf(cameraPos.y);
		camera.offset.x = floorf((float)GetScreenWidth() / 2.0f);
		camera.offset.y = floorf((float)GetScreenHeight() / 2.0f);
	}
	
	/*game->CurScreenRect.x = floorf(
		camera.target.x - camera.offset.x / camera.zoom);
	game->CurScreenRect.y = floorf(
		camera.target.y - camera.offset.y / camera.zoom);
	game->CurScreenRect.width = roundf((float)GetScreenWidth() / camera.zoom);
	game->CurScreenRect.height = roundf((float)GetScreenHeight() / camera.zoom);*/
	auto v = GetScreenToWorld2D({ 0,0 }, game->Camera);
	game->CurScreenRect.x = floorf(v.x);
	game->CurScreenRect.y = floorf(v.y);
	auto v2 = GetScreenToWorld2D(
		{ (float)GetScreenWidth(),(float)GetScreenHeight() },
		game->Camera);
	game->CurScreenRect.width = floorf(v2.x - v.x);
	game->CurScreenRect.height = floorf(v2.y - v.y);
}

internal bool InitializeGame(Game* game, GameApplication* gameApp)
{
	CALL_CONSTRUCTOR(game) Game();

	game->ScreenTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

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
	game->Camera.zoom = 1.0;
	#endif
	S_LOG_INFO("Game Initialized!");
	return true;
}
