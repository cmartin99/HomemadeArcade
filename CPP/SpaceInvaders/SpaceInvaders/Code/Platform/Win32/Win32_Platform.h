#pragma once

namespace SpaceInvaders_Platform {

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

}

namespace SpaceInvaders {

// The platform layer expects these methods to be implemented by game
void InitPlatform(void *game_memory);
eng::TdInputState* GetInput();
bool IsGameInitialized();
void RunOneFrame(double);
void HandleRawInput(uint16 vkey, bool key_released);
void GameInstanceNew(eng::TdVkInstance&);
void GameInstanceFree();

}