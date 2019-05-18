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

#include "ImGuiApp.cpp"
#include "Sim.cpp"
#include "Gamer.cpp"
#include "GameRender.cpp"

void RunOneFrame()
{
	if (memory)
	{
		auto app_state = GetAppState();
		app_state->scratch_arena.used = 0;

		Sim *sim = &app_state->sim;
		if (sim->is_active)
		{
			assert(app_state->gamers.count > 0);
			HandleGamerInput(app_state->gamers.ptr);

			if (!sim->is_paused)
			{
				sim->seconds = app_state->seconds * sim->sim_speed;
				sim->total_seconds += sim->seconds;
				UpdateSim(sim);
			}
		}

		RenderGame();
	}
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

}
