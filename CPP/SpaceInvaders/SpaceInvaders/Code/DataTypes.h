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
	static const TdPoint2 ufo_size;
	static const TdPoint2 defender_size;
	static const Vector2 defender_speed;
	static const TdPoint2 fleet_size;
	static const TdPoint2 bullet_size;
	static const Vector2 bullet_speed;
};

struct Particles
{
	Vector2 pos;
};

struct Bullet
{
	int32 alive;
	Vector2 pos;
};

struct Invader
{
	int32 alive;
};

struct UFO
{
	Vector2 pos;
};

struct DefenderShip
{
	Vector2 pos;
};

struct GameInstance
{
	enum { in_MovingAcross, in_CreepingDown	} invader_fleet_state;
	DefenderShip *ship;
	uint32 invader_count;
	uint32 invader_alive_count;
	Invader *invader_fleet;
	Vector2 invader_fleet_pos;
	float invader_fleet_speed;
	float invader_fleet_y_target;
	UFO *ufo;
	Vector2 invader_fleet_extent;
	uint32 bullet_count;
	Bullet *bullets;
	TdPcg32Random* stars_rng;
	uint64 stars_rng_seed1, stars_rng_seed2;
	uint32 wave;
	uint32 high_score;
	double new_fleet_timer;
};

struct Player
{
	enum PlayerMode	{ pm_Menu, pm_Play };
	uint32 score;
	PlayerMode mode;
	uint32 lives;
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
	TdInputState input;
	TdGamePadState prev_gamepad;
	double elapsed_scale;
	bool exit_game;
};

}
