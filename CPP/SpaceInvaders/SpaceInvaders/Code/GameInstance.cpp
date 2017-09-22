
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

static Color explosion_colors[] = {Color(0.4, 0.4, 0.4, 1), Color(1, 0, 0, 1), Color(1, 0.42, 0, 1), Color(1, 0.85, 0, 1)};

void AddExplosionParticles(Vector2 pos, float momentum, float max_vel, uint32 count, Color base_color, TdPoint2 ship_size, Vector2 particle_size_range)
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
		new_pos.x = pos.x + vel.x / max_vel * ship_size.x * 0.5f;
		new_pos.y = pos.y + vel.y / max_vel * ship_size.y * 0.5f;
		size.x = size.y = tdRandomNextDouble(game_state->rng) * particle_size_range.x + particle_size_range.y;
		color = tdRandomNext(game_state->rng, 2) ? base_color : explosion_colors[tdRandomNext(game_state->rng, 4)];
		AddParticle(new_pos, vel, size, color, age);
	}
}

ALWAYS_INLINE void AddInvaderExplosionParticles(Vector2 pos, float momentum)
{
	AddExplosionParticles(pos, momentum, 500, 20, Color(0, 0.55, 0.05, 1), GameConsts::invader_size, Vector2(4, 2));
}

ALWAYS_INLINE void AddUfoExplosionParticles(Vector2 pos, float momentum)
{
	AddExplosionParticles(pos, momentum, 500, 60, Color(0.66, 0, 0.77, 1), GameConsts::ufo_size, Vector2(6, 3));
}

ALWAYS_INLINE void AddDefenderExplosionParticles(Vector2 pos, float momentum)
{
	AddExplosionParticles(pos, momentum, 500, 100, Colors::White, GameConsts::defender_size, Vector2(8, 3));
}

void AddBullet(Vector2 pos, Vector2 vel, Color color)
{
	auto game_state = GetGameState();

	// Recycle first free (dead) bullet
	Bullet *bullet = game_state->instance->bullets;
	Bullet *bullet_end = bullet + game_state->instance->bullet_count;
	while (bullet < bullet_end)
	{
		if (!bullet->alive)
		{
			bullet->alive = 1;
			bullet->pos.x = pos.x;
			bullet->pos.y = pos.y;
			bullet->vel.x = vel.x;
			bullet->vel.y = vel.y;
			bullet->color = color;
			break;
		}
		++bullet;
	}
}

void PlayerFireBullet()
{
	auto game_state = GetGameState();
	Vector2 pos;
	pos.x = game_state->instance->ship->pos.x + GameConsts::defender_size.x / 2 - GameConsts::bullet_size.x / 2;
	pos.y = game_state->instance->ship->pos.y - GameConsts::bullet_size.y;
	AddBullet(pos, Vector2(0, -GameConsts::bullet_speed.y), Color(0.9f, 0.9f, 0.9f, 1.f));
}

void InvaderDropBomb(Vector2 pos)
{
	AddBullet(pos, GameConsts::invader_bomb_speed, Color(0.9f, 0.83f, 0.f, 1.f));
}

void UfoDropBomb(Vector2 pos)
{
	AddBullet(pos, GameConsts::ufo_bomb_speed, Color(0.75f, 0.176f, 0.84f, 1.f));
}

void NewInvaderFleet(GameInstance* instance)
{
	instance->invader_fleet_state = GameInstance::in_MovingAcross;
	instance->invader_fleet_pos.x = 100;
	instance->invader_fleet_pos.y = 100 + min(200, (int)instance->wave * 8);
	instance->invader_fleet_extent = {FLT_MAX, -FLT_MAX, -FLT_MAX};

	for (uint32 i = 0; i < InvaderCount(); ++i)
		instance->invader_fleet[i].alive = 1;

	instance->invader_fleet_speed = 80 + instance->wave * 4;
	++instance->wave;
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
	player->lives = 3;
	player->score = 0;
	instance->ship->pos.x = 100;
	instance->ship->pos.y = player->viewport.height - GameConsts::defender_size.y * 2;

	instance->ufo->pos.x = -GameConsts::ufo_size.x - 1;
	instance->ufo->pos.y = 0;
	instance->ufo_spawn_timer = game_state->total_seconds + 1;
}
