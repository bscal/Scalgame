#include "Game.h"

#include "Creature.h"
#include "LightSource.h"
#include "ResourceManager.h"
#include "SpriteAtlas.h"
#include "SMemory.h"
#include "SUI.h"
#include "SUtil.h"

#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"

#include "raymath.h"
#include "rlgl.h"

static GameApplication* GameAppPtr;
static bool IsShowingDebugUi = true;

// TODO
static int UniformSizeId = -1;
static int UniformTilesId = -1;

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
	bool didResInit = InitializeResources(Resources);
	assert(didResInit);
    
	UniformSizeId = GetShaderLocation(Resources->LightShader, "Size");
	UniformTilesId = GetShaderLocation(Resources->LightShader, "Tiles");
    
	UIState = (struct UIState*)Scal::MemAllocZero(sizeof(struct UIState));
	bool didUiInit = InitializeUI(UIState, this);
	assert(didUiInit);
	
	//TODO disable,
	//InitiailizeDebugWindow(&Resources->MainFontM, 10, 30, DARKGREEN);
    
	Game = (struct Game*)Scal::MemAllocZero(sizeof(struct Game));
	bool didGameInit = InitializeGame(Game, this);
	assert(didGameInit);
    
	Game->Atlas.Load("assets/textures/atlas/tiles.atlas", 32);
    
	Test();
	TestSTable();
    
	IsInitialized = true;
	return IsInitialized;
}

SAPI void GameApplication::Shutdown()
{
	GameAppPtr = nullptr;
	UnloadFont(Resources->FontSilver);
	UnloadFont(Resources->MainFontM);
	UnloadTexture(Resources->EntitySpriteSheet);
	UnloadShader(Resources->LightShader);
	UnloadShader(Resources->TileShader);
	UnloadShader(Resources->LightRayShader);
	FreeGame(Game);
	CloseWindow();
}

internal void LoadScreen(GameApplication* gameApp, int width, int height)
{
	const auto& game = gameApp->Game;

	game->HalfWidthHeight.x = (float)width / 2.0f;
	game->HalfWidthHeight.y = (float)height / 2.0f;
	game->WorldCamera.offset = game->HalfWidthHeight;
	game->ViewCamera.offset = game->HalfWidthHeight;

	game->CurScreenRect.width = (float)width;
	game->CurScreenRect.height = (float)height;

	UnloadRenderTexture(game->ScreenTexture);
	UnloadRenderTexture(game->ScreenLightMapTexture);

	game->ScreenTexture = LoadRenderTexture(width, height);
	game->ScreenLightMapTexture = LoadRenderTexture(width, height);

	Vector2 size = { (float)width, (float)height };
	SetShaderValue(gameApp->Resources->LightShader,
		UniformSizeId, &size, SHADER_UNIFORM_VEC2);
}

internal bool InitializeGame(Game* game, GameApplication* gameApp)
{
	// NOTE: Used for now. I use stl structures and since Game is malloced
	// constructors do not get called, messing up these. No ideal,
	// but seems to work.
	CALL_CONSTRUCTOR(game) Game();

	LoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());

	game->ChunkViewDistance = { 4, 3 };
	WorldInitialize(&game->World, gameApp);

	#if Mode3D
	Camera3D camera = {};
	camera.position = { 0.0f, 0.0f, 100.0f };   // Camera position
	camera.target = { 0.0f, 0.0f, 0.0f };        // Camera looking at point
	camera.up = { 0.0f, 1.0f, 0.0f };            // Camera up vector (rotation towards target)
	camera.fovy = 90.0f;                         // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;      // Camera mode type
	game->Camera3D = camera;
	SetCameraMode(game->Camera3D, game->Camera3D.projection);
	#else
	game->WorldCamera.zoom = 1.0f;
	game->ViewCamera.zoom = 1.0f;
	gameApp->Scale = 1.0f;
	#endif

	assert(game->WorldCamera.zoom > 0.0f);
	assert(gameApp->Scale > 0.0f);

	S_LOG_INFO("[ GAME ] Successfully initialized game!");
	return true;
}

