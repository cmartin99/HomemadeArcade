#include <windows.h>
#include <windowsx.h>
#include <locale>
#include "vulkan.h"
#include "TdEngine.h"
#include "Win32_Platform.h"

using namespace glm;
using namespace eng;

namespace SpaceInvaders {

static SpaceInvaders_Platform::GameMemory memory = {};
static FILE* log_file;
static bool app_activated;
static bool global_running;
static HINSTANCE hinst;
static HWND hwnd_main;
static uint8 rawinput_buffer[48];
static uint64 platform_frame_count;

void HandleRawInput(HWND hwnd, LPARAM lParam)
{
	if (!IsGameInitialized()) return;

	UINT dwSize;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	assert(dwSize <= 48);

	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawinput_buffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
	{
		OutputDebugString(TEXT("GetRawInputData does not return correct header size !\n"));
		return;
	}

	RAWINPUTHEADER* header = (RAWINPUTHEADER*)rawinput_buffer;
	RAWINPUT* input = (RAWINPUT*)rawinput_buffer;

	if (header->dwType == RIM_TYPEKEYBOARD)
	{
		HandleRawInput(input->data.keyboard.VKey, (input->data.keyboard.Flags & RI_KEY_BREAK) > 0);
	}
}

LRESULT CALLBACK Win32_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (uMsg == WM_ACTIVATEAPP && !app_activated)
	{
		TdVkInstance *vulkan = new TdVkInstance();
		VkResult err = tdVkInitVulkan(*vulkan, "Game1", false, hinst, hwnd);
		if (err) {
			tdDisplayError("InitVulkan", err);
			exit(1);
		}

		ApplicationNew(*vulkan);
		app_activated = true;
	}

	if (!IsGameInitialized())
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	TdInputState *input = GetInput();

	switch (uMsg)
	{
	case WM_INPUT:
		HandleRawInput(hwnd, lParam);
		break;

	case WM_KEYDOWN:
		input->keyboard.key_state[wParam] = 0x80;
		global_running = (wParam == VK_ESCAPE);
		break;

	case WM_KEYUP:
		input->keyboard.key_state[wParam] = 0;
		break;

	case WM_MOUSEMOVE:
		input->mouse.mouse_pos.x = GET_X_LPARAM(lParam);
		input->mouse.mouse_pos.y = GET_Y_LPARAM(lParam);
		break;

	case WM_LBUTTONDOWN:
		input->mouse.mb_left.button_ended_down = true;
		++input->mouse.mb_left.half_transition_count;
		break;

	case WM_LBUTTONUP:
		input->mouse.mb_left.button_ended_down = false;
		++input->mouse.mb_left.half_transition_count;
		break;

	case WM_RBUTTONDOWN:
		input->mouse.mb_right.button_ended_down = true;
		++input->mouse.mb_right.half_transition_count;
		break;

	case WM_RBUTTONUP:
		input->mouse.mb_right.button_ended_down = false;
		++input->mouse.mb_right.half_transition_count;
		break;

	case WM_MBUTTONDOWN:
		input->mouse.mb_left.button_ended_down = true;
		++input->mouse.mb_left.half_transition_count;
		break;

	case WM_MBUTTONUP:
		input->mouse.mb_middle.button_ended_down = false;
		++input->mouse.mb_middle.half_transition_count;
		break;

	case WM_PAINT:
		ValidateRect(hwnd, NULL);
		break;

	case WM_CLOSE:
		global_running = false;
		break;

	case WM_DESTROY:
		if (log_file) fclose(log_file);
		global_running = false;
		break;

	default:
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}

	return result;
}

void ExitGame()
{
	global_running = false;
}

} // end of namespace

#include "Win32_Platform.cpp"
#include "notelemetry.cpp"
#include "nothrownew.cpp"

