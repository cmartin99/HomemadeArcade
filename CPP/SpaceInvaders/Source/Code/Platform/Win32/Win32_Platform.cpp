#include "Win32_Platform.h"

namespace NewGame {

const static float gamepad_thumbleft_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
const static float gamepad_thumbright_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
const static float gamepad_thumbstick_range = 32767.0f;

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