#include "Game.h"

#include "Creature.h"
#include "ResourceManager.h"
#include "Lighting.h"
#include "SpriteAtlas.h"
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

internal void GameLoadScreen(GameApplication* gameApp, int width, int height);
internal bool GameInitialize(Game* game, GameApplication* gameApp);
internal void GameLoad(Game* game, GameApplication* gameApp);
internal void GameUnload(Game* game);
internal void GameUpdate(Game* game, GameApplication* gameApp);
internal void GameLateUpdate(Game* game);
internal void GameInputUpdate(Game* game, GameApplication* gameApp);

SAPI bool GameApplication::Start()
{
	if (IsInitialized)
	{
		TraceLog(LOG_ERROR, "GameApplication already initialized!");
		return false;
	}

	double initStart = GetTime();

	size_t gameMemorySize = Megabytes(32);
	size_t tempMemorySize = Megabytes(4);
	SMemInitialize(this, gameMemorySize, tempMemorySize);

	const int screenWidth = 1920;
	const int screenHeight = 1080;
	//SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "Some roguelike game");
	SetTargetFPS(144);
	SetTraceLogLevel(LOG_ALL);

	SRandomInitialize(&GlobalRandom, 0);

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

	GameLoad(Game, this);

	double initEnd = GetTime() - initStart;
	SLOG_INFO("[ Init ] Initialized Success. Took %f second", initEnd);
	return IsInitialized;
}

SAPI void GameApplication::Shutdown()
{
	GameAppPtr = nullptr;
	FreeResouces(&Game->Resources);
	Game->Renderer.Free();
	CloseWindow();
}

internal void GameLoadScreen(GameApplication* gameApp, int width, int height)
{
	gameApp->HalfWidthHeight.x = (float)width / 2.0f;
	gameApp->HalfWidthHeight.y = (float)height / 2.0f;
	gameApp->Game->WorldCamera.offset = gameApp->HalfWidthHeight;
	gameApp->Game->ViewCamera.offset = gameApp->HalfWidthHeight;
	
	gameApp->Game->Renderer.Free();
	gameApp->Game->Renderer.Initialize();
}

internal bool GameInitialize(Game* game, GameApplication* gameApp)
{
	// NOTE: Used for now. I use stl structures and since Game is malloced
	// constructors do not get called, messing up these. No ideal,
	// but seems to work.
	CALL_CONSTRUCTOR(game) Game();

	game->Renderer.Initialize();

	bool didResInit = InitializeResources(&game->Resources);
	SASSERT(didResInit);

	GameLoadScreen(gameApp, GetScreenWidth(), GetScreenHeight());

	LightMapInitialize(&game->LightMap);
	TileMgrInitialize(&game->TileMgr, &game->Resources.Atlas);
	EntityMgrInitialize(game);

	game->WorldCamera.zoom = 1.0f;
	game->ViewCamera.zoom = 1.0f;
	gameApp->Scale = 1.0f;

	SLOG_INFO("[ GAME ] Successfully initialized game!");
	return true;
}

internal void GameLoad(Game* game, GameApplication* gameApp)
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

