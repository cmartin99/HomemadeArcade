
static GameMemory* memory;
static char temp_text[1000];
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

const TdPoint2 GameConsts::invader_size = {54, 54};
const TdPoint2 GameConsts::invader_spacing = {85, 72};
const float GameConsts::invader_fleet_creep_speed = 20;
const float GameConsts::invader_fleet_creep_distance = 20;
const TdPoint2 GameConsts::ufo_size = {132, 32};
const TdPoint2 GameConsts::defender_size = {64, 64};
const Vector2 GameConsts::defender_speed = {200, 0};
const TdPoint2 GameConsts::fleet_size = {12, 6};
const TdPoint2 GameConsts::bullet_size = {4, 24};
const Vector2 GameConsts::bullet_speed = {0, 1500};

// -----------------------------------------------------------------------------------------------------------------------------

extern void ExitGame();
typedef void (*InputEventHandler)();
void RenderDebug(TdWindow*, TdSpriteBatch*, TdPoint2);
void GameInstanceNew(Player*);
const char *GuiText(GuiID);
void ConvertXInput(TdGamePadState&, const TdGamePadState&, XINPUT_GAMEPAD);

// -----------------------------------------------------------------------------------------------------------------------------

template<typename T>
ALWAYS_INLINE void memclear(T* p)
{
    memset(p, 0, sizeof(T));
}

bool IsGameInitialized()
{
    return memory != nullptr && memory->perm_ram != nullptr;
}

ALWAYS_INLINE GameState* GetGameState()
{
    return (GameState*)memory->perm_ram;
}

void InitPlatform(void* game_memory)
{
	assert(game_memory);
	memory = (GameMemory*)game_memory;
}

TdInputState* GetInput()
{
	return &((GameState*)memory->perm_ram)->input;
}

ALWAYS_INLINE double SetTimer(double base, double time)
{
	return base + time;
}

TdRect GetViewportSize(uint32 player_id, uint32 player_count)
{
	auto game_state = GetGameState();

	int w = player_count == 1
		? game_state->vulkan->surface_width
		: player_count < 5
				? game_state->vulkan->surface_width / 2
				: game_state->vulkan->surface_width / 4;

	int h = player_count == 1
		? game_state->vulkan->surface_height
		: player_count < 5
				? game_state->vulkan->surface_height / 2
				: game_state->vulkan->surface_height / 4;

	int x = player_count == 1
				? 0
				: player_count < 5
					  ? w * (player_id % 2)
					  : w * (player_id % 4);

	int y = player_count == 1
				? 0
				: player_count < 5
					? h * (int)(player_id / 2)
					: h * (int)(player_id / 4);

	return {x, y, w, h};
}
