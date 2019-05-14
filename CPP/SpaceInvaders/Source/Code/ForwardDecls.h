#pragma once
#include "Common.h"

namespace NewGame {

typedef int32 (*WritePacketFunc)(Sim*, uint8*);

void SimNew();
void SimEnd();
void UpdateSim(Sim*);
Player* PlayerNew(Sim*, Gamer* = nullptr, Player* = nullptr);
BotData* BotDataFromPlayer(const Sim*, const Player*);
bool IsBot(const Player*);
void AddBot(Sim*, Player* = nullptr);
void RemoveBot(Sim*);
void UpdateBots(Sim*);
void InitApplication(AppMemory&, eng::TdVkInstance*, char*, uint16);
void CloseApplication();
void HandleMouseWheel(int16);
void InputClearMouseHandlers();
int CreateServerSocket();
void ReadSockets();
void WriteSockets(WritePacketFunc);
int32 WriteCurrentSimStateToPacket(Sim*, uint8* packet);
VkResult RenderGame();
void UpdateTestClient();

ALWAYS_INLINE double GetTimer(const AppState* app_state, double base, double time)
{
	return base + time;
}

}