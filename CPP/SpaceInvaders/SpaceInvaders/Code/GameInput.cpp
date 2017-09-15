
ALWAYS_INLINE bool IsButtonDownNew(TdButtonState button)
{
	return button.button_ended_down && button.half_transition_count;
}

void HandleInput()
{
	TIMED_BLOCK(HandleInput);
	if (!IsGameInitialized()) return;

	auto game_state = GetGameState();
	auto debug_data = &game_state->debug_data;
	auto player = game_state->player;
	auto instance = game_state->instance;
	auto& input = game_state->input;
	input.seconds = game_state->seconds;

	// Set gamepad state
	memcpy(&game_state->prev_gamepad, &input.gamepad, sizeof(TdGamePadState));
	XINPUT_STATE xi_state;
	XInputGetState(0, &xi_state);
	memset(&input.gamepad, 0, sizeof(TdGamePadState));
	ConvertXInput(input.gamepad, game_state->prev_gamepad, xi_state.Gamepad);

	// Gameplay inputs
	if (player->mode == Player::pm_Play)
	{
		instance->ship->pos.x += input.gamepad.thumb_left.x * GameConsts::defender_speed.x * game_state->seconds;
		instance->ship->pos.x = tdClamp(instance->ship->pos.x, 0.f, player->viewport.width - GameConsts::defender_size.x);

		if (IsButtonDownNew(input.gamepad.a))
		{
			PlayerFireBullet();
		}
	}
}
