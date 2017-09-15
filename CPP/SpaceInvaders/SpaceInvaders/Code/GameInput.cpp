
static InputEventHandler left_click_handler;
static InputEventHandler left_pressed_handler;
static InputEventHandler right_click_handler;
static InputEventHandler right_pressed_handler;

void InputClearMouseHandlers()
{
	left_click_handler = nullptr;
	left_pressed_handler = nullptr;
	right_click_handler = nullptr;
	right_pressed_handler = nullptr;
}

ALWAYS_INLINE void InputSetLeftClickHandler(InputEventHandler h)
{
	left_click_handler = h;
}

ALWAYS_INLINE void InputSetLeftPressedHandler(InputEventHandler h)
{
	left_pressed_handler = h;
}

ALWAYS_INLINE void InputSetRightClickHandler(InputEventHandler h)
{
	right_click_handler = h;
}

ALWAYS_INLINE void InputSetRightPressedHandler(InputEventHandler h)
{
	right_pressed_handler = h;
}

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

	memcpy(&game_state->prev_gamepad, &input.gamepad, sizeof(TdGamePadState));

	XINPUT_STATE xi_state;
	XInputGetState(0, &xi_state);
	memset(&input.gamepad, 0, sizeof(TdGamePadState));
	ConvertXInput(input.gamepad, game_state->prev_gamepad, xi_state.Gamepad);

	if (player->mode == Player::pm_Play)
	{
		instance->ship->pos.x += input.gamepad.thumb_left.x * GameConsts::defender_speed.x * game_state->seconds;
		instance->ship->pos.x = tdClamp(instance->ship->pos.x, 0.f, player->viewport.width - GameConsts::defender_size.x);

		if (IsButtonDownNew(input.gamepad.a))
		{
			PlayerFireBullet();
		}
	}

	// if (game_state->player->sim->sim_paused)
	// 	return;

	TdPoint2 mouse_pos = game_state->input.mouse.mouse_pos;
	int state_change_count;
	bool down;

	if (left_click_handler)
	{
		state_change_count = input.mouse.mb_left.half_transition_count;
		down = input.mouse.mb_left.button_ended_down;
		while (state_change_count--)
		{
			if (!down) left_click_handler();
			down = !down;
		}
	}

	if (left_pressed_handler && input.mouse.mb_left.button_ended_down)
	{
		left_pressed_handler();
	}

	if (right_click_handler)
	{
		state_change_count = input.mouse.mb_right.half_transition_count;
		down = input.mouse.mb_right.button_ended_down;
		while (state_change_count--)
		{
			if (!down) right_click_handler();
			down = !down;
		}
	}

	if (right_pressed_handler && input.mouse.mb_right.button_ended_down)
	{
		right_pressed_handler();
	}
}

void HandleRawInput(uint16 vkey, bool key_released)
{
	auto game_state = GetGameState();
	Player *player = game_state->player;
	GameInstance* instance = game_state->instance;

	if (key_released)
	{
		switch (vkey)
		{
			case VK_F2:
				++game_state->debug_data.debug_verbosity;
				if (game_state->debug_data.debug_verbosity > 3)
					game_state->debug_data.debug_verbosity = 0;
				break;
		}
	}
	else
	{
		switch (vkey)
		{
			case 65:
			case VK_LEFT:
				instance->ship->pos.x -= GameConsts::defender_speed.x * game_state->seconds;
				break;

			case 68:
			case VK_RIGHT:
				instance->ship->pos.x += GameConsts::defender_speed.x * game_state->seconds;
				break;
		}
	}
}
