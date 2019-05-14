
void HandleMouseWheel(int16)
{
}

Gamer* GamerNew(Sim* sim)
{
	auto app_state = GetAppState();
	Gamer* gamer = nullptr;

	for (int i = 0; i < sim->gamers.count; ++i)
	{
		if (sim->gamers[i].gamer_id == 0)
		{
			gamer = sim->gamers.ptr + i;
			break;
		}
	}

	if (gamer == nullptr && sim->gamers.count < sim->gamers.cap)
		gamer = tdArrayPush(sim->gamers);

	if (gamer)
	{
		memclear(gamer);
		gamer->gamer_id = (int16)(gamer - sim->gamers.ptr + 1);
	}

	return gamer;
}

Player* PlayerNew(Sim* sim, Gamer* gamer, Player* player)
{
	if (!player)
	{
		if (sim->players.count == sim->players.cap)
		{
			for (int i = 0; i < sim->players.count; ++i)
			{
				if (sim->players[i].type == 0)
				{
					player = sim->players.ptr + i;
					break;
				}
			}
			assert(player);
		}
		else
			player = tdArrayPush(sim->players);
	}

	memclear<Player>(player);
	player->type = 2;
	player->gamer = gamer;
	if (gamer) gamer->player = player;

	int32 player_id = player - sim->players.ptr;
    sprintf(sim->player_name_text + player_id * 21, "Player%d", player_id + 1);

	Entity* cell = FirstPlayerCell(sim, player);
	memclear<Entity>(cell, sim->max_player_cells);
	cell->state = 2;
	cell->pos = FindPlayerCellSpawnPos(sim);
	//assert(length(cell->pos) < sim->world_size / 2 + 20);
	MassSet(cell, 245);

	player->center = cell->pos;
	assert(!isinf(player->center.x));
	assert(!isinf(player->center.y));
	player->extent = { cell->pos.x, cell->pos.y, cell->pos.x, cell->pos.y };
	player->state_data.mouse_pos_x = (int16)cell->pos.x;
	player->state_data.mouse_pos_y = (int16)cell->pos.y;

	sim->player_cells.count = max((int)sim->player_cells.count, (int)(cell - sim->player_cells.ptr + sim->max_player_cells));
	++sim->player_count;

	memclear<BotData>(BotDataFromPlayer(sim, player));

	return player;
}
/*
DWORD WINAPI Thread1Execute(LPVOID lp)
{
	World *world = (World *)lp;
	++world->threads_running;
	while (world->active)
	{
		if (!world->sim_paused)
		{
			UpdateEntityAgarCollision(world);
			UpdateBotAI(world);
		}
		else
			Sleep(1);
	}
	--world->threads_running;
	return 0;
}
*/

void RendererNew(TdVkInstance* vulkan)
{
	auto app_state = GetAppState();
	Renderer* renderer = &app_state->renderer;
	renderer->vulkan = vulkan;

	renderer->sprite_batch = tdMalloc<TdSpriteBatch>(app_state->perm_arena);
	tdVkSpriteBatchInit(vulkan, renderer->sprite_batch, 2000000);
	renderer->gui_sprite_batch = tdMalloc<TdSpriteBatch>(app_state->perm_arena);
	tdVkSpriteBatchInit(vulkan, renderer->gui_sprite_batch, 100000);

	renderer->viewport = {};
	renderer->viewport.width = (float)vulkan->surface_width;
	renderer->viewport.height = (float)vulkan->surface_height;
	renderer->viewport.minDepth = (float) 0.0f;
	renderer->viewport.maxDepth = (float) 1.0f;

	renderer->scissor_rect = {};
	renderer->scissor_rect.extent.width = vulkan->surface_width;
	renderer->scissor_rect.extent.height = vulkan->surface_height;
	renderer->scissor_rect.offset.x = 0;
	renderer->scissor_rect.offset.y = 0;

	renderer->gui = tdMalloc<TdImGui>(app_state->perm_arena);
	tdImGuiInit(renderer->gui);

	renderer->gui->sprite_batch = renderer->gui_sprite_batch;
	renderer->gui->input = &app_state->input;
	renderer->gui->GuiText = &GuiText;
	renderer->gui->GuiTipText = &GuiTipText;
	renderer->gui->text_offset.x = -1;
	renderer->gui->text_offset.y = 1;
	renderer->gui->button_border_size = 1;
}

void InitApplication(AppMemory& _memory, TdVkInstance* vulkan, char* ip_address, uint16 port)
{
	memory = &_memory;
	auto app_state = GetAppState();
	tdMemoryArenaInit(app_state->perm_arena, memory->perm_ram_size - sizeof(AppState), (uint8*)memory->perm_ram + sizeof(AppState));
	tdMemoryArenaInit(app_state->main_arena, memory->main_ram_size, memory->main_ram);
	tdMemoryArenaInit(app_state->scratch_arena, memory->scratch_ram_size, memory->scratch_ram);

	RendererNew(vulkan);
	
	app_state->debug_data.debug_verbosity = 0;
	app_state->rng_time = time(nullptr);
	srand((unsigned)app_state->rng_time);
	app_state->rng_seed = rand();
	app_state->rng_inc = rand();
	tdRandomSeed(&app_state->rng, app_state->rng_seed, app_state->rng_inc);

	app_state->packet_size = 300000;
	app_state->packet = tdMalloc<uint8>(app_state->perm_arena, app_state->packet_size);

	app_state->curr_log_error = 0;
	app_state->max_log_error_text_len = 256;
	app_state->max_log_errors = 60;
	app_state->log_error_text = tdMalloc<char>(app_state->perm_arena, app_state->max_log_error_text_len * app_state->max_log_errors);

	app_state->port = port;
	assert(strlen(ip_address) < 16);
	strcpy_safe(app_state->ip_address, ip_address);
	assert(CreateServerSocket() == 0);
}

void CloseApplication()
{
	WSACleanup();
}
