
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

#include "Common.cpp"
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
