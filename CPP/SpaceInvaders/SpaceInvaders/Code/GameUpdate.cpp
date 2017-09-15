
void UpdateGameplay(double elapsed)
{
	auto game_state = GetGameState();
	GameInstance* instance = game_state->instance;
	Player *player = game_state->player;

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

	Bullet *bullet = instance->bullets;
	Bullet *bullet_end = bullet + instance->bullet_count;
	while (bullet < bullet_end)
	{
		if (bullet->alive)
		{
			bullet->pos.y -= GameConsts::bullet_speed.y * elapsed;
			if (bullet->pos.y < -GameConsts::bullet_size.y) bullet->alive = 0;
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
