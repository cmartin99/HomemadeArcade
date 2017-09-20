
ALWAYS_INLINE uint32 InvaderCount()
{
	return GameConsts::fleet_size.x * GameConsts::fleet_size.y;
}

void AddParticle(Vector2 pos, Vector2 vel, Vector2 size, Color color, float age)
{
	auto game_state = GetGameState();
	GameInstance *instance = game_state->instance;

	int32 index = 0;
	Particle *particle = instance->particles;
	Particle *particle_end = particle + GameConsts::max_particles;
	while (particle < particle_end && particle->age > 0) ++particle;
	if (particle == particle_end) particle = instance->particles;

	particle->pos = pos;
	particle->vel = vel;
	particle->size = size;
	particle->color = color;
	particle->age = age;
}

static Color explosion_colors[] = {Color(0, 0.5, 0, 1), Color(0.4, 0.4, 0.4, 1), Color(1, 0, 0, 1), Color(1, 0.42, 0, 1), Color(1, 0.85, 0, 1)};

void AddExplosionParticles(Vector2 pos, float momentum, float max_vel, uint32 count)
{
	auto game_state = GetGameState();
	Vector2 new_pos, vel, size;
	float age;
	Color color;
	momentum *= 0.4f;
	float max_vel2 = max_vel * 2.0;

	for (uint32 i = 0; i < count; ++i)
	{
		age = tdRandomNextDouble(game_state->rng) * 0.75 + 0.35;
		vel.x = tdRandomNextDouble(game_state->rng) * max_vel2 - max_vel + momentum;
		vel.y = tdRandomNextDouble(game_state->rng) * max_vel2 - max_vel;
		new_pos.x = pos.x + vel.x / max_vel * GameConsts::invader_size.x * 0.5f;
		new_pos.y = pos.y + vel.y / max_vel * GameConsts::invader_size.y * 0.5f;
		size.x = size.y = tdRandomNextDouble(game_state->rng) * 3 + 2;
		color = tdRandomNext(game_state->rng, 2) ? explosion_colors[0] : explosion_colors[tdRandomNext(game_state->rng, 5)];
		AddParticle(new_pos, vel, size, color, age);
	}
}

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
	instance->invader_fleet_pos.x = 100;
	instance->invader_fleet_pos.y = 100 + min(100, (int)instance->wave * 7);
	instance->invader_fleet_extent = {FLT_MAX, -FLT_MAX, -FLT_MAX};

	for (uint32 i = 0; i < InvaderCount(); ++i)
		instance->invader_fleet[i].alive = 1;

	instance->invader_fleet_speed = 80 + instance->wave * 2;
}

void GameInstanceNew(Player* player)
{
	auto game_state = GetGameState();

	game_state->main_arena.used = 0;
	memset(game_state->main_arena.base, 0, game_state->main_arena.size);

	GameInstance *instance = game_state->instance;
	instance->ship = tdMalloc<DefenderShip>(game_state->main_arena);
	instance->invader_fleet = tdMalloc<Invader>(game_state->main_arena, InvaderCount());
	instance->ufo = tdMalloc<UFO>(game_state->main_arena);
	instance->particles = tdMalloc<Particle>(game_state->main_arena, GameConsts::max_particles);
	instance->bullet_count = 10;
	instance->bullets = tdMalloc<Bullet>(game_state->main_arena, instance->bullet_count);
	instance->stars_rng = tdMalloc<TdPcg32Random>(game_state->main_arena);
	instance->stars_rng_seed1 = tdRandomNext(game_state->rng);
	instance->stars_rng_seed2 = tdRandomNext(game_state->rng);
	instance->wave = 0;
	instance->invader_alive_count = 0;
	instance->gameover_timer = 0;

	player->mode = Player::pm_Play;
	player->lives = 2;
	player->score = 0;
	instance->ship->pos.x = 100;
	instance->ship->pos.y = player->viewport.height - GameConsts::defender_size.y * 2;

	instance->ufo->pos.x = -GameConsts::ufo_size.x - 1;
	instance->ufo->pos.y = 30;
}
