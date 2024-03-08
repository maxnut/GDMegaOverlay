#include "ReplayLastCheckpoint.h"
#include "../Common.h"

#include "../util.hpp"
#include "../Settings.hpp"

#include <Geode/Geode.hpp>
#include <cocos2d.h>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

using namespace geode::prelude;
using namespace ReplayLastCheckpoint;

class $modify(PlayLayer)
{
	void levelComplete()
	{
		levelCompleted = true;
		PlayLayer::levelComplete();
	}

	void resetLevel()
	{
		levelCompleted = false;
		PlayLayer::resetLevel();
	}
};

class $modify(ButtonsClass, EndLevelLayer)
{
	void onReset(CCObject* sender)
	{
		levelCompleted = false;

		// use m_fields to remove this amazing chain
		this->getParent()->getParent()->removeFromParent();

		PlayLayer::get()->resetLevel();
	}

	void customSetup()
	{
		EndLevelLayer::customSetup();

		if (!Settings::get<bool>("level/replay_checkpoint") || !PlayLayer::get()->m_isPracticeMode) return;

		auto layer = reinterpret_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
		CCMenu* buttonsMenu = nullptr;


		CCObject* object;
		CCARRAY_FOREACH(layer->getChildren(), object)
		{
			if (auto menu = typeinfo_cast<CCMenu*>(object); menu && menu->getChildrenCount() > 1)
				buttonsMenu = menu;
		}

		auto practiceSprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
		practiceSprite->setScale(1.f);
		auto practiceButton = CCMenuItemSpriteExtra::create(
			practiceSprite,
			buttonsMenu,
			menu_selector(ButtonsClass::onReset)
		);

		if (buttonsMenu->getChildrenCount() == 2)
		{
			getChild<CCMenuItemSpriteExtra>(buttonsMenu, 0)->setPositionX(-100.f);
			getChild<CCMenuItemSpriteExtra>(buttonsMenu, 1)->setPositionX(100.f);
		}
		else
		{
			getChild<CCMenuItemSpriteExtra>(buttonsMenu, 0)->setPositionX(-130.f);
			getChild<CCMenuItemSpriteExtra>(buttonsMenu, 1)->setPositionX(130.f);
			getChild<CCMenuItemSpriteExtra>(buttonsMenu, 2)->setPositionX(-45.f);
			practiceButton->setPositionX(45.f);
		}

		practiceButton->setPositionY(-125.f);
		buttonsMenu->addChild(practiceButton);
	}
};
