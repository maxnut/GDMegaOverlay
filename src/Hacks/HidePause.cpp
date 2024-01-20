#include <cocos2d.h>
#include "HidePause.h"
#include "../Settings.h"
#include "../Common.h"
#include "../utils.hpp"

void HidePause::initHooks()
{
	MH_CreateHook(GetProcAddress((HMODULE)utils::cc_base, "?update@CCScheduler@cocos2d@@UAEXM@Z"),
				  CCSchedulerUpdateHook, reinterpret_cast<void**>(&CCSchedulerUpdate));
}

void __fastcall HidePause::CCSchedulerUpdateHook(cocos2d::CCScheduler* self, void*, float dt)
{
	auto playLayer = MBO(cocos2d::CCLayer*, Common::gameManager, 0x198);
	auto runningScene = playLayer ? playLayer->getParent() : nullptr;

	// bool m_isPaused; [PlayLayer + 0x2F17]
	if (playLayer)
	{
		auto uiLayer = reinterpret_cast<cocos2d::CCLayer*>(playLayer->getChildren()->objectAtIndex(8));
		reinterpret_cast<cocos2d::CCMenu*>(
			uiLayer->getChildren()->objectAtIndex(0)
		)->setVisible(!Settings::get<bool>("general/hide_pause/button"));

		if (
			MBO(bool, MBO(cocos2d::CCLayer*, Common::gameManager, 0x198), 0x2F17) &&
			runningScene && runningScene->getChildrenCount() > 1
		) {
			auto pauseLayer = reinterpret_cast<cocos2d::CCLayer*>(runningScene->getChildren()->objectAtIndex(1));

			if (pauseLayer)
				pauseLayer->setVisible(!Settings::get<bool>("general/hide_pause/menu"));
		}
	}


	CCSchedulerUpdate(self, dt);
}
