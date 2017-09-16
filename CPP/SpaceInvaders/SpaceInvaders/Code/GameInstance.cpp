
void PlayerFireBullet()
{
	auto game_state = GetGameState();
	GameInstance *instance = game_state->instance;

	// Recycle first free (dead) bullet
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

void NewInvaderFleet(GameInstance* instance)
{
	++instance->wave;
	instance->invader_fleet_speed = 100;
	instance->invader_fleet_pos.x = 100;
	instance->invader_fleet_pos.y = 100;
	instance->invader_fleet_extent = {FLT_MAX, -FLT_MAX};

	for (uint32 i = 0; i < instance->invader_count; ++i)
		instance->invader_fleet[i].alive = 1;
}

void GameInstanceNew(Player* player)
{
	auto game_state = GetGameState();

	game_state->main_arena.used = 0;
	memset(game_state->main_arena.base, 0, game_state->main_arena.size);

	GameInstance *instance = game_state->instance;
	instance->ship = tdMalloc<DefenderShip>(game_state->main_arena);
	instance->invader_count = GameConsts::fleet_size.x * GameConsts::fleet_size.y;
	instance->invader_alive_count = 0;
	instance->invader_fleet = tdMalloc<Invader>(game_state->main_arena, instance->invader_count);
	instance->ufo = tdMalloc<UFO>(game_state->main_arena);
	instance->bullet_count = 10;
	instance->bullets = tdMalloc<Bullet>(game_state->main_arena, instance->bullet_count);
	instance->stars_rng = tdMalloc<TdPcg32Random>(game_state->main_arena);
	instance->stars_rng_seed1 = tdRandomNext(game_state->rng);
	instance->stars_rng_seed2 = tdRandomNext(game_state->rng);
	instance->wave = 0;

	player->mode = Player::pm_Play;
	player->lives = 2;
	player->score = 0;
	instance->ship->pos.x = 100;
	instance->ship->pos.y = player->viewport.height - GameConsts::defender_size.y * 2;

	instance->ufo->pos.x = -GameConsts::ufo_size.x - 1;
	instance->ufo->pos.y = 30;
}
