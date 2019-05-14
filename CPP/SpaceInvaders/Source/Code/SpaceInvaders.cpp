// todo's
// move global tables into array of structs - e.g. xml reading

#include <stdio.h>
#include "vulkan.h"
#include "ForwardDecls.h"
#include "glm/gtx/intersect.hpp"

using namespace glm;
using namespace eng;

namespace SpaceInvaders {

AppMemory* memory = nullptr;

void RunOneFrame()
{
	if (memory)
	{
		auto app_state = GetAppState();
		app_state->scratch_arena.used = 0;

		Sim *sim = &app_state->sim;
		if (sim->is_active && !sim->is_paused)
		{
			sim->seconds = app_state->seconds * sim->sim_speed;
			sim->total_seconds += sim->seconds;
			UpdateSim(sim);
		}

		RenderGame();
	}
}

void HandleMouseWheel(int16)
{
}

Gamer* GamerNew()
{
	auto app_state = GetAppState();
	Gamer* gamer = app_state->gamers.ptr;
	app_state->gamers.count = 1;
	memclear(gamer);
	gamer->gamer_id = (uint64)(gamer - app_state->gamers.ptr + 1);

	Renderer* renderer = &app_state->renderer;
	TdVkInstance* vulkan = renderer->vulkan;

	gamer->viewport = {};
	gamer->viewport.width = (float)vulkan->surface_width;
	gamer->viewport.height = (float)vulkan->surface_height;
	gamer->viewport.minDepth = (float) 0.0f;
	gamer->viewport.maxDepth = (float) 1.0f;

	gamer->scissor_rect = {};
	gamer->scissor_rect.extent.width = vulkan->surface_width;
	gamer->scissor_rect.extent.height = vulkan->surface_height;
	gamer->scissor_rect.offset.x = 0;
	gamer->scissor_rect.offset.y = 0;

	gamer->gui = tdMalloc<TdImGui>(app_state->perm_arena);
	tdImGuiInit(gamer->gui);

	gamer->gui->sprite_batch = renderer->gui_sprite_batch;
	gamer->gui->input = &app_state->input;
	gamer->gui->GuiText = &GuiText;
	gamer->gui->GuiTipText = &GuiTipText;
	gamer->gui->text_offset.x = -1;
	gamer->gui->text_offset.y = 1;
	gamer->gui->button_border_size = 1;

	return gamer;
}

Player* PlayerNew(Sim* sim, Gamer* gamer)
{
	Player* player = sim->players.ptr;
	sim->players.count = 1;

	memclear<Player>(player);
	player->type = 1;
	player->gamer = gamer;
	if (gamer) gamer->player = player;

	return player;
}

void RendererNew(TdVkInstance* vulkan)
{
	auto app_state = GetAppState();
	Renderer* renderer = &app_state->renderer;
	renderer->vulkan = vulkan;

	renderer->sprite_batch = tdMalloc<TdSpriteBatch>(app_state->perm_arena);
	tdVkSpriteBatchInit(vulkan, renderer->sprite_batch, 2000000);
	renderer->gui_sprite_batch = tdMalloc<TdSpriteBatch>(app_state->perm_arena);
	tdVkSpriteBatchInit(vulkan, renderer->gui_sprite_batch, 100000);
}

void InitApplication(AppMemory& _memory, TdVkInstance* vulkan)
{
	memory = &_memory;
	auto app_state = GetAppState();
	tdMemoryArenaInit(app_state->perm_arena, memory->perm_ram_size - sizeof(AppState), (uint8*)memory->perm_ram + sizeof(AppState));
	tdMemoryArenaInit(app_state->main_arena, memory->main_ram_size, memory->main_ram);
	tdMemoryArenaInit(app_state->scratch_arena, memory->scratch_ram_size, memory->scratch_ram);
	tdArrayInit(app_state->gamers, app_state->perm_arena, 1);

	app_state->debug_data.debug_verbosity = 0;
	app_state->rng_time = time(nullptr);
	srand((unsigned)app_state->rng_time);
	app_state->rng_seed = rand();
	app_state->rng_inc = rand();
	tdRandomSeed(&app_state->rng, app_state->rng_seed, app_state->rng_inc);

	RendererNew(vulkan);
	GamerNew();
}

void CloseApplication()
{
	if (memory)
	{
		GetAppState()->exit_app = true;
	}
}

#include "ImGuiApp.cpp"
#include "Sim.cpp"
#include "GameRender.cpp"

}
