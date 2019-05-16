
TdRect GetViewportRect(const Gamer* gamer)
{
	assert(gamer);
	return {0, 0, (int32)gamer->viewport.width, (int32)gamer->viewport.height };
}

void HandleMouseWheel(int16)
{
}

void HandleGamerInput(Gamer* gamer)
{
	assert(gamer);
	auto app_state = GetAppState();
	TdInputState* input = &app_state->input;

	// Set gamepad state
	memcpy(&app_state->prev_gamepad, &input->gamepad, sizeof(TdGamePadState));
	XINPUT_STATE xi_state;
	XInputGetState(0, &xi_state);
	memclear<TdGamePadState>(&input->gamepad);
	ConvertXInput(input->gamepad, app_state->prev_gamepad, xi_state.Gamepad);

	if (gamer->screen_mode == sm_Gameplay)
	{
		Sim* sim = &app_state->sim;
		Player* player = gamer->player;
		assert(player);
	}
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
