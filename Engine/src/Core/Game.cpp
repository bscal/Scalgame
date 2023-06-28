#include "Game.h"

#include "ResourceManager.h"
#include "SpriteAtlas.h"
#include "SUI.h"
#include "SUtil.h"
#include "SString.h"
#include "Entity.h"

#include "Structures/SArray.h"
#include "Structures/SList.h"
#include "Structures/STable.h"
#include "Structures/SHashMap.h"
#include "Structures/SparseSet.h"
#include "Structures/TreeMap.h"
#include "Structures/References.h"
#include "Structures/IndexArray.h"

#include "WickedEngine/Jobs.h"

#include "raymath.h"
#include "rlgl.h"

global_var GameApplication* GameAppPtr;

internal void HandleGameInput(GameApplication* gameApp, Game* game);

internal bool GameInitialize(Game* game, GameApplication* gameApp);
internal void GameLoad(Game* game, GameApplication* gameApp);
internal void GameUnload(Game* game, GameApplication* gameApp);
internal void GameUpdate(Game* game, GameApplication* gameApp);
internal void GameUpdateCamera(Game* game, GameApplication* gameApp);
internal void GameLateUpdate(Game* game);

SAPI bool GameApplication::Start()
{
	if (IsInitialized)
	{
		TraceLog(LOG_ERROR, "GameApplication already initialized!");
		return false;
	}

	double initStart = GetTime();

	size_t gameMemorySize = Megabytes(32);
	size_t tempMemorySize = Megabytes(8);
	SMemInitialize(this, gameMemorySize, tempMemorySize);

	InitProfile("profile.spall");

	int monitor = GetCurrentMonitor();
	int monitorWidth = GetMonitorWidth(monitor);
	int monitorHeight = GetMonitorHeight(monitor);

	View.Resolution = { 1600, 900 };
	View.ResolutionInTiles = View.Resolution / Vector2i{ TILE_SIZE, TILE_SIZE };
	View.ScreenCenter.x = (float)View.Resolution.x / 2.0f;
	View.ScreenCenter.y = (float)View.Resolution.y / 2.0f;
	View.TotalTilesOnScreen = (float)View.ResolutionInTiles.x * (float)View.ResolutionInTiles.y;

	//SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(View.Resolution.x, View.Resolution.y, "Some roguelike game");
	SetTargetFPS(144);
	SetTraceLogLevel(LOG_ALL);

	SRandomInitialize(&GlobalRandom, 0);

	GameAppPtr = this;

	wi::jobsystem::Initialize(6);

	Game = (struct Game*)SAlloc(SAllocator::Game, sizeof(struct Game), MemoryTag::Game);
	UIState = (struct UIState*)SAlloc(SAllocator::Game, sizeof(struct UIState), MemoryTag::UI);

#ifdef SCAL_GAME_TESTS

#define GAME_TEST(function) ++totalTests; passingTests += function()
	int passingTests = 0;
	int totalTests = 0;

	GAME_TEST(TestListImpl);
	GAME_TEST(TestSTable);
	GAME_TEST(TestStringImpls);
	GAME_TEST(TestSHoodTable);
	GAME_TEST(TestSparseSet);
	GAME_TEST(TreeTest);
	GAME_TEST(TestRef);
	GAME_TEST(TestIndexArray);

	SLOG_INFO("[ Tests ] %d/%d tests passed!", passingTests, totalTests);
#endif // SCAL_GAME_TESTS

	IsInitialized = true;

	bool didGameInit = GameInitialize(Game, this);
	SASSERT(didGameInit);

	bool didUiInit = InitializeUI(UIState, this);
	SASSERT(didUiInit);

	GameLoad(Game, this);

	double initEnd = GetTime() - initStart;
	SLOG_INFO("[ Init ] Initialized Success. Took %f second", initEnd);

	return IsInitialized;
}

internal bool GameInitialize(Game* game, GameApplication* gameApp)
{
	// NOTE: Used for now. I use stl structures and since Game is malloced
	// constructors do not get called, messing up these. No ideal,
	// but seems to work.
	CALL_CONSTRUCTOR(game) Game();

	bool didResInit = InitializeResources(&game->Resources);
	SASSERT(didResInit);
	gameApp->HalfWidthHeight.x = (float)GetScreenWidth() / 2.0f;
	gameApp->HalfWidthHeight.y = (float)GetScreenHeight() / 2.0f;
	gameApp->MapXY.x = (float)CULL_WIDTH / 2.0f;
	gameApp->MapXY.y = (float)CULL_HEIGHT / 2.0f;

	gameApp->Game->WorldCamera.offset = gameApp->MapXY;
	gameApp->Game->ViewCamera.offset = gameApp->HalfWidthHeight;

	game->Renderer.Initialize();
	game->TileMapRenderer.Initialize(game);
	game->LightingRenderer.Initialize(game);

	EntityMgrInitialize();
	InitializeItems(game);
	TileMgrInitialize(&game->Resources.TileSheet);

	game->WorldCamera.zoom = 1.0f;
	game->ViewCamera.zoom = 1.0f;
	gameApp->Scale = 1.0f;

	SLOG_INFO("[ GAME ] Successfully initialized game!");
	return true;
}

