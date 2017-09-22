
void UpdateGameplay(double elapsed)
{
	auto game_state = GetGameState();
	GameInstance* instance = game_state->instance;
	Player *player = game_state->player;

	// New wave
	if (instance->invader_alive_count == 0)
	{
		if (game_state->total_seconds >= instance->new_fleet_timer)
			NewInvaderFleet(instance);
	}
	else
	{
		// Move entire invader fleet
		if (instance->invader_fleet_state == GameInstance::in_MovingAcross)
		{
			instance->invader_fleet_pos.x += instance->invader_fleet_speed * elapsed;

			// Check if invader fleet has hit screen edge
			if (instance->invader_fleet_speed > 0)
			{
				if (instance->invader_fleet_extent.y > player->viewport.width - 10)
				{
					instance->invader_fleet_speed = -instance->invader_fleet_speed;
					instance->invader_fleet_state = GameInstance::in_CreepingDown;
					instance->invader_fleet_y_target = instance->invader_fleet_pos.y + GameConsts::invader_fleet_creep_distance;
				}
			}
			else
			{
				if (instance->invader_fleet_extent.x < 10)
				{
					instance->invader_fleet_speed = -instance->invader_fleet_speed;
					instance->invader_fleet_state = GameInstance::in_CreepingDown;
					instance->invader_fleet_y_target = instance->invader_fleet_pos.y + GameConsts::invader_fleet_creep_distance;
				}
			}
		}
		else if (instance->invader_fleet_state == GameInstance::in_CreepingDown)
		{
			if (instance->invader_fleet_extent.z >= instance->ship->pos.y)
			{
				if (instance->gameover_timer == 0)
					instance->gameover_timer = game_state->total_seconds + 3;
			}
			else
			{
				instance->invader_fleet_pos.y += GameConsts::invader_fleet_creep_speed * elapsed;
				if (instance->invader_fleet_pos.y >= instance->invader_fleet_y_target)
					instance->invader_fleet_state = GameInstance::in_MovingAcross;
			}
		}

		if (instance->invader_alive_count && instance->invader_fleet_state != GameInstance::in_Waiting)
		{
			// Update fleet extent and find (closest) invader to drop a bomb
			Vector2 closest_dist = {FLT_MAX, FLT_MAX};
			Vector2 closest_pos;
			Invader *closest_invader = nullptr;
			instance->invader_fleet_extent.x = FLT_MAX;
			instance->invader_fleet_extent.y = -FLT_MAX;
			instance->invader_fleet_extent.z = -FLT_MAX;
			int32 invader_index = 0;
			Invader *invader = instance->invader_fleet;
			Invader *invader_end = invader + InvaderCount();
			while (invader < invader_end)
			{
				if (invader->alive)
				{
					uint32 row = invader_index % GameConsts::fleet_size.x;
					uint32 col = invader_index / GameConsts::fleet_size.x;
					float x = instance->invader_fleet_pos.x + row * GameConsts::invader_spacing.x;
					float y = instance->invader_fleet_pos.y + col * GameConsts::invader_spacing.y;

					if (x < instance->invader_fleet_extent.x)
						instance->invader_fleet_extent.x = x;
					if (x + GameConsts::invader_size.x > instance->invader_fleet_extent.y)
						instance->invader_fleet_extent.y = x + GameConsts::invader_size.x;
					if (y + GameConsts::invader_size.y > instance->invader_fleet_extent.z)
						instance->invader_fleet_extent.z = y + GameConsts::invader_size.y;

					if (invader != instance->invader_last_to_drop_bomb)
					{
						// find best (closest) invader to drop a bomb on player
						double dx = abs(x - instance->ship->pos.x);
						double dy = abs(y - instance->ship->pos.y);
						if (dx < closest_dist.x || (dx == closest_dist.x && dy < closest_dist.y))
						{
							closest_dist.x = dx;
							closest_dist.y = dy;
							closest_pos.x = x;
							closest_pos.y = y;
							closest_invader = invader;
						}
					}
				}
				++invader;
				++invader_index;
			}

			if (player->lives > 0)
			{
				// Drop bomb (maybe)
				closest_pos.x += GameConsts::invader_size.x / 2 - GameConsts::bullet_size.x / 2;
				closest_pos.y += GameConsts::invader_size.y - 8;
				if (closest_pos.x > instance->ship->pos.x - 200 && closest_pos.x < instance->ship->pos.x + GameConsts::defender_size.x + 200)
				{
					if (game_state->total_seconds > instance->invader_bomb_timer)
					{
						InvaderDropBomb(closest_pos);
						instance->invader_bomb_timer = game_state->total_seconds + 0.5;
						instance->invader_last_to_drop_bomb = closest_invader;
					}
				}
			}
		}
	}

	// Update UFO
	UFO *ufo = instance->ufo;
	if (ufo->pos.y > 0)
	{
		ufo->pos.x += ufo->vel.x * elapsed;
		if (ufo->pos.x < -GameConsts::ufo_size.x || ufo->pos.x > player->viewport.width)
		{
			ufo->pos.y = 0;
			instance->ufo_spawn_timer = game_state->total_seconds + 1;
		}
		else if (player->lives > 0 && player->gameplay_mode != Player::gm_Respawn)
		{
			// Drop bomb (maybe)
			Vector2 bomb_pos = ufo->pos;
			bomb_pos.x += GameConsts::ufo_size.x / 2 - GameConsts::bullet_size.x / 2;
			bomb_pos.y += GameConsts::ufo_size.y - 4;
			if (bomb_pos.x > instance->ship->pos.x - 300 && bomb_pos.x < instance->ship->pos.x + GameConsts::defender_size.x + 300)
			{
				if (game_state->total_seconds > instance->ufo_bomb_timer)
				{
					UfoDropBomb(bomb_pos);
					instance->ufo_bomb_timer = game_state->total_seconds + 2;
				}
			}
		}
	}
	else
	{
		if (game_state->total_seconds > instance->ufo_spawn_timer)
		{
			if (tdRandomNext(game_state->rng, 15) == 0)
			{
				ufo->pos.y = 56;
				if (tdRandomNext(game_state->rng, 2) == 0)
				{
					ufo->pos.x = -GameConsts::ufo_size.x;
					ufo->vel.x = GameConsts::ufo_speed;
				}
				else
				{
					ufo->pos.x = player->viewport.width;
					ufo->vel.x = -GameConsts::ufo_speed;
				}
			}
			else
				instance->ufo_spawn_timer = game_state->total_seconds + 1;
		}
	}

	// Update all bullets
	Vector2 invader_pos;
	Bullet *bullet = instance->bullets;
	Bullet *bullet_end = bullet + instance->bullet_count;
	while (bullet < bullet_end)
	{
		if (bullet->alive)
		{
			bullet->pos.y += bullet->vel.y * elapsed;
			if (bullet->pos.y < -GameConsts::bullet_size.y || bullet->pos.y > player->viewport.height)
			{
				bullet->alive = 0;
			}
			else if (bullet->vel.y < 0)
			{
				if (ufo->pos.y > 0)
				{
					if (bullet->pos.y < ufo->pos.y + GameConsts::ufo_size.y - 2 &&
						bullet->pos.y + GameConsts::bullet_size.y > ufo->pos.y + 2 &&
						bullet->pos.x + GameConsts::bullet_size.x > ufo->pos.x + 1 &&
						bullet->pos.x < ufo->pos.x + GameConsts::ufo_size.x - 1)
					{
						bullet->alive = 0;
						instance->ufo_spawn_timer = game_state->total_seconds + 1;

						Vector2 particle_pos = ufo->pos;
						particle_pos.x += GameConsts::ufo_size.x * 0.5f;
						particle_pos.y += GameConsts::ufo_size.y * 0.5f;
						AddUfoExplosionParticles(particle_pos, ufo->vel.x);

						player->score += 250;
						if (player->score > instance->high_score) instance->high_score = player->score;

						// flag ufo as dead/inactive
						ufo->pos.y = 0;
						break;
					}
				}

				if (instance->invader_alive_count)
				{
					int32 invader_index = 0;
					Invader *invader = instance->invader_fleet;
					Invader *invader_end = invader + InvaderCount();
					while (invader < invader_end)
					{
						if (invader->alive)
						{
							uint32 row = invader_index % GameConsts::fleet_size.x;
							uint32 col = invader_index / GameConsts::fleet_size.x;
							invader_pos.x = instance->invader_fleet_pos.x + row * GameConsts::invader_spacing.x;
							invader_pos.y = instance->invader_fleet_pos.y + col * GameConsts::invader_spacing.y;

							if (bullet->pos.y < invader_pos.y + GameConsts::invader_size.y - 12 &&
								bullet->pos.y + GameConsts::bullet_size.y > invader_pos.y + 2 &&
								bullet->pos.x + GameConsts::bullet_size.x > invader_pos.x + 4 &&
								bullet->pos.x < invader_pos.x + GameConsts::invader_size.x - 4)
							{
								invader->alive = 0;
								bullet->alive = 0;

								Vector2 particle_pos = invader_pos;
								particle_pos.x += GameConsts::invader_size.x * 0.5f;
								particle_pos.y += GameConsts::invader_size.y * 0.5f;
								AddInvaderExplosionParticles(particle_pos, instance->invader_fleet_speed);

								if (instance->invader_alive_count == 1)
									instance->new_fleet_timer = game_state->total_seconds + 3;
								else
								{
									float inc_fac = (InvaderCount() - instance->invader_alive_count + 1) / (float)InvaderCount() * 20.0f;
									instance->invader_fleet_speed += sign(instance->invader_fleet_speed) * inc_fac;
								}

								player->score += 50;
								if (player->score > instance->high_score) instance->high_score = player->score;
								break;
							}
						}
						++invader;
						++invader_index;
					}
				}
			}
			else if (player->lives && player->gameplay_mode != Player::gm_Respawn)
			{
				DefenderShip *ship = instance->ship;
				if (bullet->pos.y < ship->pos.y + GameConsts::defender_size.y - 2 &&
					bullet->pos.y + GameConsts::bullet_size.y > ship->pos.y + 16 &&
					bullet->pos.x + GameConsts::bullet_size.x > ship->pos.x + 8 &&
					bullet->pos.x < ship->pos.x + GameConsts::defender_size.x - 8)
				{
					bullet->alive = 0;
					Vector2 particle_pos = ship->pos;
					particle_pos.x += GameConsts::defender_size.x * 0.5f;
					particle_pos.y += GameConsts::defender_size.y * 0.5f;
					AddDefenderExplosionParticles(particle_pos, ufo->vel.x);

					if (--player->lives == 0)
					{
						instance->gameover_timer = game_state->total_seconds + 3;
					}
					else
					{
						player->gameplay_mode = Player::gm_Respawn;
						player->respawn_timer = game_state->total_seconds + GameConsts::defender_respawn_time;
						instance->prev_invader_fleet_state = instance->invader_fleet_state;
						instance->invader_fleet_state = GameInstance::in_Waiting;
					}
					break;
				}
			}
		}
		++bullet;
	}

	float age_elapsed;
	Particle *particle = instance->particles;
	Particle *particle_end = particle + GameConsts::max_particles;
	while (particle < particle_end)
	{
		if (particle->age > 0)
		{
			particle->age -= elapsed;
			age_elapsed = particle->age < 0.5f ? max(0.1f, particle->age * 2) : 1;
			age_elapsed *= elapsed;
			particle->pos.x += particle->vel.x * age_elapsed;
			particle->pos.y += particle->vel.y * age_elapsed;
		}
		++particle;
	}

	if (player->gameplay_mode == Player::gm_Respawn)
	{
		if (game_state->total_seconds >= player->respawn_timer)
		{
			player->gameplay_mode = Player::gm_Play;
			instance->invader_fleet_state = instance->prev_invader_fleet_state;
		}
	}
	else if (instance->gameover_timer > 0 && game_state->total_seconds >= instance->gameover_timer)
	{
		player->mode = Player::pm_Menu;
	}
}

void UpdateFrame()
{
	TIMED_BLOCK(UpdateFrame);
	auto game_state = GetGameState();

	if (game_state->player->mode == Player::pm_Play)
	{
		const double fps60 = 1.0 / 60.0;
		double elapsed = game_state->seconds;
		if (elapsed > fps60) elapsed = fps60;

		UpdateGameplay(elapsed * game_state->elapsed_scale);
	}
}
