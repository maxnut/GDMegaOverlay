#include "CustomSongWidget.h"
#include "Hacks.h"

bool __fastcall CustomSongWidget::initHook(
	gd::CustomSongWidget* self, void*, gd::SongInfoObject* songInfo,
	gd::LevelSettingsObject* levelSettings, bool p2,
	bool p3, bool p4, bool hasDefaultSong, bool hideBackground
) {
	if (!CustomSongWidget::init(self, songInfo, levelSettings, p2, p3, p4, hasDefaultSong, hideBackground)) return false;

	CCMenu* menu = reinterpret_cast<CCMenu*>(self->getChildren()->objectAtIndex(1));

	if (hacks.copySongButton)
	{
		if (
			reinterpret_cast<cocos2d::CCSprite*>(
				reinterpret_cast<CCLabelBMFont*>(self->getChildren()->objectAtIndex(4))->getChildren()->objectAtIndex(0)
			)->isVisible()
		) {
			auto& moreButtonPosition = reinterpret_cast<gd::CCMenuItemSpriteExtra*>(menu->getChildren()->objectAtIndex(5))->getPosition();

			auto buttonSprite = gd::ButtonSprite::create("Copy", 30, true, "bigFont.fnt", "GJ_button_02.png", 20.f, 1.f);
			auto copyButton = gd::CCMenuItemSpriteExtra::create(
				buttonSprite,
				self,
				menu_selector(ButtonsClass::onCopySongCallback)
			);
			copyButton->setSizeMult(1.f);
			copyButton->setPosition({ moreButtonPosition.x + 50.f, moreButtonPosition.y });
			menu->addChild(copyButton);
		}
	}

	return true;
}

void __fastcall CustomSongWidget::onPlaySongButtonHook(gd::CustomSongWidget* self)
{
	if (hacks.playSongButton)
	{
		CCMenu* menu = reinterpret_cast<CCMenu*>(self->getChildren()->objectAtIndex(1));

		if (!reinterpret_cast<gd::CCMenuItemSpriteExtra*>(menu->getChildren()->objectAtIndex(0))->isVisible())
			reinterpret_cast<gd::CCMenuItemSpriteExtra*>(menu->getChildren()->objectAtIndex(4))->setVisible(true);
	}

	CustomSongWidget::onPlaySongButton(self);
}
