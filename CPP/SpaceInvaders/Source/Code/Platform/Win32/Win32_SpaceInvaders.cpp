#include "Win32_Platform.h"
#include <windows.h>
#include <windowsx.h>
#include <locale>
#include "vulkan.h"
#include "xinput.h"
#include "TdEngine.h"
#include "ForwardDecls.h"

using namespace glm;
using namespace eng;

bool eng::TdTimedBlock::timed_blocks_paused = false;

namespace SpaceInvaders {

static AppMemory _memory = {};
static bool app_activated;
static bool running;
static HINSTANCE hinst;

LRESULT CALLBACK Win32_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (uMsg == WM_ACTIVATEAPP && !app_activated)
	{
		TdVkInstance* vulkan = new TdVkInstance();
		VkResult err = tdVkInitVulkan(vulkan, "NewGame", false, hinst, hwnd);
		if (err)
		{
			tdDisplayError("InitVulkan", err);
			exit(1);
		}

		InitApplication(_memory, vulkan);
		app_activated = true;
	}

	if (!IsAppInitialized())
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	auto app_state = GetAppState();

	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		app_state->input.mouse.mouse_pos.x = GET_X_LPARAM(lParam);
		app_state->input.mouse.mouse_pos.y = GET_Y_LPARAM(lParam);
		break;

	case WM_MOUSEWHEEL:
		HandleMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
		break;

	case WM_LBUTTONDOWN:
		app_state->input.mouse.mb_left.button_ended_down = true;
		++app_state->input.mouse.mb_left.half_transition_count;
		break;

	case WM_LBUTTONUP:
		app_state->input.mouse.mb_left.button_ended_down = false;
		++app_state->input.mouse.mb_left.half_transition_count;
		break;

	case WM_RBUTTONDOWN:
		app_state->input.mouse.mb_right.button_ended_down = true;
		++app_state->input.mouse.mb_right.half_transition_count;
		break;

	case WM_RBUTTONUP:
		app_state->input.mouse.mb_right.button_ended_down = false;
		++app_state->input.mouse.mb_right.half_transition_count;
		break;

	case WM_MBUTTONDOWN:
		app_state->input.mouse.mb_left.button_ended_down = true;
		++app_state->input.mouse.mb_left.half_transition_count;
		break;

	case WM_MBUTTONUP:
		app_state->input.mouse.mb_middle.button_ended_down = false;
		++app_state->input.mouse.mb_middle.half_transition_count;
		break;

	case WM_PAINT:
		ValidateRect(hwnd, NULL);
		break;

	case WM_CLOSE:
		running = false;
		break;

	case WM_DESTROY:
		running = false;
		break;

	default:
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}

	return result;
}

} // end of empty namespace

#include "Win32_Platform.cpp"
#include "notelemetry.cpp"
#include "nothrownew.cpp"

// no namespace

