#include "ReplayLastCheckpoint.h"
#include "../Common.h"
#include "../Settings.h"
#include "../utils.hpp"

#include <MinHook.h>

// ok so for whatever reason on windows instead of calling togglepracticemode rob calls resetlevelfromstart. on android
// he calls togglepracticemode tho :)
void __fastcall ReplayLastCheckpoint::playLayerResetLevelFromStartHook(void* self, void*)
{
	bool replay = Settings::get<bool>("level/replay_checkpoint");
    

	if (replay && levelCompleted)
	{
		reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E42B0)(Common::getBGL());
        *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(Common::getBGL()) + 10868) = true;
        levelCompleted = false;
		return;
	}
    levelCompleted = false;
	playLayerResetLevelFromStart(self);
}

void __fastcall ReplayLastCheckpoint::playLayerLevelCompleteHook(void* self, void*)
{
	levelCompleted = true;
	playLayerLevelComplete(self);
}

int __fastcall ReplayLastCheckpoint::playLayerResetLevelHook(void* self, void*)
{
	levelCompleted = false;
	return playLayerResetLevel(self);
}

void ReplayLastCheckpoint::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E8160), playLayerResetLevelFromStartHook,
				  reinterpret_cast<void**>(&playLayerResetLevelFromStart));

    MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2DBC80), playLayerLevelCompleteHook,
				  reinterpret_cast<void**>(&playLayerLevelComplete));

    MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E8200), playLayerResetLevelHook,
				  reinterpret_cast<void**>(&playLayerResetLevel));
}