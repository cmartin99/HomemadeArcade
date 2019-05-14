#pragma once

using namespace eng;

namespace NewGame {

const float g_PI = glm::pi<float>();
const double g_write_socket_freq = 0.05;
const float g_mass_lerp_time = 0.5f;
const float g_shot_mass = 10.f;
const float g_virus_mass = 100.f;
const float g_spawner_mass = 222.f;
const double g_shot_repeat_speed = 0.15;
const float g_split_speed = 400.f;


static char commas1[20];
static char commas2[20];
static char commas3[20];
static char commas4[20];
static char commas5[20];
static char commas6[20];
static char commas7[20];
static char commas8[20];
static char commas9[20];
static char commas10[20];
static char commas11[20];
static char commas12[20];

static const int agar_color_count = 20;
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

typedef uint16 KeyFlags;

struct Keys
{
	KeyFlags split : 1;
	KeyFlags shoot : 1;
	KeyFlags pause_movement : 1;
	KeyFlags debug_merge : 1;
	KeyFlags debug_dbl_mass : 1;
	KeyFlags debug_add_mass_1k : 1;
	KeyFlags debug_rem_mass_1k : 1;
	KeyFlags debug_speed_boost : 1;
	KeyFlags debug_goto_pos : 1;
	KeyFlags debug_slow_time : 1;
};

struct Agar
{
	union {	uint8 enabled; uint8 color_id; uint8 state; };
	int16 x, y;
};

struct AgarMoving
{
	union {	uint8 enabled; uint8 color_id; uint8 state; };
	Vector2 pos;
	Vector2 vel;
	int32 agar_id;
};

struct Mass
{
	union {	uint8 enabled; uint8 color_id; uint8 state; };
	Vector2 pos;
	Vector2 vel;
	float mass;
};

struct Entity : Mass
{
	float mass_target, mass_change_orig;
	double merge_time;
	double mass_change_time_orig;
};

struct Gamer;

struct BotData
{
	double time_to_split;
	double time_to_shoot;
	double time_to_change_dir;
	Entity* biggest_cell;
	Entity* closest_cell;
	Entity* closest_enemy_cell;
	float closest_enemy_cell_dist;
};

struct PlayerStateData
{
	uint8 state_change;
	int16 mouse_pos_x;
	int16 mouse_pos_y;
	Keys keys;
};

struct Player
{
	uint16 type;
	Gamer* gamer;
	Vector2 center;
	Vector4 extent;
	double next_shot_time;
	int32 total_mass;
	int32 cell_count;
	PlayerStateData state_data;
};

struct Connection
{
	SOCKET socket;
	sockaddr_in socket_addr;
	int32 addr_len;
	Gamer *gamer;
};

struct Gamer
{
	int16 gamer_id;
	Player* player;
	uint8 movement_paused : 1;
};

struct Renderer
{
	TdVkInstance* vulkan;
	TdSpriteBatch* sprite_batch;
	TdSpriteBatch* gui_sprite_batch;
	TdImGui* gui;
	VkViewport viewport;
	VkRect2D scissor_rect;
	int32 log_offset_y;
};

struct Sim
{
	uint8 is_active : 1;
	uint8 is_paused : 1;
	uint8 leader_board_changed : 1;
	double seconds;
	double total_seconds;
	double sim_speed;
	TdPcg32Random rng;
	int32 world_size;
	int32 max_world_size;
	int32 max_player_cells;
	int32 max_mass_per_cell;
	int32 agar_cap;
	int32 virus_cap;
	int32 spawner_cap;
	TdArray<Gamer> gamers;
	TdArray<Player> players;
	TdArray<Entity> player_cells;
	TdArray<Agar> agar;
	TdArray<int32> agar_changed;
	TdArray<int32> agar_changed_thread_buffer;
	TdArray<int32> agar_changed_socket_buffer;
	TdArray<AgarMoving> agar_moving;
	TdArray<Mass> shots;
	TdArray<uint16> shots_changed;
	TdArray<Mass> viruses;
	TdArray<uint16> viruses_changed;
	TdArray<Entity> spawners;
	TdArray<uint16> spawners_changed;
	TdArray<uint16> spawners_emitting;
	TdArray<Connection> connections;
	TdArray<int16> leader_board;
	TdArray<int16> leader_board_prev;
	TdArray<BotData> bot_data;
	char* player_name_text;
	char* player_mass_text;
	int32 player_count;
	int32 threads_running;
	int32 packet_cntr;
	double next_socket_write_time;
	int32 agar_changed_largest_count;
	int32 shots_changed_largest_count;
	int16 debug_rnd_latency;
};

enum PacketType
{
	pk_None,
	pk_SimState,
	pk_ClientState,
	pk_Message,
	pk_ConnectRequest,
	pk_ConnectConfirm,
	pk_PlayRequest,
	pk_PlayConfirm,
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
	WSADATA wsa;
	SOCKET socket;
	sockaddr_in socket_addr;
	char ip_address[16];
	uint16 port;
	uint8* packet;
	int32 packet_size;
	bool exit_on_esc;
	bool reset_time;
	bool winsock_initialized;
	int32 max_log_error_text_len;
	int32 max_log_errors;
	int32 curr_log_error;
	char* log_error_text;
	int32 bytes_sent_per_frame;
	int32 bytes_sent_per_second;
	int32 bytes_sent_last_second;
	int32 bytes_recv_per_frame;
	int32 bytes_recv_per_second;
	int32 bytes_recv_last_second;
	double bytes_net_timer;
	int32 packet_size_largest;
};

}
