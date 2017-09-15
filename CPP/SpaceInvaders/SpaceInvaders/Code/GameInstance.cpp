
void PlayerFireBullet()
{
	auto game_state = GetGameState();
	GameInstance *instance = game_state->instance;
	Player* player = game_state->player;

	Bullet *bullet = instance->bullets;
	Bullet *bullet_end = bullet + instance->bullet_count;
	while (bullet < bullet_end)
	{
		if (!bullet->alive)
		{
			bullet->alive = 1;
			bullet->pos.x = instance->ship->pos.x + GameConsts::defender_size.x / 2 - GameConsts::bullet_size.x / 2;
			bullet->pos.y = instance->ship->pos.y - GameConsts::bullet_size.y;
			break;
		}
		++bullet;
	}
}

void NewInvaderWave(GameInstance* instance)
{
	instance->invader_fleet_speed = 100;
	instance->invader_fleet_pos.x = 100;
	instance->invader_fleet_pos.y = 120;
	instance->invader_fleet_extent = {FLT_MAX, -FLT_MAX};

	for (uint32 i = 0; i < instance->invader_count; ++i)
		instance->invader_fleet[i].alive = 1;
}

void GameNew(Player* player)
{
	auto game_state = GetGameState();

	game_state->main_arena.used = 0;
	memset(game_state->main_arena.base, 0, game_state->main_arena.size);

	GameInstance *instance = game_state->instance;
	instance->ship = tdMalloc<DefenderShip>(game_state->main_arena);
	instance->invader_count = GameConsts::wave_size.x * GameConsts::wave_size.y;
	instance->invader_alive_count = 0;
	instance->invader_fleet = tdMalloc<Invader>(game_state->main_arena, instance->invader_count);
	instance->ufo = tdMalloc<UFO>(game_state->main_arena);
	instance->bullet_count = 10;
	instance->bullets = tdMalloc<Bullet>(game_state->main_arena, instance->bullet_count);

	player->mode = Player::pm_Play;
	player->lives = 2;
	instance->ship->pos.x = 100;
	instance->ship->pos.y = player->viewport.height - GameConsts::defender_size.y * 2;

	instance->ufo->pos.x = -GameConsts::ufo_size.x - 1;
	instance->ufo->pos.y = 30;
}

void GameInstanceNew(TdVkInstance& vulkan)
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

	game_state->instance = tdMalloc<GameInstance>(game_state->perm_arena);
	game_state->player = tdMalloc<Player>(game_state->perm_arena);
	PlayerLocalNew(game_state->player);//, 0, 1);
}

void GameInstanceFree()
{
}
