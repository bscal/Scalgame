#include "Game.h"

#include "Creature.h"
#include "LightSource.h"
#include "ResourceManager.h"
#include "SpriteAtlas.h"
#include "Lighting.h"
#include "SMemory.h"
#include "SUI.h"
#include "SUtil.h"
#include "SMath.h"

#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"

#include "rlgl.h"

global_var GameApplication* GameAppPtr;

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
   
	Game = (struct Game*)Scal::MemAllocZero(sizeof(struct Game));
	bool didGameInit = GameInitialize(Game, this);
	assert(didGameInit);

	UIState = (struct UIState*)Scal::MemAllocZero(sizeof(struct UIState));
	bool didUiInit = InitializeUI(UIState, this);
	assert(didUiInit);
    
	Test();
	TestSTable();

	IsInitialized = true;

	GameStart(Game, this);

	return IsInitialized;
}

SAPI void GameApplication::Shutdown()
{
	GameAppPtr = nullptr;
	GameFree(Game);
	CloseWindow();
}

internal void GameLoadScreen(GameApplication* gameApp, int width, int height)
{
	const auto game = gameApp->Game;

	gameApp->HalfWidthHeight.x = (float)width / 2.0f;
	gameApp->HalfWidthHeight.y = (float)height / 2.0f;
	game->WorldCamera.offset = gameApp->HalfWidthHeight;
	game->ViewCamera.offset = gameApp->HalfWidthHeight;

	UnloadRenderTexture(game->WorldTexture);

	game->WorldTexture = LoadRenderTexture(width, height);

	Vector2 size = { (float)width, (float)height };
	SetShaderValue(game->Resources.LightRayShader,
		UniformSizeId, &size, SHADER_UNIFORM_VEC2);

	SetShaderValue(game->Resources.LightSamplerShader,
		UniformSamplerSizeId, &size, SHADER_UNIFORM_VEC2);

	SetShaderValue(game->Resources.LitShader,
		UniformLitSizeId, &size, SHADER_UNIFORM_VEC2);
}

internal bool GameInitialize(Game* game, GameApplication* gameApp)
{
	// NOTE: Used for now. I use stl structures and since Game is malloced
	// constructors do not get called, messing up these. No ideal,
	// but seems to work.
	CALL_CONSTRUCTOR(game) Game();

	bool didResInit = InitializeResources(&game->Resources);
	assert(didResInit);

	UniformLitSizeId = GetShaderLocation(game->Resources.LitShader, "Size");
	UniformLitTileMapId = GetShaderLocation(game->Resources.LitShader, "TileMap");
	UniformLitLightId = GetShaderLocation(game->Resources.LitShader, "Light");

	UniformSizeId = GetShaderLocation(game->Resources.LightRayShader, "Size");
	UniformTilesId = GetShaderLocation(game->Resources.LightRayShader, "Tiles");
	UniformLightId = GetShaderLocation(game->Resources.LightRayShader, "Light");

	UniformSamplerSizeId = GetShaderLocation(game->Resources.LightSamplerShader, "Size");
	UniformSamplerTilesId = GetShaderLocation(game->Resources.LightSamplerShader, "LightMap");
	UniformSamplerLightId = GetShaderLocation(game->Resources.LightSamplerShader, "Light");
	UniformSamplerWorldId = GetShaderLocation(game->Resources.LightSamplerShader, "WorldMap");

	game->Atlas.Load("assets/textures/atlas/tiles.atlas", 32);

	GameLoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());

	TileMgrInitialize(&game->TileMgr, &game->Atlas);
	EntityMgrInitialize(game);

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

internal void GameStart(Game* game, GameApplication* gameApp)
{
	// TODO world loading / world settings
	game->ChunkViewDistance = { 4, 3 };
	WorldInitialize(&game->World, gameApp);

	Player* player = CreatePlayer(&game->EntityMgr, &game->World);
	player->TextureInfo.Rect = PLAYER_SPRITE.TexCoord;

	Human human = {};
	human.Age = 30;
	game->EntityMgr.ComponentManager.AddComponent(player, &human);

	Human* playerHuman = game->EntityMgr.ComponentManager
		.GetComponent<Human>(player, Human::ID);

	S_LOG_INFO("[ WORLD ] players age is %d from componentId: %d",
		playerHuman->Age, playerHuman->ID);

	SCreature* rat = CreateCreature(&game->EntityMgr, &game->World);
	rat->TextureInfo.Rect = RAT_SPRITE.TexCoord;
	rat->SetTilePos({ 5, 5 });

	MarkEntityForRemove(&game->EntityMgr, rat->Id);

	WorldLoad(&game->World, game);
}

