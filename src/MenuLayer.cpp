#include "MenuLayer.h"
#include "Hacks.h"
#include "bools.h"
#include <fstream>

HacksStr hacks;
Labels labels;

bool firstCall = false;

bool __fastcall MenuLayer::hook(CCLayer* self)
{
	auto gm = gd::GameManager::sharedState();
	bool result = MenuLayer::init(self);

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();
	

 	CCLabelBMFont* pipi = CCLabelBMFont::create("Running - made by maxnut", "bigFont-uhd.fnt");
	pipi->setZOrder(1000);
	pipi->setScale(0.2f);
	pipi->setOpacity(150.0f);
	pipi->setPosition(size.width / 2, 10);
	self->addChild(pipi);

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
	}
	else if(hacks.icons)
	{
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
        f.open("settings.bin", std::fstream::binary);
        if(f) f.write((char*) &hacks, sizeof(HacksStr));
        f.close();
	}

	return result;
}