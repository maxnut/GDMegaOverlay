#include "CreatorLayer.h"

bool __fastcall CreatorLayer::initHook(gd::CreatorLayer* self)
{
	if (!CreatorLayer::init(self)) return false;

	auto menu = reinterpret_cast<CCMenu*>(self->getChildren()->objectAtIndex(1));

	auto buttonSprite = gd::ButtonSprite::create("R", 20, true, "bigFont.fnt", "GJ_button_06.png", 0.f, 0.8f);
	auto rouletteButton = gd::CCMenuItemSpriteExtra::create(
		buttonSprite,
		self,
		menu_selector(ButtonsClass::onRouletteButton)
	);
	rouletteButton->setSizeMult(1.f);
	rouletteButton->setPosition({ -235.f, -110.f });
	menu->addChild(rouletteButton);

	return true;
}
