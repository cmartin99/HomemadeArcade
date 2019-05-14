#pragma once

using namespace eng;

namespace NewGame {

static char commas1[20];
static char commas2[20];
static char commas3[20];
static char commas4[20];
static char commas5[20];
static char commas6[20];
static int last_debug_y;

struct AppConsts
{
	static const int32 save_version = 100000;
};

struct AppMemory
{
	uint64 eng_ram_size;
	void* eng_ram;
	uint64 perm_ram_size;
	void* perm_ram;
	uint64 main_ram_size;
	void* main_ram;
	uint64 scratch_ram_size;
	void* scratch_ram;
};

struct DebugData
{
	// 0 = no display
	// 1 = top line
	// 2 = timed blocks
	int32 debug_verbosity;
	int32 draw_primitive_count;
};

enum ScreenMode
{
	sm_TitleMenu,
	sm_Gameplay,
};

struct Gamer;

struct Player
{
	uint16 type;
	Gamer* gamer;
};

struct Gamer
{
	ScreenMode screen_mode;
	uint64 gamer_id;
	Player* player;
	uint16 gui_interacted : 1;
	TdImGui* gui;
	VkViewport viewport;
	VkRect2D scissor_rect;
};

struct Renderer
{
	TdVkInstance* vulkan;
	TdSpriteBatch* sprite_batch;
	TdSpriteBatch* gui_sprite_batch;
};

struct Sim
{
	uint8 is_active : 1;
	uint8 is_paused : 1;
	double seconds;
	double total_seconds;
	double sim_speed;
	TdPcg32Random rng;
	TdArray<Player> players;
};

struct AppState
{
	double seconds;
	double total_seconds;
	Sim sim;
	TdArray<Gamer> gamers;
	TdPcg32Random rng;
	bool exit_app;
	Renderer renderer;
	TdInputState input;
	int32 rng_seed, rng_inc;
	time_t rng_time;
	TdMemoryArena perm_arena;
	TdMemoryArena main_arena;
	TdMemoryArena scratch_arena;
	DebugData debug_data;
};

}
