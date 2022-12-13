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
	//SetTargetFPS(60);
	SetTraceLogLevel(LOG_ALL);

	GameAppPtr = this;

	Resources = (struct Resources*)Scal::MemAllocZero(sizeof(struct Resources));
	InitializeResources(Resources);
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
	UnloadRenderTexture(Game->ScreenLightMapTexture);
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
				//SetCameraDistance(Game, mouseWheel);
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
		BeginTextureMode(Game->ScreenTexture);
			ClearBackground(BLACK);
			BeginShaderMode(Resources->TileShader);
				UpdateGame(Game, this);
				WorldUpdate(&Game->World, Game);
				LateWorldUpdate(&Game->World, Game);
			EndShaderMode();
		EndTextureMode();

		Game->World.LightMap.BuildLightMap();
		Game->World.LightMap.UpdatePositions({});

		const auto& shader = Resources->LightShader;
		BeginTextureMode(Game->ScreenLightMapTexture);
			BeginShaderMode(shader);
				Vector2 size =
				{
					(float)GetScreenWidth(),
					(float)GetScreenHeight()
				};
				SetShaderValue(shader, Size, &size, SHADER_UNIFORM_VEC2);
				SetShaderValueTexture(shader, Tiles,
					Game->World.LightMap.Texture.texture);

				Rectangle rectSize
				{
					0.0f,
					0.0f,
					(float)GetScreenWidth(),
					(float)GetScreenHeight()
				};
				DrawRectanglePro(rectSize, {}, 0.0f, WHITE);
			EndShaderMode();
		EndTextureMode();

		BeginDrawing();

		BeginMode2D(Game->Camera);
		ClearBackground(BLACK);

		rlPushMatrix();
		rlScalef(GetScale(), GetScale(), 1.0f);

		Rectangle screenRect
		{
			0.0f,
			0.0f,
			(float)Game->ScreenTexture.texture.width,
			-(float)Game->ScreenTexture.texture.height
		};
		Rectangle r = Game->CurScreenRect;
		r.height = -r.height;
		DrawTextureRec(Game->ScreenTexture.texture, screenRect, {}, WHITE);

		Rectangle lightmapRect
		{
			0.0f,
			0.0f,
			(float)GetScreenWidth(),
			-(float)GetScreenHeight()
		};
		DrawTextureRec(Game->ScreenLightMapTexture.texture, lightmapRect, {}, WHITE);

		rlPopMatrix();
		EndMode2D();

		// ***************
		// UI
		// ***************
		//BeginShaderMode(Resources.SDFFont.Shader);
		RenderUI(UIState);

		const double drawStart = GetTime();
		EndDrawing();
		RenderTime = GetTime() - drawStart;
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
	assert(GetGameApp()->Game->World.EntityMgr.Players.size() > 0);
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

global_var float WorldScale = 2.0f;
float GetScale()
{
	return WorldScale;
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
		to = Vector2Multiply(to, { GetScale(), GetScale() });
		game->CameraT += GetDeltaTime();
		Vector2 cameraPos = Vector2Lerp(from, to, game->CameraT);
		camera.target.x = cameraPos.x;
		camera.target.y = cameraPos.y;
		camera.offset.x = (float)GetScreenWidth() / 2.0f;
		camera.offset.y = (float)GetScreenHeight() / 2.0f;
	}

	Vector2 s0 = GetScreenToWorld2D(camera.target, camera);
	Vector2 s1 = GetScreenToWorld2D({
			16.0f * 32.0f * 4.0f,
			16.0f * 32.0f * 3.0f
		}, game->Camera);

	game->CurScreenRect.x = 0;
	game->CurScreenRect.y = 0;
	game->CurScreenRect.width = (float)GetScreenWidth();
	game->CurScreenRect.height = (float)GetScreenHeight();

	game->CurWorldScreenRect.x = camera.target.x;
	game->CurWorldScreenRect.y = camera.target.y;
	game->CurWorldScreenRect.width = (float)GetScreenWidth();
	game->CurWorldScreenRect.height = (float)GetScreenHeight();
}

internal bool InitializeGame(Game* game, GameApplication* gameApp)
{
	CALL_CONSTRUCTOR(game) Game();


	int texW = 16 * 32 * 4;
	int texH = 16 * 32 * 3;
	game->ScreenTexture = LoadRenderTexture(texW, texH);
	game->ScreenLightMapTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
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
	game->Camera.zoom = 1.0f;
	#endif
	S_LOG_INFO("Game Initialized!");
	return true;
}
