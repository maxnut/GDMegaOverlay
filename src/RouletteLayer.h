#pragma once
#include "pch.h"


class CustomLayer : public gd::FLAlertLayer
{
public:
	CCPoint alertSize{};
	gd::CCMenuItemSpriteExtra* closeBtn{};

	bool createBasics(CCPoint contentSize, SEL_MenuHandler onClose, float closeBtnScale = 1, const ccColor4B& color = { 0x00, 0x00, 0x00, 0x4B });
	void createTitle(std::string text, float separatorScale = .75f, float usernameScale = 1);
	gd::CCMenuItemSpriteExtra* createButton(const char* texture, CCPoint position, SEL_MenuHandler callback, int tag = -1, float textureScale = 1.0f, float sizeMult = 1.2f);
	virtual void onClose(CCObject* sender) = 0;
	void keyBackClicked();
};


class RouletteLayer : public CustomLayer
{
private:
	static gd::LoadingCircle* levelLoadingCircle;
	inline static bool isPlusButtonToggled = false;

public:
	static RouletteLayer* create();
	bool init();

	void update(float dt);

	void onClose(CCObject* sender);
	void onInfoButton(CCObject* sender);
	void onDifficultyChosen(CCObject* sender);
	void onStartButton(CCObject* sender);
	void onPlusButton(CCObject* sender);
	void onLevelInfo(CCObject* sender);
	void onPlayButton(CCObject* sender);
	void onNextButton(CCObject* sender);
	void onSkipButton(CCObject* sender);
	void onResetButton(CCObject* sender);

	void finishLevelRoulette();

private:
	// helper functions
	gd::CCMenuItemSpriteExtra* createDifficultyButton(int tag, CCNode* sprite, CCPoint point, float scale, bool isDemon = false, bool visible = true);
};

class RouletteInfoLayer : public CustomLayer
{
public:
	static RouletteInfoLayer* create();
	bool init();

	void onClose(CCObject* sender);
	void onToggleButton(CCObject* sender);

private:
	gd::CCMenuItemToggler* createToggler(int tag, const char* label, CCPoint point, bool visible = true);

	void destroyLayerChildren();
};

class RouletteLevelLayer : public CustomLayer
{
public:
	static RouletteLevelLayer* create();
	bool init();

	void onClose(CCObject* sender);
};