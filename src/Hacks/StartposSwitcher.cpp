#include "StartposSwitcher.h"
#include "../Common.h"

#include "../util.hpp"
#include "../Settings.hpp"

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

	bool init(GJGameLevel* level, bool unk1, bool unk2)
	{
		startposObjects.clear();
		bool res = PlayLayer::init(level, unk1, unk2);
		cocos2d::CCObject* obj;
		CCARRAY_FOREACH(this->m_objects, obj)
		{
			auto g = reinterpret_cast<GameObject*>(obj);

			int id = MBO(int, g, 900);

			if (id == 31)
				startposObjects.push_back(reinterpret_cast<StartPosObject*>(obj));
		}

		index = startposObjects.size() - 1;

		return res;
	}
};

void StartposSwitcher::change(bool right)
{
	if (!GameManager::get()->getPlayLayer() || !Settings::get<bool>("level/startpos_switcher") || startposObjects.size() <= 0)
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