internal void 
GameLoad(Game* game, GameApplication* gameApp)
{
	PROFILE_BEGIN();

	LightsInitialize(&game->LightingState);

	UniverseInitialize(&game->Universe, gameApp);
	UniverseLoad(&game->Universe, gameApp);

	PROFILE_END();
}

SAPI void GameApplication::Run()
{
	TraceLog(LOG_INFO, "Game Running...");
	IsRunning = true;

	while (!WindowShouldClose())
	{
		PROFILE_BEGIN_EX("Run");

		double updateWorldStart = GetTime();

		// ***************
		// Frame Setup
		// ***************
		SMemTempReset();

		// ***************
		// Update
		// ***************

		HandleGUIInput(UIState, this);

		// **************************
		// Updates UI logic, draws to
		// screen later in frame
		// **************************
		UpdateUI(UIState, this, Game);

		// *****************
		// Updating
		// *****************

		CTileMap::Update(&Game->Universe.World.ChunkedTileMap, Game);
		Game->TileMapRenderer.Draw();

		GameUpdate(Game, this);

		Rectangle srcRect = { 0.0f, 0.0f, CULL_WIDTH, -CULL_HEIGHT };
		//Rectangle dstRectScreen = { ScreenXY.x, ScreenXY.y, CULL_WIDTH, CULL_HEIGHT };
		Vector2 v = GetScreenToWorld2D({ }, Game->WorldCamera);
		Rectangle dstRectCull = { v.x, v.y, CULL_WIDTH, CULL_HEIGHT };
		//if ((SCREEN_W / TILE_SIZE) % 2 == 0)
		//	dstRectCull.x -= HALF_TILE_SIZE;
		//if ((SCREEN_H / TILE_SIZE) % 2 == 0)
		//	dstRectCull.y -= HALF_TILE_SIZE;
		//Handles odd # tile cull rectangles.


		// Update and draw world
		BeginTextureMode(Game->Renderer.WorldTexture);
		BeginShaderMode(Game->Renderer.UnlitShader);
		BeginMode2D(Game->WorldCamera);

		ClearBackground(BLACK);

		DrawTexturePro(Game->TileMapRenderer.TileMapTexture.texture, srcRect, dstRectCull, { 0 }, 0.0f, WHITE);

		UpdateEntities(Game);
		DrawEntities(Game);
		GameLateUpdate(Game);

		EndMode2D();
		EndShaderMode();
		EndTextureMode();

		UpdateWorldTime = GetTime() - updateWorldStart;

		double drawTime = GetTime();

		// *****************
		// Post Process
		// *****************

		Game->LightingRenderer.Draw(dstRectCull);
		Game->Renderer.PostProcess(Game, Game->Renderer.WorldTexture, Game->LightingRenderer.LightingTexture);

		// ***************
		// Draws to buffer
		// ***************
		BeginDrawing();

		BeginShaderMode(Game->Renderer.LitShader);
		BeginMode2D(Game->ViewCamera);

		ClearBackground(BLACK);

		rlPushMatrix();
		rlScalef(GetScale(), GetScale(), 1.0f);

		Rectangle dst;
		dst.x = Game->ViewCamera.target.x - Game->ViewCamera.offset.x;
		dst.y = Game->ViewCamera.target.y - Game->ViewCamera.offset.y;
		dst.width = GetScreenWidth();
		dst.height = GetScreenHeight();

		SetShaderValueTexture(Game->Renderer.LitShader, Game->Renderer.UniformLightMapLoc, Game->LightingRenderer.LightingTexture.texture);
		DrawTexturePro(Game->Renderer.WorldTexture.texture, srcRect, dst, { 0 }, 0.0f, WHITE);
		Game->Renderer.DrawBloom(dst);

		rlPopMatrix();

		EndMode2D();
		EndShaderMode();

		DrawUI(UIState);

		DrawTime = GetTime() - drawTime;

		// Swap buffers
		double drawStart = GetTime();
		EndDrawing();
		RenderTime = GetTime() - drawStart;

		PROFILE_END();
	}
	IsRunning = false;
}

