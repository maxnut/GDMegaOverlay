#include "StartposSwitcher.h"
#include "../Common.h"

#include "../util.hpp"

#include <cocos2d.h>
#include <iostream>

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace StartposSwitcher;
using namespace geode::prelude;

class $modify(PlayLayer)
{
	void onQuit()
	{
		startposObjects.clear();
		PlayLayer::onQuit();
	}
};

void StartposSwitcher::change(bool right)
{
	if (!GameManager::get()->getPlayLayer() || !Mod::get()->getSavedValue<bool>("level/startpos_switcher") || startposObjects.size() <= 0)
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
	int* startPosCheckpoint = (int*)GameManager::get()->getPlayLayer() + 2949;
	*startPosCheckpoint = 0;

	if (!startPosObject && index != -1)
		return;

	reinterpret_cast<void(__thiscall*)(PlayLayer*, StartPosObject*)>(base::get() + 0x199E90)(GameManager::get()->getPlayLayer(), startPosObject);

	GameManager::get()->getPlayLayer()->resetLevel();

	// apparently you have to start music manually since gd only does it if you dont have a startpos???? (see
	// playlayer_resetlevel line 272 in ida)
	GameManager::get()->getPlayLayer()->startMusic();
}

StartPosObject* StartposSwitcher::createHook()
{
	StartPosObject* sp = reinterpret_cast<StartPosObject*(__thiscall*)()>(base::get() + 0x3A7850)();

	if (sp && Mod::get()->getSavedValue<bool>("level/startpos_switcher"))
	{
		startposObjects.push_back(sp);

		index = startposObjects.size() - 1;
	}

	return sp;
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void*>(base::get() + 0x3A7850), &createHook, "StartPosObject::create", tulip::hook::TulipConvention::Thiscall);
}