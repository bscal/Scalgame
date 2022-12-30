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

global_var GameApplication* GameAppPtr;
global_var bool IsShowingDebugUi = true;

// TODO
global_var int UniformLitTileMapId = -1;
global_var int UniformLitLightId = -1;
global_var int UniformLitSizeId = -1;

global_var int UniformSizeId = -1;
global_var int UniformTilesId = -1;
global_var int UniformLightId = -1;

global_var int UniformSamplerSizeId = -1;
global_var int UniformSamplerTilesId = -1;
global_var int UniformSamplerLightId = -1;
global_var int UniformSamplerWorldId = -1;

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
    
	UniformLitSizeId = GetShaderLocation(Resources->LitShader, "Size");
	UniformLitTileMapId = GetShaderLocation(Resources->LitShader, "TileMap");
	UniformLitLightId = GetShaderLocation(Resources->LitShader, "Light");

	UniformSizeId = GetShaderLocation(Resources->LightRayShader, "Size");
	UniformTilesId = GetShaderLocation(Resources->LightRayShader, "Tiles");
	UniformLightId = GetShaderLocation(Resources->LightRayShader, "Light");
    
	UniformSamplerSizeId = GetShaderLocation(Resources->LightSamplerShader, "Size");
	UniformSamplerTilesId = GetShaderLocation(Resources->LightSamplerShader, "LightMap");
	UniformSamplerLightId = GetShaderLocation(Resources->LightSamplerShader, "Light");
	UniformSamplerWorldId = GetShaderLocation(Resources->LightSamplerShader, "WorldMap");

	UIState = (struct UIState*)Scal::MemAllocZero(sizeof(struct UIState));
	bool didUiInit = InitializeUI(UIState, this);
	assert(didUiInit);
	
	//TODO disable,
	//InitiailizeDebugWindow(&Resources->MainFontM, 10, 30, DARKGREEN);
    
	Game = (struct Game*)Scal::MemAllocZero(sizeof(struct Game));
	bool didGameInit = GameInitialize(Game, this);
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
	UnloadShader(Resources->LightRayShader);
	UnloadShader(Resources->UnlitShader);
	UnloadShader(Resources->LightSamplerShader);
	GameFree(Game);
	CloseWindow();
}

internal void GameLoadScreen(GameApplication* gameApp, int width, int height)
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
	UnloadRenderTexture(game->LightTexture);
	UnloadRenderTexture(game->ViewTexture);

	game->ScreenTexture = LoadRenderTexture(width, height);
	game->ScreenLightMapTexture = LoadRenderTexture(width, width);
	game->LightTexture = LoadRenderTexture(width / 16, width / 16);
	game->ViewTexture = LoadRenderTexture(width, height);

	Vector2 size = { (float)width, (float)height };
	SetShaderValue(gameApp->Resources->LightRayShader,
		UniformSizeId, &size, SHADER_UNIFORM_VEC2);

	SetShaderValue(gameApp->Resources->LightSamplerShader,
		UniformSamplerSizeId, &size, SHADER_UNIFORM_VEC2);

	SetShaderValue(gameApp->Resources->LitShader,
		UniformLitSizeId, &size, SHADER_UNIFORM_VEC2);
}

internal bool GameInitialize(Game* game, GameApplication* gameApp)
{
	// NOTE: Used for now. I use stl structures and since Game is malloced
	// constructors do not get called, messing up these. No ideal,
	// but seems to work.
	CALL_CONSTRUCTOR(game) Game();

	GameLoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());

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

