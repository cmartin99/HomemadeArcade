#include "Win32_Platform.h"
//#include "PluginAPI.h"

namespace NewGame {
/*
void LoadPlugin(const char *path)
{ 
    HMODULE module = LoadLibrary(path); 
    if (!module) return; 

    GetAPIFunction get_plugin_api = (GetAPIFunction)GetProcAddress(module, "GetPluginAPI");
    if (!get_plugin_api) return;

    PluginAPI_v0* plugin = (PluginAPI_v0*)get_plugin_api(PLUGIN_API_ID, 0); 
    if (!plugin) return;

	auto game_state = GetGameState();
	tdArrayAdd(game_state->plugins, plugin);

	HPlugin h = game_state->plugins.count;
    plugin->Init(h, GetGameAPI);
}
*/
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