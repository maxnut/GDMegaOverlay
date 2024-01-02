#include "Common.h"
#include "Macrobot/Macrobot.h"
#include "Hacks/AudioChannelControl.h"
#include "Settings.h"
#include "utils.hpp"
#include <MinHook.h>

#include <imgui.h>
#include <fstream>

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

	if (Settings::get<bool>("general/speedhack/enabled") && Macrobot::playerMode != -1)
		interval = framerate * speedhack;
	else
		interval = framerate;

	cocos2d::CCDirector::sharedDirector()->setAnimationInterval(1.f / interval);
	onAudioSpeedChange();
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
	else if (Settings::get<bool>("general/tie_to_game_speed/music/enabled") && Settings::get<bool>("general/speedhack/enabled"))
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
}
