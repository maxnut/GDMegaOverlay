#include "StartposSwitcher.h"
#include "../Common.h"
#include "../Settings.h"
#include "../utils.hpp"

#include <cocos2d.h>
#include <MinHook.h>
#include <iostream>

void StartposSwitcher::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2e5650), playLayerQuitHook,
				  reinterpret_cast<void**>(&playLayerQuit));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x3A0C50), createHook,
				  reinterpret_cast<void**>(&create));
}

void StartposSwitcher::change(bool right)
{
	if (!Common::getBGL() || !Settings::get<bool>("level/startpos_switcher") || startposObjects.size() <= 0)
		return;

	if (right)
		index++;
	else
		index--;

	if (index <= -2)
		index = startposObjects.size() - 1;
	else if (index >= static_cast<int>(startposObjects.size()))
		index = -1;

	int* startPosObject = index == -1 ? nullptr : startposObjects[index];

	// delete the startposcheckpoint (see playlayer_resetlevel line 148 in ida)
	int* startPosCheckpoint = (int*)Common::getBGL() + 2941;
	*startPosCheckpoint = 0;

	if(!startPosObject && index != -1)
		return;

	reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*, int*)>(utils::gd_base + 0x195FC0)(Common::getBGL(), startPosObject);

	reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E42B0)(Common::getBGL());

	// apparently you have to start music manually since gd only does it if you dont have a startpos???? (see
	// playlayer_resetlevel line 272 in ida)
	reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(utils::gd_base + 0x2E5570)(Common::getBGL());
}

int __fastcall StartposSwitcher::playLayerQuitHook(int* self, void*)
{
	startposObjects.clear();
	return StartposSwitcher::playLayerQuit(self);
}

int* __fastcall StartposSwitcher::createHook()
{
	int* sp = StartposSwitcher::create();

	if (sp && Settings::get<bool>("level/startpos_switcher"))
	{
		startposObjects.push_back(sp);

		index = startposObjects.size() - 1;
	}

	return sp;
}