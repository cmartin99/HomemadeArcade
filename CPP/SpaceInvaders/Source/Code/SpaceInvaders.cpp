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

char *packet_type_names[] = {"None", "CurrentSimState",	"SimState",	"ClientState", "Message", "ConnectRequest",	"ConnectConfirm", "PlayRequest", "PlayConfirm", };

AppMemory* memory = nullptr;
double auto_save_timer = 0;

void LogError(char* s)
{
	auto app_state = GetAppState();

	char *e = app_state->log_error_text + app_state->curr_log_error * app_state->max_log_error_text_len;
	s[app_state->max_log_error_text_len - 1] = '\0';
	strncpy(e, s, app_state->max_log_error_text_len);
	e[app_state->max_log_error_text_len - 1] = 0;

	++app_state->curr_log_error;
	if (app_state->curr_log_error == app_state->max_log_errors) app_state->curr_log_error = 0;
}

void LogError1(char* s)
{
	assert(s);
	int32 len = strlen(s);
	assert(len > 0);

	auto app_state = GetAppState();
	char *e = app_state->log_error_text + app_state->curr_log_error * app_state->max_log_error_text_len;
	strcpy(e, s);

	++app_state->curr_log_error;
	if (app_state->curr_log_error == app_state->max_log_errors) app_state->curr_log_error = 0;
}

void RunOneFrame()
{
	if (memory)
	{
		auto app_state = GetAppState();
		app_state->scratch_arena.used = 0;
		app_state->bytes_sent_per_frame = 0;
		app_state->bytes_recv_per_frame = 0;

		Sim *sim = &app_state->sim;
		if (sim->is_active)
		{
			ReadSockets();

			if (!sim->is_paused)
			{
				sim->seconds = app_state->seconds * sim->sim_speed;
				sim->total_seconds += sim->seconds;
				UpdateSim(sim);
			}

			if (app_state->total_seconds >= sim->next_socket_write_time)
			{
				WriteSockets(&WriteCurrentSimStateToPacket);
				
				sim->next_socket_write_time = app_state->total_seconds + g_write_socket_freq;
			    if (sim->agar_changed_socket_buffer.count > sim->agar_changed_largest_count) sim->agar_changed_largest_count = sim->agar_changed_socket_buffer.count;
			    if (sim->shots_changed.count > sim->shots_changed_largest_count) sim->shots_changed_largest_count = sim->shots_changed.count;
			    sim->agar_changed_socket_buffer.count = 0; ///TODO not thread safe
			    sim->shots_changed.count = 0;
			    sim->viruses_changed.count = 0;
			    sim->spawners_changed.count = 0;
			}

			app_state->bytes_sent_per_second += app_state->bytes_sent_per_frame;
			app_state->bytes_recv_per_second += app_state->bytes_recv_per_frame;
		}

		if (app_state->bytes_net_timer <= app_state->total_seconds)
		{
			app_state->bytes_sent_last_second = app_state->bytes_sent_per_second;
			app_state->bytes_recv_last_second = app_state->bytes_recv_per_second;
			app_state->bytes_sent_per_second = 0;
			app_state->bytes_recv_per_second = 0;
			app_state->bytes_net_timer = app_state->total_seconds + 1;
		}

		RenderGame();
	}
}

#include "ImGuiApp.cpp"
#include "Sim.cpp"
#include "Bot.cpp"
#include "Server.cpp"
#include "GameRender.cpp"
#include "Network.cpp"

}
