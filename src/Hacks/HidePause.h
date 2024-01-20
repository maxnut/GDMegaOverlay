#pragma once
#include <cocos2d.h>

namespace HidePause
{
	void initHooks();

	inline void(__thiscall* CCSchedulerUpdate)(cocos2d::CCScheduler*, float);
	void __fastcall CCSchedulerUpdateHook(cocos2d::CCScheduler*, void*, float);

	// hide pause button is a patch
}
