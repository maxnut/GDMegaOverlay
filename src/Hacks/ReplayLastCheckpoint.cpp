#include "ReplayLastCheckpoint.h"
#include "../utils.hpp"
#include "../Settings.h"
#include "../Common.h"

#include <MinHook.h>

//ok so for whatever reason on windows instead of calling togglepracticemode rob calls resetlevelfromstart. on android he calls togglepracticemode tho :)
void __fastcall ReplayLastCheckpoint::playLayerResetLevelFromStartHook(void* self, void*)
{
    bool replay = Settings::get<bool>("level/replay_checkpoint");

    if(replay)
    {
        reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E42B0)(Common::getBGL());
        return;
    }
    
    playLayerResetLevelFromStart(self);
}

void ReplayLastCheckpoint::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E4210), playLayerResetLevelFromStartHook,
				  reinterpret_cast<void**>(&playLayerResetLevelFromStart));
}