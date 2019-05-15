#include "Win32_Platform.h"

namespace SpaceInvaders {

const static float gamepad_thumbleft_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
const static float gamepad_thumbright_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
const static float gamepad_thumbstick_range = 32767.0f;

void ConvertXInput(TdGamePadState& gamepad, const TdGamePadState& prev_gamepad, XINPUT_GAMEPAD x_gamepad)
{
	gamepad.a.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_A) > 0;
	gamepad.b.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_B) > 0;
	gamepad.x.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_X) > 0;
	gamepad.y.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_Y) > 0;

	gamepad.dpad_up.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) > 0;
	gamepad.dpad_down.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) > 0;
	gamepad.dpad_left.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) > 0;
	gamepad.dpad_right.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) > 0;

	gamepad.back.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_BACK) > 0;
	gamepad.start.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_START) > 0;

	gamepad.shoulder_left.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) > 0;
	gamepad.shoulder_right.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) > 0;

	gamepad.thumb_left_pressed.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) > 0;
	gamepad.thumb_right_pressed.button_ended_down = (x_gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) > 0;

	gamepad.trigger_left = x_gamepad.bLeftTrigger / 256.0f;
	gamepad.trigger_right = x_gamepad.bRightTrigger / 256.0f;

	gamepad.thumb_left.x = x_gamepad.sThumbLX;
	gamepad.thumb_left.y = x_gamepad.sThumbLY;
	gamepad.thumb_right.x = x_gamepad.sThumbRX;
	gamepad.thumb_right.y = x_gamepad.sThumbRY;

	if (gamepad.thumb_left.x > -gamepad_thumbleft_deadzone && gamepad.thumb_left.x < gamepad_thumbleft_deadzone) gamepad.thumb_left.x = 0;
	if (gamepad.thumb_left.y > -gamepad_thumbleft_deadzone && gamepad.thumb_left.y < gamepad_thumbleft_deadzone) gamepad.thumb_left.y = 0;
	if (gamepad.thumb_right.x > -gamepad_thumbright_deadzone && gamepad.thumb_right.x < gamepad_thumbright_deadzone) gamepad.thumb_right.x = 0;
	if (gamepad.thumb_right.y > -gamepad_thumbright_deadzone && gamepad.thumb_right.y < gamepad_thumbright_deadzone) gamepad.thumb_right.y = 0;

	gamepad.thumb_left.x /= gamepad_thumbstick_range;
	gamepad.thumb_left.y /= gamepad_thumbstick_range;
	gamepad.thumb_right.x /= gamepad_thumbstick_range;
	gamepad.thumb_right.y /= gamepad_thumbstick_range;

	if (gamepad.a.button_ended_down != prev_gamepad.a.button_ended_down) gamepad.a.half_transition_count = 1;
	if (gamepad.b.button_ended_down != prev_gamepad.b.button_ended_down) gamepad.b.half_transition_count = 1;
	if (gamepad.x.button_ended_down != prev_gamepad.x.button_ended_down) gamepad.x.half_transition_count = 1;
	if (gamepad.y.button_ended_down != prev_gamepad.y.button_ended_down) gamepad.y.half_transition_count = 1;
	if (gamepad.dpad_up.button_ended_down != prev_gamepad.dpad_up.button_ended_down) gamepad.dpad_up.half_transition_count = 1;
	if (gamepad.dpad_down.button_ended_down != prev_gamepad.dpad_down.button_ended_down) gamepad.dpad_down.half_transition_count = 1;
	if (gamepad.dpad_left.button_ended_down != prev_gamepad.dpad_left.button_ended_down) gamepad.dpad_left.half_transition_count = 1;
	if (gamepad.dpad_right.button_ended_down != prev_gamepad.dpad_right.button_ended_down) gamepad.dpad_right.half_transition_count = 1;
	if (gamepad.back.button_ended_down != prev_gamepad.back.button_ended_down) gamepad.back.half_transition_count = 1;
	if (gamepad.start.button_ended_down != prev_gamepad.start.button_ended_down) gamepad.start.half_transition_count = 1;
	if (gamepad.shoulder_left.button_ended_down != prev_gamepad.shoulder_left.button_ended_down) gamepad.shoulder_left.half_transition_count = 1;
	if (gamepad.shoulder_right.button_ended_down != prev_gamepad.shoulder_right.button_ended_down) gamepad.shoulder_right.half_transition_count = 1;
	if (gamepad.thumb_left_pressed.button_ended_down != prev_gamepad.thumb_left_pressed.button_ended_down) gamepad.thumb_left_pressed.half_transition_count = 1;
	if (gamepad.thumb_right_pressed.button_ended_down != prev_gamepad.thumb_right_pressed.button_ended_down) gamepad.thumb_right_pressed.half_transition_count = 1;
}

}

namespace eng {

void tdDisplayError(const char* msg, HRESULT hr)
{
	MessageBox(0, msg, "Error", MB_OK | MB_ICONERROR);
}

void tdDisplayError(const char* msg, VkResult err)
{
	MessageBox(0, (std::string(msg) + std::string("\n") + eng::tdErrorString(err)).c_str(), "Error", MB_OK | MB_ICONERROR);
}

}