// no namespace

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	using namespace SpaceInvaders;

	hinst = hInstance;
	app_activated = false;

	WNDCLASS win_class = {};
	win_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = &Win32_WindowProc;
	win_class.hInstance = hInstance;
	win_class.lpszClassName = "GameWinClass";
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&win_class);

	int width = 1600, height = 900;
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

	hwnd_main = CreateWindow(win_class.lpszClassName, "Game2", dwStyle, rect.left, rect.top, rect.right, rect.bottom, NULL, NULL, hInstance, 0);
	if (!hwnd_main) return 0;

	eng::shader_path = "content\\shaders\\";
	memory.eng_ram_size = 100 * 1024 * 1024;
	memory.eng_ram = VirtualAlloc(0, memory.eng_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	tdMemoryArenaInit(eng_arena, memory.eng_ram_size, memory.eng_ram);
	memory.perm_ram_size = 100 * 1024 * 1024;
	memory.perm_ram = VirtualAlloc(0, memory.perm_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memory.main_ram_size = 100 * 1024 * 1024;
	memory.main_ram = VirtualAlloc(0, memory.main_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	memory.scratch_ram_size = 100 * 1024 * 1024;
	memory.scratch_ram = VirtualAlloc(0, memory.scratch_ram_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!memory.eng_ram || !memory.perm_ram || !memory.main_ram || !memory.scratch_ram)
	{
		tdDisplayError("Could not allocate game memory", 0);
		exit(1);
	}
	InitPlatform(&memory);

#ifdef _PROFILE_
	tdArrayInit(timed_blocks, 200);
#endif
	RAWINPUTDEVICE input_devices[2];
	input_devices[0].usUsagePage = 1;
	input_devices[0].usUsage = 2;
	input_devices[0].dwFlags = 0;
	input_devices[0].hwndTarget = hwnd_main;
	input_devices[1].usUsagePage = 1;
	input_devices[1].usUsage = 6;
	input_devices[1].dwFlags = RIDEV_NOLEGACY;
	input_devices[1].hwndTarget = hwnd_main;

	if (!RegisterRawInputDevices(input_devices, 2, sizeof(RAWINPUTDEVICE)))
		return 1;

	LARGE_INTEGER perf_freq_temp;
	QueryPerformanceFrequency(&perf_freq_temp);
	uint64 perf_freq = perf_freq_temp.QuadPart;

	LARGE_INTEGER last_perf_counter, perf_counter;
	QueryPerformanceCounter(&last_perf_counter);

	MSG message;
	global_running = true;

	std::locale::global(std::locale(""));
	OutputDebugStringA(std::locale().name().c_str());

	ShowWindow(hwnd_main, SW_SHOW);
	SetForegroundWindow(hwnd_main);
	SetFocus(hwnd_main);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screen_width;
		dmScreenSettings.dmPelsHeight = screen_height;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	}

	log_file = fopen("game.log", "a");
	if (!log_file) return 1;

#if _PROFILE_
	TdTimedBlockCounter frame_counter = { 1, 0, 0, 0, 0, "Frame" };
	tdArrayAdd(timed_blocks, frame_counter);
	DWORD64 start_cycles = __rdtsc();
#endif

	while (global_running)
	{
		BOOL result;

		TdInputState *input = GetInput();
		TdPoint2 mpos = input->mouse.mouse_pos;
		bool mb_left_down = input->mouse.mb_left.button_ended_down;
		bool mb_middle_down = input->mouse.mb_middle.button_ended_down;
		bool mb_right_down = input->mouse.mb_right.button_ended_down;
		memset(&input->mouse, 0, sizeof(TdMouseState));
		input->mouse.mouse_pos = mpos;
		input->mouse.mb_left.button_ended_down = mb_left_down;
		input->mouse.mb_middle.button_ended_down = mb_middle_down;
		input->mouse.mb_right.button_ended_down = mb_right_down;
		memcpy(input->keyboard.prev_state, input->keyboard.key_state, 256);

		while ((result = PeekMessage(&message, 0, 0, 0, PM_REMOVE)) > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (result >= 0)
		{
			QueryPerformanceCounter(&perf_counter);
			uint64 delta = perf_counter.QuadPart - last_perf_counter.QuadPart;
			double delta_secs = (double)delta / perf_freq;
			last_perf_counter = perf_counter;
#if _PROFILE_
			DWORD64 cycles = __rdtsc();
			timed_blocks[0].total_cycles = cycles - start_cycles;
			start_cycles = cycles;
#endif
			RunOneFrame(delta_secs);

			++platform_frame_count;
		}
		else
		{
			global_running = false;
		}
	}

	ApplicationFree();

	return 0;
}
