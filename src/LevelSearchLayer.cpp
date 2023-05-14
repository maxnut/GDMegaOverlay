#include "LevelSearchLayer.h"
#include "FetchDemonlist.h"
#include "Hacks.h"

bool __fastcall LevelSearchLayer::hook(gd::LevelSearchLayer* self)
{
	bool res = LevelSearchLayer::init(self);

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	static_cast<CCNode*>(self->getChildren()->objectAtIndex(11))->setVisible(false);
	static_cast<CCNode*>(self->getChildren()->objectAtIndex(8))->setVisible(false);
	static_cast<CCNode*>(self->getChildren()->objectAtIndex(9))->setScaleY(1.3f);

	auto buttonContainer = static_cast<CCMenu*>(self->getChildren()->objectAtIndex(10));

	buttonContainer->setPositionY(142);

	{
		CCSprite* trophy = CCSprite::createWithSpriteFrameName("rankIcon_1_001.png");
		trophy->setAnchorPoint({0, 0.5f});
		trophy->setPosition({137, 16});
		trophy->setScale(0.75);
		CCLabelBMFont* writing = CCLabelBMFont::create("Demon List", "bigFont.fnt");
		writing->setPosition({74.5f, 16.5});
		writing->setScale(0.6);
		writing->setAnchorPoint({0.5, 0.5});
		CCSprite* button = CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png");
		auto nextButton =
			gd::CCMenuItemSpriteExtra::create(button, self, menu_selector(FetchDemonlist::demonlistCallback));
		nextButton->setPosition({-90.5, 100});

		button->addChild(trophy);
		button->addChild(writing);

		buttonContainer->addChild(nextButton);
	}

	{
		CCSprite* trophy = CCSprite::createWithSpriteFrameName("rankIcon_1_001.png");
		trophy->setAnchorPoint({0, 0.5f});
		trophy->setPosition({137, 16});
		trophy->setScale(0.75);
		CCLabelBMFont* writing = CCLabelBMFont::create("Challenge List", "bigFont.fnt");
		writing->setPosition({74.5f, 16.5});
		writing->setScale(0.45f);
		writing->setAnchorPoint({0.5, 0.5});
		CCSprite* button = CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png");
		auto nextButton =
			gd::CCMenuItemSpriteExtra::create(button, self, menu_selector(FetchDemonlist::challengeListCallback));
		nextButton->setPosition({90.5, 100});

		button->addChild(trophy);
		button->addChild(writing);

		buttonContainer->addChild(nextButton);
	}

	CCObject* obj;
	size_t i = 0;
	CCARRAY_FOREACH(self->getChildren(), obj)
	{
		obj->setTag(i);
		i++;
	}

	return res;
}

void __fastcall LevelSearchLayer::httpHook(gd::GameLevelManager* self, void*, gd::string gdurl, gd::string gdquery,
										   gd::string idk, int type)
{
	std::string url = gdurl.c_str();
	std::string query = gdquery.c_str();
	std::string idk2 = idk.c_str();
	if (url == "http://www.boomlings.com/database/getGJLevels21.php")
	{
		auto thing = atoi(query.substr(query.find("page=") + 5).c_str());
		if (query.find("type=3141") != std::string::npos)
		{
			gdurl = std::string("http://absolllute.com/api/mega_hack/demonlist/page") + std::to_string(thing) + ".txt";
		}
		else if (query.find("type=3142") != std::string::npos)
		{
			gdurl =
				std::string("http://absolllute.com/api/mega_hack/challengelist/page") + std::to_string(thing) + ".txt";
		}
	}
	sizeof(gd::CCCircleWaveDelegate);
	sizeof(std::map<std::string,bool>);
}