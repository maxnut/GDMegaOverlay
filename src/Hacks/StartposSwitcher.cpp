#include "StartposSwitcher.h"
#include "../Common.h"

#include "../util.hpp"
#include "../Settings.hpp"

#include <cocos2d.h>
#include <iostream>

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/StartPosObject.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

#include "ConstData.h"

using namespace StartposSwitcher;
using namespace geode::prelude;

class $modify(CCKeyboardDispatcher) {
	bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr)
	{
		if (!down || arr)
			return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
		
		int leftKey = Settings::get<int>("level/startpos_switcher/left", ImGuiKey_LeftArrow);
		int rightKey = Settings::get<int>("level/startpos_switcher/right", ImGuiKey_RightArrow);
		if (ConvertKeyEnum(key) == leftKey)
			change(false);
		else if (ConvertKeyEnum(key) == rightKey)
			change(true);

		return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
	}
};

class $modify(StartPosObject)
{
	bool init()
	{
		bool res = StartPosObject::init();
		startposObjects.push_back(this);
		index = startposObjects.size() - 1;
		return res;
	}
};

class $modify(LevelEditorLayer)
{
	bool init(GJGameLevel* level, bool unk)
	{
		startposObjects.clear();

		bool res = LevelEditorLayer::init(level, unk);
		return res;
	}
};

class $modify(PlayLayer)
{
	void onQuit()
	{
		startposObjects.clear();
		PlayLayer::onQuit();
	}

	bool init(GJGameLevel* p0, bool p1, bool p2)
	{
		startposObjects.clear();
		bool res = PlayLayer::init(p0, p1, p2);

		if (startposObjects.size() > 0)
		{
			std::sort(startposObjects.begin(), startposObjects.end(), [](StartPosObject* s0, StartPosObject* s1)
			{
				return s0->getPositionX() < s1->getPositionX();
			});
		}

		return res;
	}
};

void StartposSwitcher::showLabel()
{
	auto size = cocos2d::CCDirector::sharedDirector()->getWinSize();
	if (!startPosLabel)
	{
		startPosLabel = cocos2d::CCLabelBMFont::create("", "bigFont.fnt");
		startPosLabel->retain();
	}

	startPosLabel->setPositionX(size.width / 2.f);
	startPosLabel->setPositionY(25);
	startPosLabel->setZOrder(1000);
	startPosLabel->setScale(0.5f);
	startPosLabel->setOpacity(0);
	PlayLayer::get()->addChild(startPosLabel);

	std::string labelStr = fmt::format("{}/{}", (index + 1), startposObjects.size());

	startPosLabel->setString(labelStr.c_str());

	auto opacityPulseBegin = CCFadeTo::create(0.1f, 200);
	auto opacityPulseWait = CCFadeTo::create(1.5f, 200);
	auto opacityPulseEnd = CCFadeTo::create(0.3f, 0);

	CCArray* actions = CCArray::create();

	actions->addObject(opacityPulseBegin);
	actions->addObject(opacityPulseWait);
	actions->addObject(opacityPulseEnd);

	startPosLabel->runAction(CCSequence::create(actions));
}

void StartposSwitcher::change(bool right)
{
	if (!PlayLayer::get() || !Settings::get<bool>("level/startpos_switcher") || startposObjects.size() <= 0)
		return;

	if (right)
		index++;
	else
		index--;

	if (index <= -2)
		index = startposObjects.size() - 1;
	else if (index >= static_cast<int>(startposObjects.size()))
		index = -1;

	StartPosObject* startPosObject = index == -1 ? nullptr : startposObjects[index];

	// delete the startposcheckpoint (see playlayer_resetlevel line 148 in ida)
	int* startPosCheckpoint = (int*)PlayLayer::get() + 2949;
	*startPosCheckpoint = 0;

	if (!startPosObject && index != -1)
		return;

	PlayLayer::get()->setStartPosObject(startPosObject);

	PlayLayer::get()->resetLevel();

	// apparently you have to start music manually since gd only does it if you dont have a startpos???? (see
	// playlayer_resetlevel line 272 in ida)
	PlayLayer::get()->startMusic();

	showLabel();
}