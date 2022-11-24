#include "LevelSearchLayer.h"
#include "FetchDemonlist.h"
#include "Hacks.h"

bool __fastcall LevelSearchLayer::hook(CCLayer* self)
{
    bool res = LevelSearchLayer::init(self);

    auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	CCSprite* button = CCSprite::createWithSpriteFrameName("rankIcon_1_001.png");
	button->setFlipX(true);
	auto nextButton = gd::CCMenuItemSpriteExtra::create(button, self, menu_selector(FetchDemonlist::callback));

	CCMenu* menu = CCMenu::create();
	menu->setPosition(size.width - 40, size.height / 2);
	menu->addChild(nextButton);
	self->addChild(menu);

    return res;
}