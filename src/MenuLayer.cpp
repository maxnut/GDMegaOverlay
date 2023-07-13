#include "MenuLayer.h"
#include "Hacks.h"
#include "bools.h"
#include <fstream>

HacksStr hacks;
Labels labels;
Debug debug;

bool firstCall = false;

bool __fastcall MenuLayer::hook(CCLayer* self)
{
	bool result = MenuLayer::init(self);

	auto gm = gd::GameManager::sharedState();

	if (!firstCall)
	{
		firstCall = true;
		Hacks::MenuMusic();

		if (hacks.iconIds[0] >= 0 && hacks.iconIds[0] < 200)
		{
			gm->setPlayerFrame(hacks.iconIds[0]);
			gm->setPlayerShip(hacks.iconIds[1]);
			gm->setPlayerBall(hacks.iconIds[2]);
			gm->setPlayerBird(hacks.iconIds[3]);
			gm->setPlayerDart(hacks.iconIds[4]);
			gm->setPlayerRobot(hacks.iconIds[5]);
			gm->setPlayerSpider(hacks.iconIds[6]);
			gm->setPlayerColor(hacks.iconIds[7]);
			gm->setPlayerColor2(hacks.iconIds[8]);
			gm->setPlayerDeathEffect(hacks.iconIds[9]);
			gm->setPlayerGlow(hacks.iconIds[10]);
			gm->setPlayerStreak(hacks.iconIds[11]);
		}
	}

	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);

	if (tm.tm_mon == 11 && hacks.snow)
	{
		CCParticleSnow* snow;
		snow = CCParticleSnow::createWithTotalParticles(700);
		self->addChild(snow);
	}

	if (ExternData::ds.core && hacks.discordRPC)
	{
		discord::Activity activity{};
		activity.GetTimestamps().SetStart(ExternData::ds.timeStart);
		activity.SetState("Browsing Menus");
		activity.GetAssets().SetLargeImage("cool");
		activity.GetAssets().SetLargeText(gd::GameManager::sharedState()->m_sPlayerName.c_str());
		activity.SetType(discord::ActivityType::Playing);
		ExternData::ds.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}

	return result;
}

void __fastcall MenuLayer::onBackHook(CCLayer* self, void*, cocos2d::CCObject* sender)
{
	auto gm = gd::GameManager::sharedState();
	hacks.iconIds[0] = gm->getPlayerFrame();
	hacks.iconIds[1] = gm->getPlayerShip();
	hacks.iconIds[2] = gm->getPlayerBall();
	hacks.iconIds[3] = gm->getPlayerBird();
	hacks.iconIds[4] = gm->getPlayerDart();
	hacks.iconIds[5] = gm->getPlayerRobot();
	hacks.iconIds[6] = gm->getPlayerSpider();
	hacks.iconIds[7] = gm->getPlayerColor();
	hacks.iconIds[8] = gm->getPlayerColor2();
	hacks.iconIds[9] = gm->getPlayerDeathEffect();
	hacks.iconIds[10] = gm->getPlayerGlow();
	hacks.iconIds[11] = gm->getPlayerStreak();

	std::ofstream f;
	f.open("GDMenu/settings.bin", std::fstream::binary);
	if (f)
		f.write((char*)&hacks, sizeof(HacksStr));
	f.close();

	MenuLayer::onBack(self, sender);
}

const char* __fastcall MenuLayer::loadingStringHook(CCLayer* self, void*)
{
	return "GD Mega Overlay - Made by maxnut";
}