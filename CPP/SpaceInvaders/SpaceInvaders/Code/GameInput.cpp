
ALWAYS_INLINE bool IsKeyDown(const TdInputState& input, int key)
{
	assert(key >= 0 && key < 256);
	return (GetKeyState(key) & 0x80) > 0;
	//return (input.keyboard.key_state[key] & 0x80) > 0;
}

ALWAYS_INLINE bool IsKeyDownNew(const TdInputState& input, int key)
{
	assert(key >= 0 && key < 256);
	return IsKeyDown(input, key) && ((input.keyboard.prev_state[key] & 0x80) == 0);
}

ALWAYS_INLINE bool IsKeyUp(const TdInputState& input, int key)
{
	return !IsKeyDown(input, key);
}

ALWAYS_INLINE bool IsKeyUpNew(const TdInputState& input, int key)
{
	assert(key >= 0 && key < 256);
	return IsKeyUp(input, key) && ((input.keyboard.prev_state[key] & 0x80) > 0);
}

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

void HandleRawInput(uint16 vkey, bool key_released)
{
	auto game_state = GetGameState();
	Player *player = game_state->player;

	if (key_released)
	{
		switch (vkey)
		{
			case VK_ESCAPE:
				break;
			case VK_F2:
				++game_state->debug_data.debug_verbosity;
				if (game_state->debug_data.debug_verbosity > 3)
					game_state->debug_data.debug_verbosity = 0;
				break;
			case 49:
				game_state->elapsed_scale *= 0.5;
				break;
			case 50:
				game_state->elapsed_scale *= 2.0;
				break;
		}
	}
}
