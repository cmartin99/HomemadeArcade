// todo's
// move global tables into array of structs - e.g. xml reading

#include <stdio.h>
#include <winsock2.h>
#include "vulkan.h"
#include "ForwardDecls.h"
#include "glm/gtx/intersect.hpp"

using namespace glm;
using namespace eng;

namespace NewGame {

AppMemory* memory = nullptr;
double auto_save_timer = 0;

void RunOneFrame()
{
	if (memory)
	{
		auto app_state = GetAppState();
		app_state->scratch_arena.used = 0;

		Sim *sim = &app_state->sim;
		if (sim->is_active)
		{
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

#include "ImGuiApp.cpp"
#include "Sim.cpp"
#include "Server.cpp"
#include "GameRender.cpp"

}
