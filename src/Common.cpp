#include "Common.h"
#include "Hacks/AudioChannelControl.h"
#include "Macrobot/Macrobot.h"
#include "Settings.h"
#include "utils.hpp"
#include <MinHook.h>

#include <fstream>
#include <imgui.h>

cocos2d::CCLayer* Common::getBGL()
{
	return MBO(cocos2d::CCLayer*, gameManager, 408);
}

void Common::calculateFramerate()
{
	float speedhack = Settings::get<float>("general/speedhack/value", 1.f);
	float framerate = 60.f;
	float interval = 60.f;

	if (Settings::get<bool>("general/fps/enabled"))
		framerate = Settings::get<float>("general/fps/value", 60.f);
	else
		framerate = MBO(float, gameManager, 900);

	if (framerate < 60.f)
		framerate = 60.f;

	if (Settings::get<bool>("general/speedhack/enabled") && Macrobot::playerMode != -1)
		interval = framerate * speedhack;
	else
		interval = framerate;

	cocos2d::CCDirector::sharedDirector()->setAnimationInterval(1.f / interval);
	onAudioSpeedChange();
}

void Common::setIconValue(int valueOffset, int value)
{
	auto gm = Common::gameManager;
	MBO(int, Common::gameManager, valueOffset) = value;
	MBO(int, Common::gameManager, valueOffset - 8) = value + MBO(int, Common::gameManager, valueOffset - 4);
}

void Common::saveIcons()
{
	Settings::set<int>("icons/cube", MBO(int, Common::gameManager, 556));
	Settings::set<int>("icons/ship", MBO(int, Common::gameManager, 568));
	Settings::set<int>("icons/ball", MBO(int, Common::gameManager, 580));
	Settings::set<int>("icons/ufo", MBO(int, Common::gameManager, 592));
	Settings::set<int>("icons/wave", MBO(int, Common::gameManager, 604));
	Settings::set<int>("icons/robot", MBO(int, Common::gameManager, 616));
	Settings::set<int>("icons/spider", MBO(int, Common::gameManager, 628));
	Settings::set<int>("icons/swing", MBO(int, Common::gameManager, 640));
	Settings::set<int>("icons/color", MBO(int, Common::gameManager, 652));
	Settings::set<int>("icons/color2", MBO(int, Common::gameManager, 664));
	Settings::set<int>("icons/colorglow", MBO(int, Common::gameManager, 676));
	Settings::set<int>("icons/streak", MBO(int, Common::gameManager, 688));
	Settings::set<int>("icons/shipfire", MBO(int, Common::gameManager, 700));
	Settings::set<int>("icons/death", MBO(int, Common::gameManager, 712));
	Settings::set<int>("icons/jetpack", MBO(int, Common::gameManager, 724));
	Settings::save();
}

void Common::loadIcons()
{
	auto setIcon = [](std::string setting, int offset) {
		int icon = Settings::get<int>(setting, -1);
		if (icon >= 0)
			Common::setIconValue(offset, icon);
	};

	setIcon("icons/cube", 556);
	setIcon("icons/ship", 568);
	setIcon("icons/ball", 580);
	setIcon("icons/ufo", 592);
	setIcon("icons/wave", 604);
	setIcon("icons/robot", 616);
	setIcon("icons/spider", 628);
	setIcon("icons/swing", 640);
	setIcon("icons/color", 652);
	setIcon("icons/color2", 664);
	setIcon("icons/colorglow", 676);
	setIcon("icons/streak", 688);
	setIcon("icons/shipfire", 700);
	setIcon("icons/death", 712);
	setIcon("icons/jetpack", 724);
}

void Common::setPriority()
{
	static int prev_priority = -1;
	int priority = Settings::get<int>("general/priority", 2);

	if (prev_priority != priority)
	{
		switch (priority)
		{
		case 0:
			SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
			break;
		case 1:
			SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
			break;
		case 2:
			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			break;
		case 3:
			SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
			break;
		case 4:
			SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
			break;
		}

		prev_priority = priority;
	}
}

void Common::onAudioSpeedChange()
{
	float speed;

	if (Settings::get<bool>("general/music/speed/enabled"))
		speed = Settings::get<float>("general/music/speed/value");
	else if (Settings::get<bool>("general/tie_to_game_speed/music/enabled") &&
			 Settings::get<bool>("general/speedhack/enabled"))
		speed = Settings::get<float>("general/speedhack/value");
	else
		speed = 1.f;

	AudioChannelControl::set(speed);
	onAudioPitchChange();
}

void Common::onAudioPitchChange()
{
	bool enabled = Settings::get<bool>("general/music/pitch/enabled", false);
	float pitch = Settings::get<float>("general/music/pitch/value", 1.f);

	AudioChannelControl::setPitch(enabled ? pitch : 1.f);
}

void Common::openLink(const char* path)
{
#ifdef _WIN32
	::ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
#else
#if __APPLE__
	const char* open_executable = "open";
#else
	const char* open_executable = "xdg-open";
#endif
	char command[256];
	snprintf(command, 256, "%s \"%s\"", open_executable, path);
	system(command);
#endif
}

bool __fastcall Common::menuLayerInitHook(int* self, void*)
{
	saveIcons();
	return menuLayerInit(self);
}

const char* __fastcall Common::splashStringHook()
{
	if (utils::randomInt(0, 100) == 2)
		return "Bella ragazzi qua e' St3pNy e bentornati in questo nuovo video";
	else
		return "GD Mega Overlay - Made by maxnut";
}

void Common::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x272A20), splashStringHook,
				  reinterpret_cast<void**>(&splashString));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x276700), menuLayerInitHook,
				  reinterpret_cast<void**>(&menuLayerInit));
}