internal void GameUnload(Game* game)
{
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
		if (IsSuspended) continue;

		// ***************
		// Frame Setup
		// ***************
		SMemTempReset();

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
						{ 512.0f * GetDeltaTime(), 0, 0 });
				else if (IsKeyDown(KEY_J))
					SetCameraPosition(Game,
						{ -512.0f * GetDeltaTime(), 0, 0 });
				if (IsKeyDown(KEY_K))
					SetCameraPosition(Game,
						{ 0, 512.0f * GetDeltaTime(), 0 });
				else if (IsKeyDown(KEY_I))
					SetCameraPosition(Game,
						{ 0, -512.0f * GetDeltaTime(), 0 });
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
					UpdatingLight light = {};
					light.Pos = clickedTilePos.AsVec2();
					light.Intensity = 1.0f;
					light.Radius = 16.0f;

					light.MinIntensity = 14.0f;
					light.MaxIntensity = 16.0f;
					//light.Colors[0] = { 0xfb, 0xbe, 0x46, 255 };
					//light.Colors[1] = { 0xc7, 0x46, 0x07, 255 };
					//light.Colors[2] = { 0x6e, 0x15, 0x05, 255 };
					//light.Colors[3] = { 0xf9, 0x92, 0x20, 255 };
					light.Colors[0] = { 0xab, 0x16, 0x0a, 255 };
					light.Colors[1] = { 0x89, 0x12, 0x08, 255 };
					light.Colors[2] = { 0xd6, 0x1b, 0x0c, 255 };
					light.Colors[3] = { 0xbf, 0x05, 0x00, 255 };
					//Color c = ColorFromNormalized({ 1.0, 0.8, 0.2, 1.0f });
					/*light.Colors[0] = c;
					light.Colors[1] = c;
					light.Colors[2] = c;
					light.Colors[3] = c;*/
					light.Color = light.Colors[0];
					LightsAddUpdating(light);
				}
			}
			if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
			{
				Vector2i clickedTilePos = GetTileFromMouse(Game);
				if (CTileMap::IsTileInBounds(&Game->World.ChunkedTileMap, clickedTilePos))
				{
					Tile* tile = CTileMap::GetTile(&Game->World.ChunkedTileMap, clickedTilePos);
					tile->HasCeiling = true;
				}
			}

			if (IsKeyPressed(KEY_ONE))
			{
				Action testAction = {};
				testAction.Cost = 100;
				testAction.ActionFunction = TestActionFunc;
				AddAction(&Game->World, &testAction);
			}

			if (IsKeyPressed(KEY_TWO))
			{
				Game->DebugTest = !Game->DebugTest;
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
		// TODO maybe add a non drawing preupdate?

		// *****************
		// Drawing
		// *****************

		float screenW = (float)GetScreenWidth();
		float screenH = (float)GetScreenHeight();
		Rectangle srcRect = { 0.0f, 0.0f, screenW, -screenH };
		Rectangle dstRect = { ScreenXY.x, ScreenXY.y, screenW, screenH };
		Rectangle screenRect = { 0.0f, 0.0f, screenW, screenH };

		// Update and draw world
		BeginShaderMode(Game->Renderer.UnlitShader);
		BeginTextureMode(Game->Renderer.WorldTexture);
		BeginMode2D(Game->WorldCamera);
		ClearBackground(BLACK);
		double updateWorldStart = GetTime();

		GameUpdate(Game, this);
		GameLateUpdate(Game);

		UpdateWorldTime = GetTime() - updateWorldStart;
		EndMode2D();
		EndTextureMode();
		EndShaderMode();

		BeginShaderMode(Game->Renderer.LightingShader);
		BeginTextureMode(Game->Renderer.EffectTextureOne);
		BeginMode2D(Game->WorldCamera);
		ClearBackground(BLACK);
		LightMapUpdate(&Game->LightMap, Game);
		EndMode2D();
		EndTextureMode();
		EndShaderMode();

		// Brightness pass
		BeginShaderMode(Game->Renderer.BrightnessShader);
		BeginTextureMode(Game->Renderer.EffectTextureTwo);
		ClearBackground(BLACK);
		DrawTexturePro(Game->Renderer.EffectTextureOne.texture, srcRect, screenRect, { 0 }, 0.f, WHITE);
		EndTextureMode();
		EndShaderMode();
		
		// Blur pass
		Game->Renderer.BlurShader.Draw(Game->Renderer.EffectTextureTwo.texture);

		// ***************
		// Draws to buffer
		// ***************
		BeginDrawing();

		BeginShaderMode(Game->Renderer.LitShader);
		BeginMode2D(Game->ViewCamera);

		rlPushMatrix();
		rlScalef(GetScale(), GetScale(), 1.0f);

		SetShaderValueTexture(Game->Renderer.LitShader, Game->Renderer.UniformLightMapLoc, Game->Renderer.EffectTextureOne.texture);
		DrawTexturePro(Game->Renderer.WorldTexture.texture, srcRect, dstRect, { 0 }, 0.0f, WHITE);
		
		// Enable brighter bright
		if (Game->DebugTest)
		{
			BeginBlendMode(BLEND_ADDITIVE);
			DrawTexturePro(Game->Renderer.EffectTextureTwo.texture, srcRect, dstRect, { 0 }, 0.0f, WHITE);
			EndBlendMode();
		}

		//BeginBlendMode(BLEND_ADDITIVE);
		//Rectangle r = { 0.f, 0.f, (float)Game->Resources.Blur.TextureVert.texture.width, -(float)Game->Resources.Blur.TextureVert.texture.height };
		//DrawTexturePro(Game->Resources.Blur.TextureVert.texture, r, dstRect, { 0 }, 0.0f, WHITE);
		//EndBlendMode();

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


	Vector2i playerPos = GetClientPlayer()->Transform.TilePos;
	CTileMap::SetVisible(&game->World.ChunkedTileMap, playerPos);
	for (int i = 0; i < sizeof(Vec2i_NEIGHTBORS); ++i)
	{
		Vector2i pos = Vec2i_NEIGHTBORS[i].Add(playerPos);
		CTileMap::SetVisible(&game->World.ChunkedTileMap, pos);
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
	SASSERT(GameAppPtr);
	SASSERT(GameAppPtr->IsInitialized);
	return GameAppPtr;
}

Player* const GetClientPlayer()
{
	SASSERT(GetGame()->EntityMgr.Players.Count > 0);
	return GetGame()->EntityMgr.Players.PeekAt(0);
}

EntityMgr* GetEntityMgr()
{
	SASSERT(GetGame()->World.IsAllocated);
	return &GetGame()->EntityMgr;
}

Game* const GetGame()
{
	SASSERT(GetGameApp()->Game);
	return GetGameApp()->Game;
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
