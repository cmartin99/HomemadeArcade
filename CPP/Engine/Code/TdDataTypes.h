#pragma once
#include "TdBase.h"
#include "TdArray.h"
#include "TdPoint.h"

namespace Colors {
const Color None(0);
const Color Black(0,0,0,1);
const Color White(1);
const Color Gray(0.5f, 0.5f, 0.5f, 1.0f);
const Color LightGray(0.75f, 0.75f, 0.75f, 1.0f);
const Color DarkGray(0.25f, 0.25f, 0.25f, 1.0f);
const Color Red(1, 0, 0, 1);
const Color Green(0, 1, 0, 1);
const Color Blue(0, 0, 1, 1);
const Color Yellow(1, 0.95, 0.02, 1);
const Color Cyan(0, 0.7, 1, 1);
const Color Orange(1, 0.4, 0, 1);
const Color SunOrange(1, 0.623, 0, 1);
}

namespace eng {

#ifdef _PROFILE_
struct TdTimedBlockCounter
{
	int total_hits;
	int last_frame_hits;
	DWORD64 total_cycles;
	DWORD64 last_frame_cycles;
	DWORD64 last_cycles;
	DWORD64 lowest_cycles;
	DWORD64 highest_cycles;
	const char* name;
};

extern TdArray<TdTimedBlockCounter> timed_blocks;

struct TdTimedBlock
{
	const char* name;
	DWORD64 start_cycles;
	DWORD64 delta_cycles;
	bool once;
	static bool timed_blocks_paused;

	TdTimedBlock(const char* name, bool once = false) : name(name), once(once), start_cycles(__rdtsc())
	{
	}

	~TdTimedBlock()
	{
		if (!timed_blocks_paused)
		{
			DWORD64 cycles = __rdtsc() - start_cycles;

			TdTimedBlockCounter* blocks = timed_blocks.ptr;
			int32 count = timed_blocks.count;
			TdTimedBlockCounter* p;

			for (int32 i = 0; i < count; ++i)
			{
				if (strcmp(blocks[i].name, name) == 0)
				{
					if (!once)
					{
						p = blocks + i;
						++p->total_hits;
						p->last_cycles = cycles;
						p->total_cycles += cycles;
						if (cycles < p->lowest_cycles) p->lowest_cycles = cycles;
						else if (cycles > p->highest_cycles || p->highest_cycles > cycles * 10) p->highest_cycles = cycles;
					}
					return;
				}
			}

			TdTimedBlockCounter a = { 1, 0, cycles, 0, cycles, cycles, cycles, name };
			assert(timed_blocks.count < timed_blocks.cap);
			tdArrayAdd(timed_blocks, a);
		}
	}
};
#endif

typedef HANDLE TdMutexHandle;

struct TdMutexLock
{
	TdMutexLock(TdMutexHandle handle) : handle(handle)
	{
		WaitForSingleObject(handle, INFINITE);
	}
	~TdMutexLock()
	{
		ReleaseMutex(handle);
	}
private:
	TdMutexHandle handle;
};

struct TdRect
{
	int32 x, y, w, h;
};

struct TdRay
{
	Vector3 orig;
	Vector3 dir;
};

struct TdBoundingBox
{
	Vector3 min;
	Vector3 max;
};

struct TdBoundingBoxP3
{
	TdPoint3 min;
	TdPoint3 max;
	TdPoint3 GetSize() const { return max - min; }
};

struct TdButtonState
{
	bool button_ended_down;
	uint16 half_transition_count;
};

struct TdKeyState
{
	uint16 key;
	uint16 half_transition_count;
};

struct TdGamePadState
{
	float trigger_left;
	float trigger_right;
	Vector2 thumb_left;
	Vector2 thumb_right;
	TdButtonState a, b, x, y;
	TdButtonState dpad_up;
	TdButtonState dpad_down;
	TdButtonState dpad_left;
	TdButtonState dpad_right;
	TdButtonState back;
	TdButtonState start;
	TdButtonState shoulder_left;
	TdButtonState shoulder_right;
	TdButtonState thumb_left_pressed;
	TdButtonState thumb_right_pressed;
};

struct TdMouseState
{
	TdPoint2 mouse_pos;
	TdButtonState mb_left;
	TdButtonState mb_middle;
	TdButtonState mb_right;
};

struct TdKeyboardState
{
	int16 key_state[256];
	int16 prev_state[256];
	TdKeyState keys[10];
};

struct TdInputState
{
	double seconds;
	TdMouseState mouse;
	TdKeyboardState keyboard;
	TdGamePadState gamepad;
};

}