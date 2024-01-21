
#include "Common.h"
#include "Hacks/AudioChannelControl.h"
#include "Macrobot/Macrobot.h"
#include "Settings.h"
#include "util.hpp"

#include <fstream>
#include <imgui.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/cocos/platform/third_party/win32/curl/curl.h>

#include <Geode/binding/GameManager.hpp>

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

	if (Settings::get<bool>("general/speedhack/enabled") && Macrobot::playerMode != -1)
		interval = framerate * speedhack;
	else
		interval = framerate;

	cocos2d::CCDirector::sharedDirector()->setAnimationInterval(1.f / interval);
	onAudioSpeedChange();
}

void Common::saveIcons()
{
	Settings::set<int>("icons/cube", GameManager::get()->m_playerFrame);
	Settings::set<int>("icons/ship", GameManager::get()->m_playerShip);
	Settings::set<int>("icons/ball", GameManager::get()->m_playerBall);
	Settings::set<int>("icons/ufo", GameManager::get()->m_playerBird);
	Settings::set<int>("icons/wave", GameManager::get()->m_playerDart);
	Settings::set<int>("icons/robot", GameManager::get()->m_playerRobot);
	Settings::set<int>("icons/spider", GameManager::get()->m_playerSpider);
	Settings::set<int>("icons/swing", GameManager::get()->m_playerSwing);
	Settings::set<int>("icons/color", GameManager::get()->m_playerColor);
	Settings::set<int>("icons/color2", GameManager::get()->m_playerColor2);
	Settings::set<int>("icons/colorglow", GameManager::get()->m_playerGlowColor);
	Settings::set<int>("icons/streak", GameManager::get()->m_playerStreak);
	Settings::set<int>("icons/shipfire", GameManager::get()->m_playerShipFire);
	Settings::set<int>("icons/death", GameManager::get()->m_playerDeathEffect);
	Settings::set<int>("icons/jetpack", GameManager::get()->m_playerJetpack);
	Settings::save();
}

void Common::loadIcons()
{
	GameManager::get()->m_playerFrame = Settings::get<int>("icons/cube", -1);
	GameManager::get()->m_playerShip = Settings::get<int>("icons/ship", -1);
	GameManager::get()->m_playerBall = Settings::get<int>("icons/ball", -1);
	GameManager::get()->m_playerBird = Settings::get<int>("icons/ufo", -1);
	GameManager::get()->m_playerDart = Settings::get<int>("icons/wave", -1);
	GameManager::get()->m_playerRobot = Settings::get<int>("icons/robot", -1);
	GameManager::get()->m_playerSpider = Settings::get<int>("icons/spider", -1);
	GameManager::get()->m_playerSwing = Settings::get<int>("icons/swing", -1);
	GameManager::get()->m_playerColor = Settings::get<int>("icons/color", -1);
	GameManager::get()->m_playerColor2 = Settings::get<int>("icons/color2", -1);
	GameManager::get()->m_playerGlowColor = Settings::get<int>("icons/colorglow", -1);
	GameManager::get()->m_playerStreak = Settings::get<int>("icons/streak", -1);
	GameManager::get()->m_playerShipFire = Settings::get<int>("icons/shipfire", -1);
	GameManager::get()->m_playerDeathEffect = Settings::get<int>("icons/death", -1);
	GameManager::get()->m_playerJetpack = Settings::get<int>("icons/jetpack", -1);
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

size_t CurlWrite_CallbackFunc_StdString(void* contents, size_t size, size_t nmemb, std::string* s)
{
	size_t newLength = size * nmemb;
	try
	{
		s->append((char*)contents, newLength);
	}
	catch (std::bad_alloc& e)
	{
		// handle memory problem
		return 0;
	}
	return newLength;
}

int Common::getRequest(std::string url, std::string* buffer, std::string userAgent)
{
	curl_global_init(CURL_GLOBAL_ALL);
	CURL* curl = curl_easy_init();
	CURLcode res = CURLE_FAILED_INIT;

	if (!curl)
		return res;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite_CallbackFunc_StdString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	return res;
}

class $modify(MenuLayer) {
	bool init()
	{
		Common::saveIcons();

		return MenuLayer::init();
	}
};
