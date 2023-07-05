#pragma once
#include "pch.h"

class RouletteLayer : public gd::FLAlertLayer
{
private:
	gd::LoadingCircle* circle{};
	gd::TextArea* changelog{};
	gd::TextArea* description{};
	CCLabelBMFont* changelogFailText{};
	CCLabelBMFont* title{};

	CCLabelBMFont* startButtonText;
	CCSprite* startButtonSprite;
	gd::CCMenuItemSpriteExtra* startButton;
	gd::CCMenuItemSpriteExtra* plusButton;

	std::array<bool, 6> difficultyArr{
		false, false, false,
		false, false, false
	};
	std::array<bool, 5> demonDiffArr{
		false, false, false,
		false, false
	};

public:
	static RouletteLayer* create();
	bool init();
	static CCLabelBMFont* createTextLabel(const std::string text, const CCPoint& position, const float scale, CCNode* menu, const char* font = "bigFont.fnt");
	void createDifficultyButton(CCNode* sprite, CCObject* obj, CCMenu* menu, CCPoint point, float scale);

	void onClose(CCObject* sender);

private:
	CCPoint _alertSize{};
	gd::CCMenuItemSpriteExtra* _closeBtn{};

	bool _createBasics(CCPoint contentSize, SEL_MenuHandler onClose, float closeBtnScale = 1, const cocos2d::ccColor4B& color = { 0x00, 0x00, 0x00, 0x4B });
	void _createTitle(std::string text, float separatorScale = .75f, float usernameScale = 1);
	gd::CCMenuItemSpriteExtra* _createButton(const char* texture, CCPoint position, SEL_MenuHandler callback, float textureScale = 1.0f, float sizeMult = 1.2f);
	void _keyBackClicked();
};
