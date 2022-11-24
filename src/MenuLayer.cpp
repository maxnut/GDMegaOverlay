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
	auto gm = gd::GameManager::sharedState();
	bool result = MenuLayer::init(self);

	if(hacks.icons && !firstCall)
	{
		firstCall = true;
		
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

		srand(time(NULL));
		Hacks::MenuMusic();
	}

	return result;
}

void __fastcall MenuLayer::onBackHook(CCLayer* self, void*, cocos2d::CCObject* sender)
{
	if(hacks.icons)
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
        if(f) f.write((char*) &hacks, sizeof(HacksStr));
        f.close();
	}
	MenuLayer::onBack(self, sender);
}

const char* __fastcall MenuLayer::loadingStringHook(CCLayer* self, void*)
{
	return "GD Mod Menu - Made by maxnut";
}