
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
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/cocos/platform/third_party/win32/curl/curl.h>

#include <Geode/binding/GameManager.hpp>

#include <Geode/Geode.hpp>
using namespace geode::prelude;

void Common::calculateFramerate()
{
	bool vsync = GameManager::get()->getGameVariable("0030");
	CCApplication::sharedApplication()->toggleVerticalSync(vsync);

	if(vsync)
		return;

	float framerate = 60.f;
	float interval = 60.f;

	if (Settings::get<bool>("general/fps/enabled"))
		framerate = Settings::get<float>("general/fps/value", 60.f);
	else
		framerate = GameManager::get()->m_customFPSTarget;// MBO(float, gameManager, 900);

	if (framerate < 60.f)
		framerate = 60.f;

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

float Common::getTPS()
{
	float tps = 240.f;

	if (Settings::get<bool>("general/tps/enabled"))
		tps = Settings::get<float>("general/tps/value", 240.f);
	else
		tps = 240.f;
	if(Macrobot::playerMode == Macrobot::PLAYBACK)
		tps = Macrobot::macro.framerate;
	if(tps < 1.f)
		tps = 1.f;

	return tps;
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

void Common::updateCheating()
{
	auto checkPatchGroup = [&](JsonPatches::patch_group_type group)
	{
		for(const auto& pair : group)
		{
			if (cheatOpcodes.contains(pair.second.patches[0]->getAddress() - base::get()) && pair.second.patches[0]->isEnabled())
			{
				isCheating = true;
				return true;
			}
		}
		return false;
	};
	if (checkPatchGroup(JsonPatches::bypass))
		return;
	if (checkPatchGroup(JsonPatches::creator))
		return;
	if (checkPatchGroup(JsonPatches::global))
		return;
	if (checkPatchGroup(JsonPatches::level))
		return;
	if (checkPatchGroup(JsonPatches::player))
		return;
	float speedhack =
		Settings::get<bool>("general/speedhack/enabled") ? Settings::get<float>("general/speedhack/value") : 1.f;

	bool showHitbox = Settings::get<bool>("level/show_hitbox/enabled", false);
	bool onDeath = Settings::get<bool>("level/show_hitbox/on_death", false);

	bool hidePause = Settings::get<bool>("general/hide_pause/menu");
	bool noShaders = Settings::get<bool>("level/no_shaders", false);
	bool instantComplete = Settings::get<bool>("level/instant_complete", false);
	bool hitboxMultiplier = Settings::get<bool>("level/hitbox_multiplier", false);

	if (speedhack != 1.f || Macrobot::playerMode == 0 || (showHitbox && !onDeath) || hidePause || hitboxMultiplier || noShaders || instantComplete)
	{
		isCheating = true;
		return;
	}

	float tps = getTPS();

	PlayLayer* pl = GameManager::get()->getPlayLayer();

	if(pl)
	{
		int levelID = pl->m_level->m_levelID;
		if(levelID <= 97454394 && levelID != 0 && tps > 360)
		{
			isCheating = true;
			return;
		}
		else if((levelID > 97454394 || levelID == 0) && tps > 240)
		{
			isCheating = true;
			return;
		}
	}

	isCheating = false;
}

class $modify(PlayLayer)
{
	void updateVisibility(float dt)
	{
		PlayLayer::updateVisibility(dt);

		bool doLoop = false;

		for(auto &pair : Common::sectionLoopFunctions)
		{
			bool setting = Settings::get<bool>(pair.second, false);
			doLoop |= setting;
		}

		if(!doLoop)
			return;

		int cameraSection = MBO(int, this, 10676);
		int cameraSectionLast = MBO(int, this, 10680);

		gd::vector<gd::vector<gd::vector<GameObject*>*>*> sections = MBO(gd::vector<gd::vector<gd::vector<GameObject*>*>*>, this, 11336);

		for(int i = cameraSection; i < cameraSectionLast; i++)
		{
			if(!sections.at(i))
				continue;

			for(int j = 0; j < sections.at(i)->size(); j++)
			{
				if(!sections.at(i)->at(j))
					continue;

				for(int k = 0; k < sections.at(i)->at(j)->size(); k++)
				{
					auto obj = sections.at(i)->at(j)->at(k);
					for(auto &pair : Common::sectionLoopFunctions)
					{
						if(Settings::get<bool>(pair.second, false))
							pair.first(obj);
					}
				}
			}
		}
	}
};

class $modify(MenuLayer)
{
	bool init()
	{
		if (Common::iconsLoaded)
			Common::saveIcons();

		return MenuLayer::init();
	}
};

void Common::uncompleteLevel()
{
	if(!GameManager::get()->getPlayLayer())
	{
		FLAlertLayer::create("Error", "Enter a level first!", "Ok")->show();
		return;
	}
	GJGameLevel* level = GameManager::get()->getPlayLayer()->m_level;

	//uncompleteLevel()
	reinterpret_cast<void(__thiscall *)(GameStatsManager*, GJGameLevel*)>(base::get() + 0x170400)(GameStatsManager::sharedState(), level);
	level->m_practicePercent = 0;
	level->m_normalPercent = 0;
	level->m_newNormalPercent2 = 0;
	level->m_orbCompletion = 0;
	level->m_54 = 0;
	level->m_k111 = 0;
	level->m_bestPoints = 0;
	level->m_bestTime = 0;

	GameLevelManager::sharedState()->saveLevel(level);
}