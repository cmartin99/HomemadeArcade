
#include <stdlib.h>
#include <chrono>
#include "vulkan.h"
#include "TdEngine.h"

using namespace glm;
using namespace eng;

#include "TdRandom.h"
#include "TdPoint.h"
#include "TdJSonParser.h"
#include "DataTypes.h"

namespace SpaceInvaders {

static GameMemory* memory;
static char temp_text[1000];
static char commas1[20];
static char commas2[20];
static char commas3[20];
static char commas4[20];
static char commas5[20];
static char commas6[20];
static char commas7[20];
static char commas8[20];
static char commas9[20];
static char commas10[20];
static char commas11[20];
static char commas12[20];

const TdPoint2 GameConsts::invader_size = {64, 56};
const TdPoint2 GameConsts::invader_spacing = {90, 72};
const float GameConsts::invader_fleet_creep_speed = 20;
const float GameConsts::invader_fleet_creep_distance = 20;
const Vector2 GameConsts::invader_bomb_speed = {0, 250};
const float GameConsts::ufo_speed = 300;
const TdPoint2 GameConsts::ufo_size = {88, 32};
const Vector2 GameConsts::ufo_bomb_speed = {0, 750};
const TdPoint2 GameConsts::defender_size = {72, 48};
const Vector2 GameConsts::defender_speed = {200, 0};
const double GameConsts::defender_respawn_time = 2.0;
const TdPoint2 GameConsts::fleet_size = {12, 6};
const TdPoint2 GameConsts::bullet_size = {4, 24};
const Vector2 GameConsts::bullet_speed = {0, 1500};

extern void ExitGame();
typedef void (*InputEventHandler)();
void RenderDebug(TdWindow*, TdSpriteBatch*, TdPoint2);
const char *GuiText(GuiID);
void ConvertXInput(TdGamePadState&, const TdGamePadState&, XINPUT_GAMEPAD);

template<typename T>
ALWAYS_INLINE void memclear(T* p)
{
    memset(p, 0, sizeof(T));
}

bool IsGameInitialized()
{
    return memory != nullptr && memory->perm_ram != nullptr;
}

ALWAYS_INLINE GameState* GetGameState()
{
    return (GameState*)memory->perm_ram;
}

void InitPlatform(void* game_memory)
{
	assert(game_memory);
	memory = (GameMemory*)game_memory;
}

TdInputState* GetInput()
{
	return &((GameState*)memory->perm_ram)->input;
}

ALWAYS_INLINE double SetTimer(double base, double time)
{
	return base + time;
}

TdRect GetViewportSize(uint32 player_id, uint32 player_count)
{
	auto game_state = GetGameState();

	int w = player_count == 1
		? game_state->vulkan->surface_width
		: player_count < 5
				? game_state->vulkan->surface_width / 2
				: game_state->vulkan->surface_width / 4;

	int h = player_count == 1
		? game_state->vulkan->surface_height
		: player_count < 5
				? game_state->vulkan->surface_height / 2
				: game_state->vulkan->surface_height / 4;

	int x = player_count == 1
				? 0
				: player_count < 5
					  ? w * (player_id % 2)
					  : w * (player_id % 4);

	int y = player_count == 1
				? 0
				: player_count < 5
					? h * (int)(player_id / 2)
					: h * (int)(player_id / 4);

	return {x, y, w, h};
}

#include "GameInstance.cpp"
#include "GameInput.cpp"
#include "GameUpdate.cpp"
#include "GameRender.cpp"
#include "ImGuiApp.cpp"

void RunOneFrame(double delta_secs)
{
	if (IsGameInitialized())
	{
		auto game_state = GetGameState();
		eng::total_seconds += delta_secs;
		game_state->seconds = delta_secs;
		game_state->total_seconds += delta_secs;
		game_state->scratch_arena.used = 0;

		HandleInput();
		UpdateFrame();
		RenderFrame();
	}
}

void PlayerLocalNew(Player* player)
{
	assert(player);
	//assert(player_count > 0 && player_count < 17);
	//assert(player_id >= 0 && player_id < player_count);

	auto game_state = GetGameState();
	memclear<Player>(player);
	player->mode = Player::pm_Menu;

	TdRect vp = GetViewportSize(0, 1);//player_id, player_count);
	player->viewport = {};
	player->viewport.x = vp.x;
	player->viewport.y = vp.y;
	player->viewport.width = vp.w;
	player->viewport.height = vp.h;
	player->viewport.minDepth = (float)0.0f;
	player->viewport.maxDepth = (float)1.0f;

	player->scissor_rect = {};
	player->scissor_rect.extent.width = player->viewport.width;
	player->scissor_rect.extent.height = player->viewport.height;
	player->scissor_rect.offset.x = player->viewport.x;
	player->scissor_rect.offset.y = player->viewport.y;

	player->gui.input = &game_state->input;
	player->gui.sprite_batch = game_state->gui_sprite_batch;
	player->gui.GuiText = &GuiText;
}

void ApplicationNew(TdVkInstance& vulkan)
{
	auto game_state = GetGameState();
	tdMemoryArenaInit(game_state->perm_arena, memory->perm_ram_size - sizeof(GameState), (uint8*)memory->perm_ram + sizeof(GameState));
	tdMemoryArenaInit(game_state->main_arena, memory->main_ram_size, memory->main_ram);
	tdMemoryArenaInit(game_state->scratch_arena, memory->scratch_ram_size, memory->scratch_ram);

	game_state->vulkan = &vulkan;
	game_state->debug_data.debug_verbosity = 0;
	srand((unsigned)time(nullptr));
	game_state->rng = tdMalloc<TdPcg32Random>(game_state->perm_arena);
	tdRandomSeed(game_state->rng, rand(), rand());

	game_state->sprite_batch = tdMalloc<TdSpriteBatch>(game_state->perm_arena);
	tdVkSpriteBatchInit(*game_state->sprite_batch, *game_state->vulkan, 2000000);
	game_state->gui_sprite_batch = tdMalloc<TdSpriteBatch>(game_state->perm_arena);
	tdVkSpriteBatchInit(*game_state->gui_sprite_batch, *game_state->vulkan, 100000);

	tdVkLoadTexture(*game_state->vulkan, "content/textures/spritesheet.ktx", VK_FORMAT_R8G8B8A8_UNORM, game_state->sprite_sheet, false);

	LARGE_INTEGER currentTime;
	currentTime.QuadPart = 5;
	QueryPerformanceCounter(&currentTime);
	game_state->elapsed_scale = 1;

	game_state->instance = tdMalloc<GameInstance>(game_state->perm_arena);
	game_state->player = tdMalloc<Player>(game_state->perm_arena);
	PlayerLocalNew(game_state->player);//, 0, 1);
}

void ApplicationFree()
{
}

}
