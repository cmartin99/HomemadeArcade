#pragma once

namespace SpaceInvaders {

struct GameMemory
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
	uint32 debug_verbosity;
	uint32 last_debug_height;
};

struct GameConsts
{
	static const TdPoint2 invader_size;
	static const TdPoint2 invader_spacing;
	static const float invader_fleet_creep_speed;
	static const float invader_fleet_creep_distance;
	static const Vector2 invader_bomb_speed;
	static const float ufo_speed;
	static const TdPoint2 ufo_size;
	static const Vector2 ufo_bomb_speed;
	static const TdPoint2 defender_size;
	static const Vector2 defender_speed;
	static const double defender_respawn_time;
	static const TdPoint2 fleet_size;
	static const TdPoint2 bullet_size;
	static const Vector2 bullet_speed;
	static const uint32 max_particles = 1000;
};

struct Particle
{
	float age;
	Vector2 pos;
	Vector2 vel;
	Vector2 size;
	Color color;
};

struct Bullet
{
	int32 alive;
	Vector2 pos;
	Vector2 vel;
	Color color;
};

struct Invader
{
	int32 alive;
};

struct UFO
{
	Vector2 pos;
	Vector2 vel;
};

struct DefenderShip
{
	Vector2 pos;
};

struct GameInstance
{
	enum { in_MovingAcross, in_CreepingDown, in_Waiting } invader_fleet_state, prev_invader_fleet_state;
	DefenderShip *ship;
	uint32 invader_alive_count;
	Invader *invader_fleet;
	float invader_fleet_speed;
	Vector2 invader_fleet_pos;
	float invader_fleet_y_target;
	int invader_anim_frame;
	Invader *invader_last_to_drop_bomb;
	UFO *ufo;
	Particle *particles;
	Vector3 invader_fleet_extent;
	uint32 bullet_count;
	Bullet *bullets;
	TdPcg32Random* stars_rng;
	uint64 stars_rng_seed1, stars_rng_seed2;
	uint32 wave;
	uint32 high_score;
	double new_fleet_timer;
	double gameover_timer;
	double ufo_spawn_timer;
	double ufo_bomb_timer;
	double invader_bomb_timer;
	double invader_anim_timer;
	double fire_button_timer;
};

struct Player
{
	enum PlayerMode	{ pm_Menu, pm_Paused, pm_Play };
	enum GameplayMode { gm_Play, gm_Respawn };
	uint32 score;
	PlayerMode mode;
	GameplayMode gameplay_mode;
	uint32 lives;
	double respawn_timer;
	VkViewport viewport;
	VkRect2D scissor_rect;
	ImGui gui;
};

struct GameState
{
	double seconds;
	double total_seconds;
	GameInstance* instance;
	Player *player;
	TdPcg32Random* rng;
	TdMemoryArena perm_arena;
	TdMemoryArena main_arena;
	TdMemoryArena scratch_arena;
	uint64 frame_count;
	DebugData debug_data;
	TdVkInstance* vulkan;
	TdSpriteBatch* sprite_batch;
	TdSpriteBatch* gui_sprite_batch;
	TdVkTexture sprite_sheet;
	TdInputState input;
	TdInputState input_prev;
	double elapsed_scale;
	bool exit_game;
};

}