const double fps60 = 60.0/1000.0;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	using namespace SpaceInvaders;

	hinst = hInstance;
	app_activated = false;
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	WNDCLASS win_class = {};
	win_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = &Win32_WindowProc;
	win_class.hInstance = hInstance;
	win_class.lpszClassName = "GameWinClass";

	RegisterClass(&win_class);

	int width = 1200, height = 640;
	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	bool fullscreen = false;

	DWORD dwStyle = fullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT rect;
	if (fullscreen)
	{
		rect.left = (long)0;
		rect.right = (long)screen_width;
		rect.top = (long)0;
		rect.bottom = (long)screen_height;
	}
	else
	{
		rect.left = (long)screen_width / 2 - width / 2;
		rect.right = (long)width;
		rect.top = (long)screen_height / 2 - height / 2;
		rect.bottom = (long)height;
	}

	AdjustWindowRect(&rect, dwStyle, FALSE);

	HWND win = CreateWindow(win_class.lpszClassName, "Game", dwStyle, rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, 0);
	if (!win) return 0;

	eng::shader_path = "content\\shaders\\";
	_memory.eng_ram_size = 100 * 1024 * 1024;
	_memory.perm_ram_size = 10 * 1024 * 1024;
	_memory.main_ram_size = 100 * 1024 * 1024;
	_memory.scratch_ram_size = 10 * 1024 * 1024;

	_memory.eng_ram = VirtualAlloc(0, _memory.eng_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	_memory.perm_ram = VirtualAlloc(0, _memory.perm_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	_memory.main_ram = VirtualAlloc(0, _memory.main_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	_memory.scratch_ram = VirtualAlloc(0, _memory.scratch_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!_memory.eng_ram || !_memory.perm_ram || !_memory.main_ram || !_memory.scratch_ram)
	{
		tdDisplayError("Could not allocate game memory", 0);
		exit(1);
	}

	tdMemoryArenaInit(eng_arena, _memory.eng_ram_size, _memory.eng_ram);

#ifdef _PROFILE_
	tdArrayInit(timed_blocks, 200);
#endif
	RAWINPUTDEVICE input_devices[2];
	input_devices[0].usUsagePage = 1;
	input_devices[0].usUsage = 2;
	input_devices[0].dwFlags = 0;
	input_devices[0].hwndTarget = win;
	input_devices[1].usUsagePage = 1;
	input_devices[1].usUsage = 6;
	input_devices[1].dwFlags = RIDEV_NOLEGACY;
	input_devices[1].hwndTarget = win;

	if (!RegisterRawInputDevices(input_devices, 2, sizeof(RAWINPUTDEVICE)))
	{
		return 1;
	}

	LARGE_INTEGER perf_freq_temp;
	QueryPerformanceFrequency(&perf_freq_temp);
	uint64 perf_freq = perf_freq_temp.QuadPart;
	LARGE_INTEGER last_perf_counter, perf_counter;
	QueryPerformanceCounter(&last_perf_counter);

	MSG message;
	running = true;

	std::locale::global(std::locale(""));
	OutputDebugStringA(std::locale().name().c_str());

	ShowWindow(win, SW_SHOW);
	SetForegroundWindow(win);
	auto r = SetFocus(win);

#if _PROFILE_
	TdTimedBlockCounter frame_counter = { 1, 0, 0, 0, 0, 0, 0, "frame" };
	tdArrayAdd(timed_blocks, frame_counter);
	DWORD64 start_cycles = __rdtsc();
#endif

	auto app_state = GetAppState();
	while (running)
	{
		BOOL result;
		TdPoint2 mpos = app_state->input.mouse.mouse_pos;
		bool mb_left_down = app_state->input.mouse.mb_left.button_ended_down;
		bool mb_middle_down = app_state->input.mouse.mb_middle.button_ended_down;
		bool mb_right_down = app_state->input.mouse.mb_right.button_ended_down;
		memclear<TdMouseState>(&app_state->input.mouse);

		app_state->input.mouse.mouse_pos = mpos;
		app_state->input.mouse.mb_left.button_ended_down = mb_left_down;
		app_state->input.mouse.mb_middle.button_ended_down = mb_middle_down;
		app_state->input.mouse.mb_right.button_ended_down = mb_right_down;
		memclear<TdGamePadState>(&app_state->input.gamepad);

		while ((result = PeekMessage(&message, 0, 0, 0, PM_REMOVE)) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (result >= 0)
		{
			memcpy(app_state->input.keyboard.prev_state, app_state->input.keyboard.key_state, sizeof(int16) * 256);
			//GetKeyboardState(app_state->input.keyboard.key_state);
		   	for (int i = 0; i < 256; ++i) app_state->input.keyboard.key_state[i] = (int16)GetAsyncKeyState(i);

			QueryPerformanceCounter(&perf_counter);
			uint64 delta = perf_counter.QuadPart - last_perf_counter.QuadPart;
			double delta_secs = (double)delta / perf_freq;
			eng::total_seconds += delta_secs;
			app_state->seconds = delta_secs;
			app_state->total_seconds += app_state->seconds;
			last_perf_counter = perf_counter;
#if _PROFILE_
			DWORD64 cycles = __rdtsc();
			timed_blocks[0].total_cycles = cycles - start_cycles;
			start_cycles = cycles;
#endif
			RunOneFrame();

			if (app_state->exit_app) running = false;
		}
		else
		{
			running = false;
		}
	}

	if (app_activated)
		CloseApplication();

	return 0;
}
