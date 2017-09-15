
void UpdateGameplay(double elapsed)
{
	auto game_state = GetGameState();
	GameInstance* instance = game_state->instance;
	Player *player = game_state->player;

	if (instance->invader_alive_count == 0)
	{
		NewInvaderWave(instance);
	}

	if (instance->invader_fleet_speed > 0)
	{
		if (instance->invader_fleet_extent.y > player->viewport.width - 10)
			instance->invader_fleet_speed = -instance->invader_fleet_speed;
	}
	else
	{
		if (instance->invader_fleet_extent.x < 10)
			instance->invader_fleet_speed = -instance->invader_fleet_speed;
	}

	instance->invader_fleet_pos.x += instance->invader_fleet_speed * elapsed;

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
			else
			{
				int32 invader_index = 0;
				Invader *invader = instance->invader_fleet;
				Invader *invader_end = invader + instance->invader_count;
				while (invader < invader_end)
				{
					if (invader->alive)
					{
						uint32 row = invader_index % GameConsts::wave_size.x;
						uint32 col = invader_index / GameConsts::wave_size.x;
						invader_pos.x = instance->invader_fleet_pos.x + row * GameConsts::invader_spacing.x;
						invader_pos.y = instance->invader_fleet_pos.y + col * GameConsts::invader_spacing.y;

						if (bullet->pos.y < invader_pos.y + GameConsts::invader_size.y &&
							bullet->pos.y + GameConsts::bullet_size.y > invader_pos.y &&
							bullet->pos.x + GameConsts::bullet_size.x > invader_pos.x &&
							bullet->pos.x < invader_pos.x + GameConsts::invader_size.x)
						{
							invader->alive = 0;
							bullet->alive = 0;
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
}

void UpdateFrame()
{
	TIMED_BLOCK(UpdateFrame);

	auto game_state = GetGameState();
	Player *player = game_state->player;

	if (player->mode == Player::pm_Play)
	{
		const double fps60 = 1.0 / 60.0;
		double elapsed = game_state->seconds;
		if (elapsed > fps60) elapsed = fps60;
		UpdateGameplay(elapsed);
	}
}
