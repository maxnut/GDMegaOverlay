
#include "Common.h"
#include "Hacks/AudioChannelControl.h"
#include "Macrobot/Macrobot.h"
#include "util.hpp"
#include "Settings.hpp"
#include "ConstData.h"
#include "JsonPatches/JsonPatches.h"

#include <fstream>
#include <imgui.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#include <Geode/modify/MenuLayer.hpp>
#include <Geode/cocos/platform/third_party/win32/curl/curl.h>

#include <Geode/binding/GameManager.hpp>

#include <Geode/Geode.hpp>
using namespace geode::prelude;

void Common::calculateFramerate()
{
	float speedhack = Settings::get<float>("general/speedhack/value", 1.f);
	float framerate = 60.f;
	float interval = 60.f;

	if (Settings::get<bool>("general/fps/enabled"))
		framerate = Settings::get<float>("general/fps/value", 60.f);
	else
		framerate = GameManager::get()->m_customFPSTarget;// MBO(float, gameManager, 900);

	if (framerate < 60.f)
		framerate = 60.f;

	/* if (Settings::get<bool>("general/speedhack/enabled") && Macrobot::playerMode != -1)
		interval = framerate * speedhack;
	else
		interval = framerate; */

	cocos2d::CCDirector::sharedDirector()->setAnimationInterval(1.f / framerate);
	onAudioSpeedChange();
}

void Common::saveIcons()
{
	Mod::get()->setSavedValue<int>("icons/cube", GameManager::get()->m_playerFrame);
	Mod::get()->setSavedValue<int>("icons/ship", GameManager::get()->m_playerShip);
	Mod::get()->setSavedValue<int>("icons/ball", GameManager::get()->m_playerBall);
	Mod::get()->setSavedValue<int>("icons/ufo", GameManager::get()->m_playerBird);
	Mod::get()->setSavedValue<int>("icons/wave", GameManager::get()->m_playerDart);
	Mod::get()->setSavedValue<int>("icons/robot", GameManager::get()->m_playerRobot);
	Mod::get()->setSavedValue<int>("icons/spider", GameManager::get()->m_playerSpider);
	Mod::get()->setSavedValue<int>("icons/swing", GameManager::get()->m_playerSwing);
	Mod::get()->setSavedValue<int>("icons/color", GameManager::get()->m_playerColor);
	Mod::get()->setSavedValue<int>("icons/color2", GameManager::get()->m_playerColor2);
	Mod::get()->setSavedValue<int>("icons/colorglow", GameManager::get()->m_playerGlowColor);
	Mod::get()->setSavedValue<int>("icons/glow", GameManager::get()->m_playerGlow);
	Mod::get()->setSavedValue<int>("icons/streak", GameManager::get()->m_playerStreak);
	Mod::get()->setSavedValue<int>("icons/shipfire", GameManager::get()->m_playerShipFire);
	Mod::get()->setSavedValue<int>("icons/death", GameManager::get()->m_playerDeathEffect);
	Mod::get()->setSavedValue<int>("icons/jetpack", GameManager::get()->m_playerJetpack);
}

void Common::loadIcons()
{
	iconsLoaded = true;
	GameManager::get()->m_playerFrame = Settings::get<int>("icons/cube", GameManager::get()->m_playerFrame);
	GameManager::get()->m_playerShip = Settings::get<int>("icons/ship", GameManager::get()->m_playerShip);
	GameManager::get()->m_playerBall = Settings::get<int>("icons/ball", GameManager::get()->m_playerBall);
	GameManager::get()->m_playerBird = Settings::get<int>("icons/ufo", GameManager::get()->m_playerBird);
	GameManager::get()->m_playerDart = Settings::get<int>("icons/wave", GameManager::get()->m_playerDart);
	GameManager::get()->m_playerRobot = Settings::get<int>("icons/robot", GameManager::get()->m_playerRobot);
	GameManager::get()->m_playerSpider = Settings::get<int>("icons/spider", GameManager::get()->m_playerSpider);
	GameManager::get()->m_playerSwing = Settings::get<int>("icons/swing", GameManager::get()->m_playerSwing);
	GameManager::get()->m_playerColor = Settings::get<int>("icons/color", GameManager::get()->m_playerColor);
	GameManager::get()->m_playerColor2 = Settings::get<int>("icons/color2", GameManager::get()->m_playerColor2);
	GameManager::get()->m_playerGlowColor = Settings::get<int>("icons/colorglow", GameManager::get()->m_playerGlowColor);
	GameManager::get()->m_playerGlow = Settings::get<int>("icons/glow", GameManager::get()->m_playerGlow);
	GameManager::get()->m_playerStreak = Settings::get<int>("icons/streak", GameManager::get()->m_playerStreak);
	GameManager::get()->m_playerShipFire = Settings::get<int>("icons/shipfire", GameManager::get()->m_playerShipFire);
	GameManager::get()->m_playerDeathEffect = Settings::get<int>("icons/death", GameManager::get()->m_playerDeathEffect);
	GameManager::get()->m_playerJetpack = Settings::get<int>("icons/jetpack", GameManager::get()->m_playerJetpack);
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

void Common::updateCheathing()
{
	auto checkPatchGroup = [&](JsonPatches::patch_group_type group)
	{
		for(const auto& pair : group)
		{
			if(cheatOpcodes.contains(pair.second.patches[0]->getAddress() - base::get()) && pair.second.patches[0]->isEnabled())
			{
				isCheating = true;
				return true;
			}
		}

		return false;
	};

	if(checkPatchGroup(JsonPatches::bypass))
		return;
	if(checkPatchGroup(JsonPatches::creator))
		return;
	if(checkPatchGroup(JsonPatches::global))
		return;
	if(checkPatchGroup(JsonPatches::level))
		return;
	if(checkPatchGroup(JsonPatches::player))
		return;

	float speedhack =
		Settings::get<bool>("general/speedhack/enabled") ? Settings::get<float>("general/speedhack/value") : 1.f;

	if(speedhack != 1.f)
	{
		isCheating = true;
		return;
	}

	isCheating = false;
}

class $modify(MenuLayer) {
	bool init()
	{
		if(Common::iconsLoaded)
			Common::saveIcons();

		return MenuLayer::init();
	}
};