internal void FreeGame(Game* game)
{
	UnloadRenderTexture(game->ScreenTexture);
	UnloadRenderTexture(game->ScreenLightMapTexture);
	game->Atlas.Unload();
	WorldFree(&game->World);
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
        
		// Don't want game input when over UI
		if (!UIState->IsMouseHoveringUI)
		{
			IsKeyPressed(KEY_LEFT_SHIFT);
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
				SetCameraDistance(this, mouseWheel);
			}
            
			// Debug place tiles
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			{
				// LocalToWorld
				Matrix invMatCamera = MatrixInvert(GetCameraMatrix2D(Game->WorldCamera));
				Vector3 transform = Vector3Transform(
                                                     { (float)GetMouseX(), (float)GetMouseY(), 0.0f }, invMatCamera);
                
				int x = (int)transform.x / 16;
				int y = (int)transform.y / 16;
				
				Vector2i vec = ScaleWorldVec2i({ x,y });
                
				Tile* tile = CTileMap::GetTile(&Game->World.ChunkedTileMap, { x, y });
                
				Tile newTile = CreateTileId(&Game->World.TileMgr, 5);
				CTileMap::SetTile(&Game->World.ChunkedTileMap, &newTile, { vec.x, vec.y });
                
				S_LOG_INFO("Clicked Tile[%d, %d] Id: %d",
                           vec.x, vec.y, tile->TileDataId);
			}
            
			if (IsKeyPressed(KEY_ONE))
			{
				Action testAction = {};
				testAction.Cost = 100;
				testAction.ActionFunction = TestActionFunc;
				AddAction(&Game->World, &testAction);
			}
            
			if (IsKeyPressed(KEY_GRAVE))
			{
				IsShowingDebugUi = !IsShowingDebugUi;
			}
		}
        
		if (IsShowingDebugUi)
		{
			UpdateUI(UIState);
		}
        
		// ***************
		// Draw world to Texture
		// ***************
        
		BeginShaderMode(Resources->TileShader);
        BeginTextureMode(Game->ScreenTexture);
        ClearBackground({});

        BeginMode2D(Game->WorldCamera);

        UpdateGame(Game, this);
        WorldUpdate(&Game->World, Game);

        EndMode2D();
        EndTextureMode();
		EndShaderMode();
        
		Game->World.LightMap.BuildLightMap();
		Game->World.LightMap.UpdatePositions({});
        
		const auto& shader = Resources->LightShader;
		BeginShaderMode(shader);
        BeginTextureMode(Game->ScreenLightMapTexture);

        ClearBackground({});
        // TODO look into updating only if tiles to draw?
        SetShaderValueTexture(shader, UniformTilesId, Game->World.LightMap.Texture.texture);
        Rectangle rectSize
        { 
            0.0f, 0.0f,
            (float)GetScreenWidth(), (float)GetScreenHeight()
        };
        DrawRectanglePro(rectSize, {}, 0.0f, WHITE);

        EndTextureMode();
		EndShaderMode();
		
        
		// ***************
		// Draw
		// ***************
        
		BeginDrawing();
        
		BeginMode2D(Game->ViewCamera);
        ClearBackground(BLACK);
        
        Rectangle drawRect
        {
            0.0f, 0.0f,
            (float)GetScreenWidth(), (float)-GetScreenHeight()
        };
        
        Vector2 offset = Vector2Subtract(Game->WorldCamera.target, Game->WorldCamera.offset);
        Rectangle screenRect
        {
            offset.x, offset.y,
            (float)GetScreenWidth(), (float)GetScreenHeight()
        };
        
        rlPushMatrix();
        rlScalef(GetScale(), GetScale(), 1.0f);
        
        DrawTexturePro(Game->ScreenTexture.texture,
                       drawRect, screenRect, {}, 0.0f, WHITE);
        
        DrawTextureRec(Game->ScreenLightMapTexture.texture,
                       drawRect, {}, WHITE);
        
        rlPopMatrix();

		EndMode2D();
        
		RenderUI(UIState);
        
		double drawStart = GetTime();
		EndDrawing();
		RenderTime = GetTime() - drawStart;
	}
	IsRunning = false;
}

internal void UpdateGame(Game* game, GameApplication* gameApp)
{
	Camera2D& camera = game->WorldCamera;

	// Handle Camera Move
	if (!game->IsFreeCam)
	{
		if (game->CameraLerpTime > 1.0f) game->CameraLerpTime = 1.0f;
		else game->CameraLerpTime += GetDeltaTime();

		Vector2 from = camera.target;
		Vector2 playerPos = GetClientPlayer()->Transform.Pos;
		playerPos.x += 8.0f;
		playerPos.y += 8.0f;
		Vector2 cameraPos = Vector2Lerp(from, playerPos, game->CameraLerpTime);
		camera.target = cameraPos;

		Vector2 viewTarget = Vector2Multiply(cameraPos, { GetScale(), GetScale() });
		game->ViewCamera.target = viewTarget;
	}

	Vector2 viewScaled = Vector2Divide(
		Vector2Subtract(game->ViewCamera.target, game->ViewCamera.offset),
		{ GetScale(), GetScale() });
	game->CurScreenRect.x = viewScaled.x;
	game->CurScreenRect.y = viewScaled.y;

	if (IsWindowResized())
	{
		LoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());
		S_LOG_INFO("[ GAME ] Window Resizing!");
	}
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

float GetDeltaTime()
{
	return GetFrameTime();
}

float GetScale()
{
	assert(GetGameApp()->Scale > 0.0f);
	return GetGameApp()->Scale;
}

Vector2 ScaleWorldVec2(Vector2 vec)
{
	return Vector2Divide(vec, { GetScale(), GetScale() });
}

Vector2i ScaleWorldVec2i(Vector2i vec)
{
	return vec.Divide({ (int)GetScale(), (int)GetScale() });
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
	Camera2D& camera = game->WorldCamera;
	camera.target.x += pos.x;
	camera.target.y += pos.y;
#endif
}

void SetCameraDistance(GameApplication* gameApp, float zoom)
{
#if Mode3D // 3D
	Camera3D& camera = game->Camera;
	zoom = camera->position.z + zoom * 15.0f;
	if (zoom > 256.0f) zoom = 250.0f;
	if (zoom < 16.0f) zoom = 15.0f;
	camera->position.z = floorf(zoom);
#else // 2D
	gameApp->Scale += zoom * 0.25f;
	if (gameApp->Scale < 1.0f)
		gameApp->Scale = 1.0f;
	else if (gameApp->Scale > 4.0f)
		gameApp->Scale = 4.0f;
    
	Vector2 playerPos = GetClientPlayer()->Transform.Pos;
	playerPos.x += 8.0f;
	playerPos.y += 8.0f;
	gameApp->Game->WorldCamera.target = playerPos;
	Vector2 viewTarget = Vector2Multiply(playerPos, { GetScale(), GetScale() });
	gameApp->Game->ViewCamera.target = viewTarget;
#endif
}