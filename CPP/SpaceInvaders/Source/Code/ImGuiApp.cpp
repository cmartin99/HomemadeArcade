
struct Gui
{
	enum Item
	{
		None,
	};
};

const char* gui_text[] = {
	nullptr,
};

const char* gui_tip_text[] = {
	nullptr, //None,
};

const char* GuiText(TdImGuiContext context)
{ 
	return gui_text[context.id]; 
}

const char* GuiTipText(TdImGuiContext context)
{ 
	return nullptr;//gui_tip_text[context.id]; 
}

void ImGuiUpdateServer(Renderer* renderer)
{
	assert(renderer);
	auto app_state = GetAppState();
	Sim* sim = &app_state->sim;
	TdImGui* gui = renderer->gui;
	gui->button_text_scale = 0.6f;
	gui->tooltip_text_scale = 0.5f;

	TdRect rect;
	rect.w = 28;
	rect.x = 4;
	rect.y = 4 + last_debug_y;
	rect.h = 28;

	if (DoButton(gui, Gui::StartSim, rect, !sim->is_active))
	{
		SimNew();
	}
	rect.x += rect.w + 4;
	if (sim->is_active)
	{
		if (DoButton(gui, Gui::EndSim, rect))
		{
			SimEnd();
			return;
		}
		rect.x += rect.w + 4;
		if (!sim->is_paused)
		{
			if (DoButton(gui, Gui::PauseSim, rect))
			{
				sim->is_paused = true;
			}
		}
		else
		{
			if (DoButton(gui, Gui::ResumeSim, rect))
			{
				sim->is_paused = false;
			}
		}
		rect.x += rect.w + 4;
		rect.w += 6;
		if (DoButton(gui, Gui::SlowSimSpeed, rect))
		{
			sim->sim_speed *= 0.5;
		}
		rect.x += rect.w + 4;
		rect.w += 38;
		if (DoButton(gui, {Gui::ResetSimSpeed, sim}, rect))
		{
			sim->sim_speed = 1;
		}
		rect.x += rect.w + 4;
		rect.w -= 38;
		if (DoButton(gui, Gui::FastSimSpeed, rect))
		{
			sim->sim_speed *= 2.0;
		}
		rect.x += rect.w + 4;
		if (DoButton(gui, Gui::AddBot1, rect))
		{
			AddBot(sim);
		}
		rect.x += rect.w + 4;
		if (DoButton(gui, Gui::RemoveBot1, rect))
		{
			RemoveBot(sim);
		}
		rect.x += rect.w + 4;
		rect.w += 18;
		if (DoButton(gui, Gui::AddBot10, rect))
		{
			for (int i = 0; i < 10; ++i) AddBot(sim);
		}
		rect.x += rect.w + 4;
		if (DoButton(gui, Gui::RemoveBot10, rect))
		{
			for (int i = 0; i < 10; ++i) RemoveBot(sim);
		}
		rect.x += rect.w + 4;
		rect.w += 10;
		if (DoButton(gui, Gui::AddBot100, rect))
		{
			for (int i = 0; i < 100; ++i) AddBot(sim);
		}
		rect.x += rect.w + 4;

		rect.x = 4;
		rect.y += 4 + rect.h;

		if (DoButton(gui, Gui::RemoveBot100, rect))
		{
			for (int i = 0; i < 100; ++i) RemoveBot(sim);
		}
		rect.x += rect.w + 4;
		rect.w -= 28;
		if (DoButton(gui, Gui::LessRndLatency, rect))
		{
			sim->debug_rnd_latency -= 20;
			if (sim->debug_rnd_latency < 0) sim->debug_rnd_latency = 0;
		}
		rect.x += rect.w + 4;
		rect.w += 30;
		if (DoButton(gui, {Gui::ResetRndLatency, sim}, rect))
		{
			sim->debug_rnd_latency = 0;
		}
		rect.x += rect.w + 4;
		rect.w -= 30;
		if (DoButton(gui, Gui::MoreRndLatency, rect))
		{
			sim->debug_rnd_latency += 20;
		}
		rect.x += rect.w + 4;
		if (DoButton(gui, Gui::DecWorldSize, rect))
		{
			sim->world_size -= 64;
		}
		rect.x += rect.w + 4;
		rect.w += 50;
		if (DoButton(gui, {Gui::ResetWorldSize, sim}, rect))
		{
			sim->world_size = max(sim->max_world_size, (int)sqrt(sim->player_count * 65000 / g_PI));
		}
		rect.x += rect.w + 4;
		rect.w -= 50;
		if (DoButton(gui, Gui::IncWorldSize, rect))
		{
			sim->world_size += 64;
		}
		rect.x += rect.w + 4;
		rect.w -= 6;
	}
	if (DoButton(gui, Gui::Debug, rect))
	{
		if (++app_state->debug_data.debug_verbosity > 2)
			app_state->debug_data.debug_verbosity = 0;
	}
	rect.x += rect.w + 4;
	if (DoButton(gui, Gui::CloseServer, rect))
	{
		app_state->exit_app = true;
	}

	renderer->log_offset_y = rect.y + rect.h + 4;	
}

void ImGuiUpdate(Renderer* renderer)
{
	assert(renderer);
	TIMED_BLOCK(ImGuiUpdate);
	renderer->gui->new_hot = Gui::None;

	ImGuiUpdateServer(renderer);

	renderer->gui->hot = renderer->gui->new_hot;
	DoToolTip(renderer->gui);
}