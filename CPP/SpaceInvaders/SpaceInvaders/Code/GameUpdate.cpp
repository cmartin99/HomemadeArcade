
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
	}

	// Update all bullets
	Vector2 invader_pos;
	Bullet *bullet = instance->bullets;
	Bullet *bullet_end = bullet + instance->bullet_count;
	while (bullet < bullet_end)
	{
		if (bullet->alive)
		{
			bullet->pos.y -= GameConsts::bullet_speed.y * elapsed;
			if (bullet->pos.y < -GameConsts::bullet_size.y)
			{
				bullet->alive = 0;
			}
			else if (instance->invader_alive_count)
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

						if (bullet->pos.y < invader_pos.y + GameConsts::invader_size.y - 2 &&
							bullet->pos.y + GameConsts::bullet_size.y > invader_pos.y + 2 &&
							bullet->pos.x + GameConsts::bullet_size.x > invader_pos.x + 1 &&
							bullet->pos.x < invader_pos.x + GameConsts::invader_size.x - 1)
						{
							invader->alive = 0;
							bullet->alive = 0;

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
		++bullet;
	}

	if (instance->gameover_timer > 0 && game_state->total_seconds >= instance->gameover_timer)
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
