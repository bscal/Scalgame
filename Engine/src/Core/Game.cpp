#include "Game.h"

#include "Creature.h"
#include "ResourceManager.h"
#include "SpriteAtlas.h"
#include "Lighting.h"
#include "SUI.h"
#include "SUtil.h"
#include "SString.h"

#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/SHoodTable.h"

#include "raymath.h"
#include "rlgl.h"

global_var GameApplication* GameAppPtr;

SAPI bool GameApplication::Start()
{
	if (IsInitialized)
	{
		TraceLog(LOG_ERROR, "GameApplication already initialized!");
		return false;
	}

	size_t gameMemorySize = Megabytes(32);
	size_t tempMemorySize = Megabytes(4);
	SMemInitialize(this, gameMemorySize, tempMemorySize);

	const int screenWidth = 1600;
	const int screenHeight = 900;
	InitWindow(screenWidth, screenHeight, "Some roguelike game");
	SetTargetFPS(144);
	SetTraceLogLevel(LOG_ALL);

	GameAppPtr = this;

	Game = (struct Game*)SMemAllocTag(sizeof(struct Game), MemoryTag::Game);
	SMemClear(Game, sizeof(struct Game));
	bool didGameInit = GameInitialize(Game, this);
	SASSERT(didGameInit);

	UIState = (struct UIState*)SMemAllocTag(sizeof(struct UIState), MemoryTag::UI);
	SMemClear(UIState, sizeof(struct UIState));
	bool didUiInit = InitializeUI(UIState, this);
	SASSERT(didUiInit);

	#ifdef SCAL_GAME_TESTS

	#define GAME_TEST(function) ++totalTests; passingTests += function()
	int passingTests = 0;
	int totalTests = 0;

	GAME_TEST(TestListImpl);
	GAME_TEST(TestSTable);
	GAME_TEST(TestEntities);
	GAME_TEST(TestStringImpls);
	GAME_TEST(TestSHoodTable);

	SLOG_INFO("[ Tests ] %d/%d tests passed!", passingTests, totalTests);
	#endif // SCAL_GAME_TESTS



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
}

internal bool GameInitialize(Game* game, GameApplication* gameApp)
{
	// NOTE: Used for now. I use stl structures and since Game is malloced
	// constructors do not get called, messing up these. No ideal,
	// but seems to work.
	CALL_CONSTRUCTOR(game) Game();

	bool didResInit = InitializeResources(&game->Resources);
	SASSERT(didResInit);

	GameLoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());

	TileMgrInitialize(&game->TileMgr, &game->Resources.Atlas);
	EntityMgrInitialize(game);

	game->WorldCamera.zoom = 1.0f;
	game->ViewCamera.zoom = 1.0f;
	gameApp->Scale = 1.0f;

	SLOG_INFO("[ GAME ] Successfully initialized game!");
	return true;
}

internal void GameStart(Game* game, GameApplication* gameApp)
{
	// TODO world loading / world settings
	WorldInitialize(&game->World, gameApp);

	Player* player = CreatePlayer(&game->EntityMgr, &game->World);
	player->TextureInfo.Rect = PLAYER_SPRITE.TexCoord;

	Human human = {};
	human.Age = 30;
	game->EntityMgr.ComponentManager.AddComponent(player, &human);

	Human* playerHuman = game->EntityMgr.ComponentManager
		.GetComponent<Human>(player, Human::ID);

	SASSERT(playerHuman);
	SASSERT(playerHuman->Age == 30);
	SASSERT(playerHuman->EntityId == player->Id);

	SLOG_INFO("[ WORLD ] players age is %d from componentId: %d",
		playerHuman->Age, playerHuman->ID);

	game->EntityMgr.ComponentManager.RemoveComponent<Human>(player, Human::ID);

	Human* playerHuman2 = game->EntityMgr.ComponentManager
		.GetComponent<Human>(player, Human::ID);
	SASSERT(!playerHuman2);


	SCreature* rat = CreateCreature(&game->EntityMgr, &game->World);
	rat->TextureInfo.Rect = RAT_SPRITE.TexCoord;
	rat->SetTilePos({ 5, 5 });

	MarkEntityForRemove(&game->EntityMgr, rat->Id);

	SCreature* getRat = (SCreature*)FindEntity(&game->EntityMgr, rat->Id);
	SASSERT(getRat);
	SASSERT(getRat->ComponentIndex[0] == CREATURE_EMPTY_COMPONENT);
	SASSERT(GetEntityId(getRat->Id) == 1);
	SASSERT(GetEntityType(getRat->Id) == CREATURE);
	SASSERT(getRat->ShouldRemove);

	WorldLoad(&game->World, game);
}