internal void GameFree(Game* game)
{
	UnloadFont(game->Resources.FontSilver);
	UnloadFont(game->Resources.MainFontM);
	UnloadTexture(game->Resources.EntitySpriteSheet);
	UnloadShader(game->Resources.LightRayShader);
	UnloadShader(game->Resources.UnlitShader);
	UnloadShader(game->Resources.LightSamplerShader);
	UnloadRenderTexture(game->WorldTexture);
	game->Atlas.Unload();
	WorldFree(&game->World);
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
				Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), Game->ViewCamera);
				Vector2i clickedTileCoord = WorldToTileCoord(&Game->World, mouseWorld);
				Vector2i vec = ScaleWorldVec2i(clickedTileCoord);;

				auto tilemap = &Game->World.ChunkedTileMap;
				if (CTileMap::IsTileInBounds(tilemap, vec))
				{
					Tile* tile = CTileMap::GetTile(tilemap, vec);
					Tile newTile = CreateTileId(&Game->TileMgr, 5);
					CTileMap::SetTile(&Game->World.ChunkedTileMap, &newTile, vec);
					S_LOG_INFO("Clicked Tile[%d, %d] Id: %d", vec.x, vec.y, tile->TileDataId);
				}
			}
			if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), Game->ViewCamera);
				Vector2i clickedTileCoord = WorldToTileCoord(&Game->World, mouseWorld);
				Vector2i vec = ScaleWorldVec2i(clickedTileCoord);

				if (CTileMap::IsTileInBounds(&Game->World.ChunkedTileMap, vec))
				{
					Light light = {};
					light.Pos = vec.AsVec2();
					light.Radius = 16.0f;
					light.Intensity = 1.0f;

					local_var int next = 0;
					Color c;
					++next;
					if (next == 1)
						c = RED;
					else if (next == 2)
						c = GREEN;
					else if (next == 3)
						c = BLUE;
					else
					{
						c = WHITE;
						next = 0;
					}

					light.Color = c;
					LightsAdd(light);
				}
			}
            
			if (IsKeyPressed(KEY_ONE))
			{
				Action testAction = {};
				testAction.Cost = 100;
				testAction.ActionFunction = TestActionFunc;
				AddAction(&Game->World, &testAction);
			}
            
			if (IsKeyPressed(KEY_F1))
			{
				Game->DebugDisableDarkess = !Game->DebugDisableDarkess;
			}
			if (IsKeyPressed(KEY_F2))
			{
				Game->DebugDisableFOV = !Game->DebugDisableFOV;
			}
			if (IsKeyPressed(KEY_GRAVE))
			{
				UIState->IsDebugPanelOpen = !UIState->IsDebugPanelOpen;
			}
			if (IsKeyPressed(KEY_EQUAL))
			{
				UIState->IsConsoleOpen = !UIState->IsConsoleOpen;
			}
		}
        
		// **************************
		// Updates UI logic, draws to
		// screen later in frame
		// **************************
		UpdateUI(UIState);
        
		// *****************
		// Update/Draw World
		// *****************
		BeginShaderMode(Game->Resources.UnlitShader);
			BeginTextureMode(Game->WorldTexture);
				BeginMode2D(Game->WorldCamera);
					ClearBackground(BLACK);

					// Updates
					GameUpdate(Game, this);
					WorldUpdate(&Game->World, Game);
					EntityMgrUpdate(&Game->EntityMgr, Game);
				EndMode2D();
			 EndTextureMode();
		EndShaderMode();

		// ***************
		// Draws to buffer
		// ***************
		BeginDrawing();
        
		BeginShaderMode(Game->Resources.UnlitShader);
			BeginMode2D(Game->ViewCamera);
				rlPushMatrix();
				rlScalef(GetScale(), GetScale(), 1.0f);
        
				float screenW = (float)GetScreenWidth();
				float screenH = (float)GetScreenHeight();
				Rectangle srcRect = { 0.0f, 0.0f, screenW, -screenH };
				Rectangle dstRect = { ScreenXY.x, ScreenXY.y, screenW, screenH };
				DrawTexturePro(Game->WorldTexture.texture, srcRect,
					dstRect, {}, 0.0f, WHITE);
        
				rlPopMatrix();
			EndMode2D();
		EndShaderMode();
        
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
		Vector2 playerPos = VecToTileCenter(GetClientPlayer()->Transform.Pos);
		game->WorldCamera.target = Vector2Lerp(from, playerPos, game->CameraLerpTime);
		game->ViewCamera.target = Vector2Multiply(game->WorldCamera.target, { GetScale(), GetScale() });
	}
	
	gameApp->ScreenXY = Vector2Subtract(game->WorldCamera.target, game->WorldCamera.offset);
	gameApp->ScaledScreenXY = Vector2Divide(gameApp->ScreenXY, { GetScale(), GetScale() });

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
	assert(GetGameApp()->Game->EntityMgr.Players.Length > 0);
	return GetGameApp()->Game->EntityMgr.Players.PeekAtPtr(0);
}

EntityMgr* GetEntityMgr()
{
	assert(GetGameApp()->Game->World.IsInitialized);
	return &GetGameApp()->Game->EntityMgr;
}

Game* GetGame()
{
	return GetGameApp()->Game;
}


Game* GetGame();

float GetDeltaTime()
{
	return GetFrameTime();
}

float GetScale()
{
	assert(GetGameApp()->Scale > 0.0f);
	return GetGameApp()->Scale;
}

Rectangle GetScaledScreenRect()
{
	return { 
		GetGameApp()->ScreenXY.x,
		GetGameApp()->ScreenXY.y,
		(float)GetScreenWidth(),
		(float)GetScreenHeight() };
}

Vector2 VecToTileCenter(Vector2 vec)
{
	return { vec.x + HALF_TILE_SIZE, vec.y + HALF_TILE_SIZE };
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
    
	// NOTE: this is so we dont get weird camera
	// jerking when we scroll
	Vector2 playerPos = VecToTileCenter(GetClientPlayer()->Transform.Pos);
	gameApp->Game->WorldCamera.target = playerPos;
	Vector2 viewTarget = Vector2Multiply(playerPos, { GetScale(), GetScale() });
	gameApp->Game->ViewCamera.target = viewTarget;
#endif
}