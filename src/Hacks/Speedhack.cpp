#include "Speedhack.h"
#include "../Common.h"
#include "../Macrobot/Macrobot.h"
#include "../Settings.h"
#include "../Macrobot/AudioRecord.h"

#include <MinHook.h>

void __fastcall Speedhack::CCSchedulerUpdateHook(void* self, void*, float dt)
{
	float speedhack =
		Settings::get<bool>("general/speedhack/enabled") ? Settings::get<float>("general/speedhack/value") : 1.f;

	dt *= speedhack;

	if (Macrobot::playerMode != -1 && !AudioRecord::recording)
	{
		float framerate;

		if (Settings::get<bool>("general/fps/enabled"))
			framerate = Settings::get<float>("general/fps/value", 60.f);
		else
			framerate = MBO(float, Common::gameManager, 900);

		dt = 1.0f / (framerate * speedhack);
		dt *= speedhack;
	}

	CCSchedulerUpdate(self, dt);
}

void Speedhack::initHooks()
{
	MH_CreateHook(GetProcAddress((HMODULE)utils::cc_base, "?update@CCScheduler@cocos2d@@UAEXM@Z"),
				  CCSchedulerUpdateHook, reinterpret_cast<void**>(&CCSchedulerUpdate));
}