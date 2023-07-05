#include "CreatorLayer.h"

bool __fastcall CreatorLayer::initHook(gd::CreatorLayer* self)
{
	if (!CreatorLayer::init(self)) return false;

	auto menu = reinterpret_cast<CCMenu*>(self->getChildren()->objectAtIndex(1));

	auto rouletteButton = gd::CCMenuItemSpriteExtra::create(
		CCSprite::create("RL_blankBtn_001.png"),
		self,
		menu_selector(ButtonsClass::onRouletteButton)
	);
	rouletteButton->setSizeMult(1.f);
	rouletteButton->setPosition({ -235.f, -110.f });
	auto rouletteSprite = CCSprite::create("RL_btn_001.png");
	rouletteSprite->setScale(.035f);
	rouletteSprite->setPosition({ 22.5f, 23.5f });
	rouletteButton->addChild(rouletteSprite);
	menu->addChild(rouletteButton);

	return true;
}