internal void
GameUpdate(Game* game, GameApplication* gameApp)
{
	PROFILE_BEGIN();

	if (game->IsPlayersTurn)
	{
		// Waits for play to make their move
		if (!gameApp->IsGameInputDisabled)
		{
			HandleGameInput(gameApp, game);
		}
	}
	else
	{
		// Process ai ticks


		game->IsPlayersTurn = true;
		++game->GameTick;
	}

	GameUpdateCamera(game, gameApp);

	UniverseUpdate(&game->Universe, game);
	LightsUpdate(&game->LightingState, game);

	PROFILE_END();
}

internal void GameUpdateCamera(Game* game, GameApplication* gameApp)
{
	PROFILE_BEGIN();

	// Handle Camera Move
	if (!game->IsFreeCam)
	{
		game->CameraLerpTime += GetDeltaTime();
		if (game->CameraLerpTime > 1.0f) game->CameraLerpTime = 1.0f;

		game->WorldCamera.target = Vector2Add(GetClientPlayer()->AsPosition(), { 0, 0 });
		game->ViewCamera.target = Vector2Multiply(game->WorldCamera.target, { GetScale(), GetScale() });
	}

	Vector2 d = Vector2Subtract(game->WorldCamera.target, game->ViewCamera.offset);

	gameApp->ScreenXY = Vector2Subtract(game->WorldCamera.target, gameApp->MapXY);
	gameApp->ScreenXYTiles.x = (int)floorf(gameApp->ScreenXY.x / TILE_SIZE_F);
	gameApp->ScreenXYTiles.y = (int)floorf(gameApp->ScreenXY.y / TILE_SIZE_F);
	gameApp->CullRect.x = d.x;
	gameApp->CullRect.y = d.y;
	gameApp->CullRect.width = CULL_WIDTH;
	gameApp->CullRect.height = CULL_HEIGHT;
	gameApp->CullXYTiles = gameApp->ScreenXYTiles;

	PROFILE_END();
}

internal void
GameLateUpdate(Game* game)
{
	PROFILE_BEGIN();
	WorldLateUpdate(&game->Universe.World, game);
	PROFILE_END();
}

internal void
HandleGameInput(GameApplication* gameApp, Game* game)
{
	// Free Camera Controls
	if (IsKeyPressed(KEY_SLASH))
		game->IsFreeCam = !game->IsFreeCam;

	if (game->IsFreeCam)
	{
		if (IsKeyDown(KEY_L))
			SetCameraPosition(game,
				{ 512.0f * GetDeltaTime(), 0, 0 });
		else if (IsKeyDown(KEY_J))
			SetCameraPosition(game,
				{ -512.0f * GetDeltaTime(), 0, 0 });
		if (IsKeyDown(KEY_K))
			SetCameraPosition(game,
				{ 0, 512.0f * GetDeltaTime(), 0 });
		else if (IsKeyDown(KEY_I))
			SetCameraPosition(game,
				{ 0, -512.0f * GetDeltaTime(), 0 });
	}

	// Camera zoom controls
	float mouseWheel = GetMouseWheelMove();
	if (mouseWheel != 0)
	{
		SetCameraDistance(gameApp, mouseWheel);
	}

	// Debug place tiles
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Vector2i clickedTilePos = GetTileFromMouse(game);
		if (CTileMap::IsTileInBounds(&game->Universe.World.ChunkedTileMap, clickedTilePos))
		{
			TileData* tile = CTileMap::GetTile(&game->Universe.World.ChunkedTileMap, clickedTilePos);
			TileData newTile = TileMgrCreate(TileMgrToTileId(ROCKY_WALL));
			CTileMap::SetTile(&game->Universe.World.ChunkedTileMap, &newTile, clickedTilePos);
			SLOG_INFO("Clicked Tile[%d, %d] Id: %u", clickedTilePos.x, clickedTilePos.y, TileMgrToTileId(tile->AsCoord()));
		}
	}
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		Vector2i clickedTilePos = GetTileFromMouse(game);
		if (CTileMap::IsTileInBounds(&game->Universe.World.ChunkedTileMap, clickedTilePos))
		{
			UpdatingLight light = {};
			light.EntityId = ENT_NOT_FOUND;
			light.Pos = clickedTilePos.AsVec2();
			light.MinIntensity = 7.0f;
			light.MaxIntensity = 9.0f;
			light.Colors[0] = { 0xab, 0x16, 0x0a, 255 };
			light.Colors[1] = { 0x89, 0x12, 0x08, 255 };
			light.Colors[2] = { 0xd6, 0x1b, 0x0c, 255 };
			light.Colors[3] = { 0xbf, 0x05, 0x00, 255 };
			light.Color = light.Colors[0];
			light.Radius = light.MaxIntensity;
			LightAddUpdating(&GetGame()->LightingState, &light);
		}
	}
	if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
	{
		Vector2i clickedTilePos = GetTileFromMouse(game);
		if (CTileMap::IsTileInBounds(&game->Universe.World.ChunkedTileMap, clickedTilePos))
		{
			CTileMap::GetTile(&game->Universe.World.ChunkedTileMap, clickedTilePos)->HasCeiling = true;
		}
	}

	if (IsKeyPressed(KEY_F1))
		game->DebugDisableDarkess = !game->DebugDisableDarkess;
	if (IsKeyPressed(KEY_F2))
		game->DebugDisableFOV = !game->DebugDisableFOV;
	if (IsKeyPressed(KEY_F3))
		game->DebugTileView = !game->DebugTileView;
	if (IsKeyPressed(KEY_F4))
	{
		if (game->ViewCamera.zoom == 1.f)
			game->ViewCamera.zoom = .5f;
		else
			game->ViewCamera.zoom = 1.f;
	}
}

