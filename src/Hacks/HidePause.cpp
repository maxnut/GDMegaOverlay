#include "HidePause.h"

#include <Geode/modify/CCScheduler.hpp>

#include <Geode/Utils.hpp>

#include <Geode/Geode.hpp>

#include "../util.hpp"
#include "../Settings.hpp"

using namespace geode::prelude;

class $modify(CCScheduler)
{
	void update(float dt)
	{
		auto playLayer = GameManager::sharedState()->getPlayLayer();
		auto runningScene = playLayer ? playLayer->getParent() : nullptr;

		if (playLayer)
		{
			auto uiLayer = reinterpret_cast<cocos2d::CCLayer*>(playLayer->getChildren()->objectAtIndex(8));

			if (typeinfo_cast<EndLevelLayer*>(uiLayer))
				uiLayer = reinterpret_cast<cocos2d::CCLayer*>(playLayer->getChildren()->objectAtIndex(9));

			if (uiLayer && uiLayer->getChildrenCount() > 0)
				reinterpret_cast<cocos2d::CCMenu*>(
					uiLayer->getChildren()->objectAtIndex(0)
				)->setVisible(!Settings::get<bool>("general/hide_pause/button"));


			// bool m_isPaused; [PlayLayer + 0x2F17]
			if (
				MBO(bool, playLayer, 0x2F17) &&
				runningScene && runningScene->getChildrenCount() > 1
			) {
				auto pauseLayer = reinterpret_cast<cocos2d::CCLayer*>(runningScene->getChildren()->objectAtIndex(1));

				if (pauseLayer)
					pauseLayer->setVisible(!Settings::get<bool>("general/hide_pause/menu"));
			}
		}

		CCScheduler::update(dt);
	}
};