internal void GameFree(Game* game)
{
	UnloadFont(game->Resources.FontSilver);
	UnloadFont(game->Resources.MainFontM);
	UnloadTexture(game->Resources.EntitySpriteSheet);
	UnloadShader(game->Resources.UnlitShader);
	game->Resources.Atlas.Unload();
	UnloadRenderTexture(game->WorldTexture);
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

		SMemTempReset();

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
				Vector2i clickedTilePos = GetTileFromMouse(Game);
				auto tilemap = &Game->World.ChunkedTileMap;
				if (CTileMap::IsTileInBounds(tilemap, clickedTilePos))
				{
					Tile* tile = CTileMap::GetTile(tilemap, clickedTilePos);
					Tile newTile = CreateTileId(&Game->TileMgr, 5);
					CTileMap::SetTile(&Game->World.ChunkedTileMap, &newTile, clickedTilePos);
					SLOG_INFO("Clicked Tile[%d, %d] Id: %d", clickedTilePos.x, clickedTilePos.y, tile->TileId);
				}
			}
			if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			{
				Vector2i clickedTilePos = GetTileFromMouse(Game);
				if (CTileMap::IsTileInBounds(&Game->World.ChunkedTileMap, clickedTilePos))
				{
					Light light = {};
					light.Pos = clickedTilePos.AsVec2();
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
			if (IsKeyPressed(KEY_F3))
			{
				Game->DebugTileView = !Game->DebugTileView;
			}
			if (IsKeyPressed(KEY_F4))
			{
				if (Game->ViewCamera.zoom == 1.f)
					Game->ViewCamera.zoom = .5f;
				else
					Game->ViewCamera.zoom = 1.f;
			}
			if (IsKeyPressed(KEY_GRAVE))
				UIState->IsDebugPanelOpen = !UIState->IsDebugPanelOpen;
			if (IsKeyPressed(KEY_EQUAL))
				UIState->IsConsoleOpen = !UIState->IsConsoleOpen;
			if (IsKeyPressed(KEY_BACKSLASH))
				UIState->IsDrawingFPS = !UIState->IsDrawingFPS;
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
		GameLateUpdate(Game);

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
		SLOG_INFO("[ GAME ] Window Resizing!");
	}

	// Base LOS for player
	SList<Vector2i> nearbyTiles = QueryTilesRadius(&game->World, GetClientPlayer()->Transform.TilePos, 4.0f);
	for (int i = 0; i < nearbyTiles.Count; ++i)
	{
		CTileMap::SetVisible(
			&game->World.ChunkedTileMap,
			nearbyTiles[i]);

		CTileMap::SetColor(
			&game->World.ChunkedTileMap,
			nearbyTiles[i],
			{ 1.0f, 1.0f, 1.0f });
	}

	WorldUpdate(&game->World, game);
	EntityMgrUpdate(&game->EntityMgr, game);
}

internal void GameLateUpdate(Game* game)
{
	WorldLateUpdate(&game->World, game);
}

GameApplication* const GetGameApp()
{
	assert(GameAppPtr->IsInitialized);
	return GameAppPtr;
}

Player* const GetClientPlayer()
{
	assert(GetGameApp()->Game->EntityMgr.Players.Count > 0);
	return GetGameApp()->Game->EntityMgr.Players.PeekAt(0);
}

EntityMgr* GetEntityMgr()
{
	assert(GetGameApp()->Game->World.IsAllocated);
	return &GetGameApp()->Game->EntityMgr;
}

Game* const GetGame()
{
	return GetGameApp()->Game;
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

Vector2 GetZoomedMousePos(const Camera2D& camera)
{
	Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
	return Vector2Divide(mouseWorld, { GetScale(), GetScale() });
}

Vector2i GetTileFromMouse(Game* game)
{
	Vector2 mousePos = GetZoomedMousePos(game->ViewCamera);
	return WorldToTileCoord(&game->World, mousePos);
}

void SetCameraPosition(Game* game, Vector3 pos)
{
	Camera2D& camera = game->ViewCamera;
	camera.target.x += pos.x;
	camera.target.y += pos.y;
}

void SetCameraDistance(GameApplication* gameApp, float zoom)
{
	const float ZOOM_MAX = 5.0f;
	const float ZOOM_MIN = 1.0f;
	const float ZOOM_SPD = .25f;

	float newZoom = gameApp->Scale + zoom * ZOOM_SPD;
	if (newZoom > ZOOM_MAX) newZoom = ZOOM_MAX;
	else if (newZoom < ZOOM_MIN) newZoom = ZOOM_MIN;
	gameApp->Scale = newZoom;

	// NOTE: this is so we dont get weird camera
	// jerking when we scroll
	Vector2 playerPos = VecToTileCenter(GetClientPlayer()->Transform.Pos);
	gameApp->Game->WorldCamera.target = playerPos;
	Vector2 viewTarget = Vector2Multiply(playerPos, { GetScale(), GetScale() });
	gameApp->Game->ViewCamera.target = viewTarget;
}