internal void GameFree(Game* game)
{
	UnloadRenderTexture(game->ScreenTexture);
	UnloadRenderTexture(game->ScreenLightMapTexture);
	UnloadRenderTexture(game->LightTexture);
	UnloadRenderTexture(game->ViewTexture);
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
        
		// *****************
		// Update/Draw World
		// *****************
        
		BeginShaderMode(Resources->UnlitShader);
        
		BeginTextureMode(Game->ScreenTexture);
        BeginMode2D(Game->WorldCamera);
		ClearBackground({});

		// World
        GameUpdate(Game, this);
		WorldUpdate(&Game->World, Game);

		EndMode2D();
        EndTextureMode();

		// LightMap
		BeginTextureMode(Game->LightTexture);
		ClearBackground({});
		Game->World.LightMap.BuildLightMap(Game);
		Game->World.LightMap.UpdatePositions({});
		EndTextureMode();

		EndShaderMode();

		auto m = GetMousePosition();
		Vector3 light;
		light.x = m.x;//Game->World.LightMap.LightSources[0].Position.x;
		light.y = m.y;//Game->World.LightMap.LightSources[0].Position.y;
		light.z = 300.0f;//Game->World.LightMap.LightSources[0].Intensity;

		// Light Shader/Texture
		//const auto& shader = Resources->LightRayShader;
		//BeginShaderMode(shader);
        //BeginTextureMode(Game->ScreenLightMapTexture);
		//SetShaderValue(shader, UniformLightId, &light, SHADER_UNIFORM_VEC3);
		//ClearBackground({ 0, 0, 0, 255 });
        // TODO look into updating only if tiles to draw?
        //SetShaderValueTexture(shader, UniformTilesId, 
			//Game->LightTexture.texture);

		Rectangle rect
        { 
			0.0f, 0.0f,
			(float)GetScreenWidth(), (float)GetScreenWidth()
        };
        //DrawRectangleRec(rect, WHITE);
        //EndTextureMode();
		//EndShaderMode();
		
		// **************
		// Draw to buffer
		// **************
		BeginDrawing();
        
		BeginMode2D(Game->ViewCamera);
        ClearBackground(BLACK);
        

		//SetShaderValue(Resources->LitShader, UniformLitLightId,
			//&light, SHADER_UNIFORM_VEC3);

		//SetShaderValueTexture(Resources->LitShader, UniformLitTileMapId,
			//Game->LightTexture.texture);

		BeginShaderMode(Resources->UnlitShader);

		Rectangle srcRect
		{
			0.0f, 0.0f, (float)GetScreenWidth(), (float)-GetScreenHeight()
		};

        Vector2 offset = Vector2Subtract(Game->WorldCamera.target,
			Game->WorldCamera.offset);

        Rectangle destRect
        {
            offset.x, offset.y, (float)GetScreenWidth(), (float)GetScreenHeight()
        };
        

        rlPushMatrix();
        rlScalef(GetScale(), GetScale(), 1.0f);
        
		//DrawRectanglePro(destRect, {}, 0.0f, WHITE);

        DrawTexturePro(Game->ScreenTexture.texture,
			srcRect, destRect, {}, 0.0f, WHITE);
        
		// TODO unhardcore value, half of tilesize
		// Camera.target is centered on player's tile
		//destRect.x -= 8.0f;
		//destRect.y -= 8.0f;
		//DrawTexturePro(Game->ScreenLightMapTexture.texture,
		//	srcRect, destRect, {}, 0.0f, WHITE);
        
        rlPopMatrix();

		EndShaderMode();

		EndMode2D();
        
		DrawUI(UIState);
        
		// Swap buffers
		double drawStart = GetTime();
		EndDrawing();
		RenderTime = GetTime() - drawStart;
	}
	IsRunning = false;
}

internal void GameUpdate(Game* game, GameApplication* gameApp)
{
	// Handle Camera Move
	if (!game->IsFreeCam)
	{
		if (game->CameraLerpTime > 1.0f) game->CameraLerpTime = 1.0f;
		else game->CameraLerpTime += GetDeltaTime();

		Vector2 from = game->WorldCamera.target;
		Vector2 playerPos = GetClientPlayer()->Transform.Pos;
		playerPos.x += 8.0f;
		playerPos.y += 8.0f;
		Vector2 cameraPos = Vector2Lerp(from, playerPos,
			game->CameraLerpTime);
		game->WorldCamera.target = cameraPos;

		Vector2 viewTarget = Vector2Multiply(cameraPos,
			{ GetScale(), GetScale() });
		game->ViewCamera.target = viewTarget;
	}

	Vector2 viewScaled = Vector2Divide(
		Vector2Subtract(game->ViewCamera.target, game->ViewCamera.offset),
		{ GetScale(), GetScale() });
	game->CurScreenRect.x = viewScaled.x;
	game->CurScreenRect.y = viewScaled.y;

	if (IsWindowResized())
	{
		GameLoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());
		S_LOG_INFO("[ GAME ] Window Resizing!");
	}
}

GameApplication* const GetGameApp()
{
	assert(GameAppPtr->IsInitialized);
    return GameAppPtr;
}

Player* GetClientPlayer()
{
	assert(GetGameApp()->Game->World.EntityMgr.Players.size() > 0);
	return &GetGameApp()->Game->World.EntityMgr.Players[0];
}

const World& GetMainWorld()
{
	assert(GetGameApp()->Game->World.IsInitialized)
	return GetGameApp()->Game->World;
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