SAPI void GameApplication::Shutdown()
{
	FreeResouces(&Game->Resources);
	Game->Renderer.Free();
	Game->TileMapRenderer.Free();
	Game->LightingRenderer.Free();
	GameUnload(Game, this);
	ExitProfile();

	CloseWindow();
	SMemFree();

	GameAppPtr = nullptr;
}

internal void GameUnload(Game* game, GameApplication* gameApp)
{
	UniverseUnload(&game->Universe, gameApp);
}

GameApplication* GetGameApp()
{
	SASSERT(GameAppPtr);
	SASSERT(GameAppPtr->IsInitialized);
	return GameAppPtr;
}

Game* GetGame()
{
	SASSERT(GetGameApp()->Game);
	return GetGameApp()->Game;
}

Player* GetClientPlayer()
{
	return &GetEntityMgr()->Player;
}

SRandom* GetGlobalRandom()
{
	return &GetGameApp()->GlobalRandom;
}

float GetDeltaTime()
{
	return GetFrameTime();
}

float GetScale()
{
	SASSERT(GetGameApp()->Scale > 0.0f);
	return GetGameApp()->Scale;
}

Vector2 VecToTileCenter(Vector2 vec)
{
	return { vec.x + HALF_TILE_SIZE, vec.y + HALF_TILE_SIZE };
}

Vector2i GetTileFromMouse(Game* game)
{
	Vector2 mouse = GetMousePosition();
	Vector2 mouseWorld = GetScreenToWorld2D(mouse, game->ViewCamera);
	// Scales mouseWorld based on world scale
	//mouseWorld.x = mouseWorld.x / GetScale();
	//mouseWorld.y = mouseWorld.y / GetScale();
	return CTileMap::WorldToTile(mouseWorld);
}

void SetCameraPosition(Game* game, Vector3 pos)
{
	Camera2D& camera = game->ViewCamera;
	camera.target.x += pos.x;
	camera.target.y += pos.y;
}

void SetCameraDistance(GameApplication* gameApp, float zoom)
{
	float ZOOM_MAX = 5.0f;
	float ZOOM_MIN = 1.2f; // Min values are to stop tiles cutting off, maybe we fix some day
	float ZOOM_SPD = .20f;

	float newZoom = gameApp->Scale + zoom * ZOOM_SPD;

	if (newZoom > ZOOM_MAX) newZoom = ZOOM_MAX;
	else if (newZoom < ZOOM_MIN) newZoom = ZOOM_MIN;
	gameApp->Scale = newZoom;

	// NOTE: this is so we dont get weird camera
	// jerking when we scroll
	Vector2 playerPos = VecToTileCenter(GetClientPlayer()->AsPosition());
	gameApp->Game->WorldCamera.target = playerPos;
	Vector2 viewTarget = Vector2Multiply(playerPos, { GetScale(), GetScale() });
	gameApp->Game->ViewCamera.target = viewTarget;
}

bool TileInsideCullRect(Vector2i coord)
{
	Vector2i offset = GetGameApp()->CullXYTiles;
	return (coord.x >= offset.x
		&& coord.y >= offset.y
		&& coord.x < offset.x + CULL_WIDTH_TILES
		&& coord.y < offset.y + CULL_HEIGHT_TILES);
}

Vector2i WorldTileToCullTile(Vector2i coord)
{
	return coord.Subtract(GetGameApp()->CullXYTiles);
}

Vector2i CullTileToWorldTile(Vector2i coord)
{
	return coord.Add(GetGameApp()->CullXYTiles);
}
