#include "CustomSongWidget.h"
#include "Hacks.h"

bool __fastcall CustomSongWidget::initHook(
	gd::CustomSongWidget* self, void*, gd::SongInfoObject* songInfo,
	gd::LevelSettingsObject* levelSettings, bool p2,
	bool p3, bool p4, bool hasDefaultSong, bool hideBackground
) {
	if (!CustomSongWidget::init(self, songInfo, levelSettings, p2, p3, p4, hasDefaultSong, hideBackground)) return false;

	CCMenu* menu = reinterpret_cast<CCMenu*>(self->getChildren()->objectAtIndex(1));

	if (hacks.playSongButton)
		reinterpret_cast<gd::CCMenuItemSpriteExtra*>(menu->getChildren()->objectAtIndex(4))->setVisible(true);

	auto& moreButtonPosition = reinterpret_cast<gd::CCMenuItemSpriteExtra*>(menu->getChildren()->objectAtIndex(5))->getPosition();

	if (hacks.copySongButton)
	{
		auto buttonSprite = gd::ButtonSprite::create("Copy", 30, true, "bigFont.fnt", "GJ_button_02.png", 20, 1.0f);
		auto copyButton = gd::CCMenuItemSpriteExtra::create(
			buttonSprite,
			self,
			menu_selector(ButtonsClass::onCopySongCallback)
		);
		copyButton->setSizeMult(1.f);
		copyButton->setPosition({ moreButtonPosition.x + 50.f, -157.000 });
		menu->addChild(copyButton);
	}

	return true;
}
