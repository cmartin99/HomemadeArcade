#pragma once

using namespace eng;

namespace NewGame {

static char commas1[20];
static char commas2[20];
static char commas3[20];
static char commas4[20];
static char commas5[20];
static int last_debug_y;

struct AppConsts
{
	static const int32 save_version = 100000;
	static const char *save_path;
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

struct Gamer;

struct Player
{
	uint16 type;
	Gamer* gamer;
};

struct Gamer
{
	int16 gamer_id;
	Player* player;
};

struct Renderer
{
	TdVkInstance* vulkan;
	TdSpriteBatch* sprite_batch;
	TdSpriteBatch* gui_sprite_batch;
	TdImGui* gui;
	VkViewport viewport;
	VkRect2D scissor_rect;
};

struct Sim
{
	uint8 is_active : 1;
	uint8 is_paused : 1;
	double seconds;
	double total_seconds;
	double sim_speed;
	TdPcg32Random rng;
	TdArray<Gamer> gamers;
	TdArray<Player> players;
};

struct AppState
{
	double seconds;
	double total_seconds;
	Sim sim;
	bool exit_app;
	TdPcg32Random rng;
	uint64 frame_count;
	TdMemoryArena perm_arena;
	TdMemoryArena main_arena;
	TdMemoryArena scratch_arena;
	Renderer renderer;
	DebugData debug_data;
	TdInputState input;
	int32 rng_seed, rng_inc;
	time_t rng_time;
	bool exit_on_esc;
	bool reset_time;
};

}
