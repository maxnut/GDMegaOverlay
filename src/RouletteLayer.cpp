#include <nlohmann/json.hpp>
#include <thread>
#include <imgui.h>
#include "RouletteLayer.h"
#include "ListFetcher.h"
#include "Hacks.h"
#define INITIALIZEROULETTEMANAGER
#include "RouletteManager.h"


std::map<std::string, int> difficultyToTag{
	{ "Easy", 103 },
	{ "Normal", 104 },
	{ "Hard", 105 },
	{ "Harder", 106 },
	{ "Insane", 107 },
	{ "Easy Demon", 108 },
	{ "Medium Demon", 109 },
	{ "Hard Demon", 110 },
	{ "Insane Demon", 111 },
	{ "Extreme Demon", 112 }
};

CCLabelBMFont* createTextLabel(const std::string text, const CCPoint& position, const float scale, CCNode* menu, const char* font = "bigFont.fnt")
{
	CCLabelBMFont* bmFont = CCLabelBMFont::create(text.c_str(), font);
	bmFont->setPosition(position);
	bmFont->setScale(scale);
	menu->addChild(bmFont);

	return bmFont;
}

template<typename T, std::size_t S>
std::ptrdiff_t getIndexOf(const std::array<T, S>& arr, T to_find)
{
	auto it = std::find(arr.cbegin(), arr.cend(), to_find);

	if (it != arr.cend())
		return it - arr.cbegin();
	else
		return -1;
}

template<typename T, std::size_t S>
int getCountOf(const std::array<T, S>& arr, T to_find)
{
	return std::count(arr.cbegin(), arr.cend(), to_find);
}

void getlistLevel(RouletteLayer* self, int difficulty, nlohmann::json& list)
{
	int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
	std::thread getListThread;

	switch (listType)
	{
	case 0:
		getListThread = std::thread(ListFetcher::getNormalList, difficulty, std::ref(list));
		break;
	case 1:
		getListThread = std::thread(ListFetcher::getRandomDemonListLevel, std::ref(list));
		break;
	case 2:
		getListThread = std::thread(ListFetcher::getRandomChallengeListLevel, std::ref(list));
		break;
	default:
		break;
	}
	self->scheduleUpdate();
	getListThread.detach();
}


bool CustomLayer::createBasics(CCPoint contentSize, SEL_MenuHandler onClose, float closeBtnScale, const ccColor4B& color)
{
	bool init = CCLayerColor::initWithColor(color);
	if (!init) return false;

	alertSize = contentSize;

	CCDirector* director = CCDirector::sharedDirector();
	director->getTouchDispatcher()->incrementForcePrio(2);

	setTouchEnabled(true);
	setKeypadEnabled(true);

	m_pLayer = CCLayer::create();

	this->addChild(m_pLayer);

	CCSize winSize = director->getWinSize();
	extension::CCScale9Sprite* bg = extension::CCScale9Sprite::create("GJ_square01.png", { .0f, .0f, 80.0f, 80.0f });
	bg->setContentSize(alertSize);
	m_pLayer->addChild(bg, -1);
	bg->setPosition({ winSize.width / 2, winSize.height / 2 });

	m_pButtonMenu = CCMenu::create();
	m_pLayer->addChild(m_pButtonMenu, 10);

	closeBtn = createButton("GJ_closeBtn_001.png", { -((alertSize.x) / 2) + 9.5f, (alertSize.y / 2) - 10 }, onClose, -1, closeBtnScale);

	return true;
}

void CustomLayer::createTitle(std::string text, float separatorScale, float usernameScale)
{
	auto userName = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
	userName->setPosition({ 0, (alertSize.y / 2) - 22 });
	userName->setScale(usernameScale);
	m_pButtonMenu->addChild(userName);

	auto separator = CCSprite::createWithSpriteFrameName("floorLine_001.png");
	separator->setPosition({ 0, (alertSize.y / 2) - 46 });
	separator->setScaleX(separatorScale);
	separator->setOpacity(100);
	m_pButtonMenu->addChild(separator);
}

gd::CCMenuItemSpriteExtra* CustomLayer::createButton(const char* texture, CCPoint position, SEL_MenuHandler callback, int tag, float textureScale, float sizeMult)
{
	auto buttonSprite = CCSprite::createWithSpriteFrameName(texture);
	buttonSprite->setScale(textureScale);
	auto button = gd::CCMenuItemSpriteExtra::create(
		buttonSprite,
		this,
		callback
	);
	button->setPosition(position);
	button->setSizeMult(sizeMult);
	if (tag != -1)
		button->setTag(tag);
	m_pButtonMenu->addChild(button);

	return button;
}

void CustomLayer::keyBackClicked()
{
	onClose(nullptr);
}


RouletteInfoLayer* RouletteInfoLayer::create()
{
	auto ret = new RouletteInfoLayer();

	if (ret && ret->init())
		ret->autorelease();
	else
	{
		delete ret;
		ret = nullptr;
	}

	return ret;
}

bool RouletteInfoLayer::init()
{
	if (!createBasics({ 365.f, 240.f }, menu_selector(RouletteInfoLayer::onClose))) return false;

	auto infoBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { .0f, .0f, 80.0f, 80.0f });
	infoBg->setContentSize({ 320.f, 190.f });
	infoBg->setAnchorPoint({ .5f, 1.f });
	infoBg->setColor({ 123, 60, 31 });
	infoBg->setPosition({.0f, 85.f });
	m_pButtonMenu->addChild(infoBg, -1);


	auto infoTitle = CCLabelBMFont::create("GD Level Roulette Info", "goldFont.fnt");
	infoTitle->setPosition({.0f, 102.f });
	infoTitle->setScale(.725f);
	m_pButtonMenu->addChild(infoTitle);
	

	auto infoText1 = CCLabelBMFont::create("Welcome to GD Level Roulette!", "chatFont.fnt");
	auto infoText2 = CCLabelBMFont::create("Here you can modify some settings to your liking.", "chatFont.fnt");
	infoText1->setPosition({.0f, 72.f });
	infoText2->setPosition({.0f, 56.f });
	infoText1->setScale(.9f);
	infoText2->setScale(.9f);
	m_pButtonMenu->addChild(infoText1);
	m_pButtonMenu->addChild(infoText2);


	createToggler(0, "Normal List", { -120.f, 20.f });
	createToggler(1, "Demon List", { 20.f, 20.f });
	createToggler(2, "Challenge List", { -120.f, -20.f });

	return true;
}

void RouletteInfoLayer::destroyLayerChildren()
{
	for (unsigned int i = 0; i < this->getChildrenCount(); i++)
	{
		auto node = reinterpret_cast<CCNode*>(this->getChildren()->objectAtIndex(0));
		node->removeFromParentAndCleanup(true);
	}

	init();
}

void RouletteInfoLayer::onClose(CCObject* sender)
{
	setKeypadEnabled(false);
	removeFromParentAndCleanup(true);
}

void RouletteInfoLayer::onToggleButton(CCObject* sender)
{
	sender->retain();

	auto button = reinterpret_cast<gd::CCMenuItemToggler*>(sender);
	auto parent = reinterpret_cast<CCMenu*>(button->getParent());
	auto ind = getIndexOf(RouletteManager.togglesStatesArr, true);

	RouletteManager.togglesStatesArr.at(ind) = false;
	RouletteManager.togglesStatesArr.at(button->getTag()) = true;

	destroyLayerChildren();
	sender->release();
}


gd::LoadingCircle* RouletteLayer::levelLoadingCircle;
nlohmann::json list;
// for demon & challenge lists
nlohmann::json listLevel;
int lastListIndex;

RouletteLayer* RouletteLayer::create()
{
	auto ret = new RouletteLayer();

	if (ret && ret->init())
		ret->autorelease();
	else
	{
		delete ret;
		ret = nullptr;
	}

	return ret;
}

bool RouletteLayer::init()
{
	if (!createBasics({ 440.f, 290.f }, menu_selector(RouletteLayer::onClose))) return false;


	auto title = CCLabelBMFont::create("GD Level Roulette", "goldFont.fnt");
	title->setPosition({.0f, 120.f });
	title->limitLabelWidth(340, 1, 0);
	m_pButtonMenu->addChild(title);


	auto rouletteBg = cocos2d::extension::CCScale9Sprite::create("square02b_001.png", { .0f, .0f, 80.0f, 80.0f });
	rouletteBg->setContentSize({ 340.f, 210.f });
	rouletteBg->setAnchorPoint({ .5f, 1.f });
	rouletteBg->setColor({ 123, 60, 31 });
	rouletteBg->setPosition({.0f, 95.f });
	m_pButtonMenu->addChild(rouletteBg, -1);


	createButton("GJ_infoIcon_001.png", { 200.f, 125.f }, menu_selector(RouletteLayer::onInfoButton));

	
	createDifficultyButton(
		0, CCSprite::createWithSpriteFrameName("difficulty_01_btn_001.png"),
		{ -90.f, 40.f }, 1.2f
	);
	createDifficultyButton(
		1, CCSprite::createWithSpriteFrameName("difficulty_02_btn_001.png"),
		{.0f, 40.f }, 1.2f
	);
	createDifficultyButton(
		2, CCSprite::createWithSpriteFrameName("difficulty_03_btn_001.png"),
		{ 90.f, 40.f }, 1.2f
	);
	createDifficultyButton(
		3, CCSprite::createWithSpriteFrameName("difficulty_04_btn_001.png"),
		{ -90.f, -25.f }, 1.2f
	);
	createDifficultyButton(
		4, CCSprite::createWithSpriteFrameName("difficulty_05_btn_001.png"),
		{.0f, -25.f }, 1.2f
	);
	createDifficultyButton(
		5, CCSprite::createWithSpriteFrameName("difficulty_06_btn_001.png"),
		{ 90.f, -25.f }, 1.2f
	);
	createDifficultyButton(
		6, CCSprite::createWithSpriteFrameName("difficulty_07_btn2_001.png"),
		{ -120.f, -40.f }, 1.0f, true, false
	);
	createDifficultyButton(
		7, CCSprite::createWithSpriteFrameName("difficulty_08_btn2_001.png"),
		{ -60.f, -40.f }, 1.0f, true, false
	);
	createDifficultyButton(
		8, CCSprite::createWithSpriteFrameName("difficulty_06_btn2_001.png"),
		{.0f, -40.f }, 1.f, true, false
	);
	createDifficultyButton(
		9, CCSprite::createWithSpriteFrameName("difficulty_09_btn2_001.png"),
		{ 60.f, -40.f }, 1.0f, true, false
	);
	createDifficultyButton(
		10, CCSprite::createWithSpriteFrameName("difficulty_10_btn2_001.png"),
		{ 120.f, -40.f }, 1.0f, true, false
	);
	auto plusButton = createButton(
		"GJ_plus2Btn_001.png", { 135.f, -20.f },
		menu_selector(RouletteLayer::onPlusButton), 11
	);
	plusButton->setVisible(RouletteManager.difficultyArr[5]);

	auto startButtonText = CCLabelBMFont::create("Start", "bigFont.fnt");
	startButtonText->setPosition({ 46.f, 16.f });
	startButtonText->setScale(.725f);
	auto startButton = gd::CCMenuItemSpriteExtra::create(
		CCSprite::createWithSpriteFrameName("GJ_longBtn02_001.png"),
		this,
		menu_selector(RouletteLayer::onStartButton)
	);
	startButton->setPosition({ .0f, -85.f });
	startButton->setTag(12);
	startButton->addChild(startButtonText);
	m_pButtonMenu->addChild(startButton);


	levelLoadingCircle = gd::LoadingCircle::create();
	levelLoadingCircle->setPosition({ -285.f, -170.f });
	levelLoadingCircle->setVisible(false);
	m_pButtonMenu->addChild(levelLoadingCircle);

	auto levelNameItem = gd::CCMenuItemSpriteExtra::create(
		CCLabelBMFont::create("LevelName", "bigFont.fnt"),
		this,
		menu_selector(RouletteLayer::onLevelInfo)
	);
	levelNameItem->setPosition({ 45.f, 75.f });
	levelNameItem->setTag(100);
	levelNameItem->setVisible(false);
	m_pButtonMenu->addChild(levelNameItem);

	auto levelAuthorItem = gd::CCMenuItemSpriteExtra::create(
		CCLabelBMFont::create("LevelAuthor", "bigFont.fnt"),
		this,
		menu_selector(RouletteLayer::onLevelInfo)
	);
	levelAuthorItem->setPosition({ 45.f, 45.f });
	levelAuthorItem->setTag(101);
	levelAuthorItem->setScale(.8f);
	levelAuthorItem->setVisible(false);
	m_pButtonMenu->addChild(levelAuthorItem);

	auto levelIDItem = gd::CCMenuItemSpriteExtra::create(
		CCLabelBMFont::create("LevelID", "bigFont.fnt"),
		this,
		menu_selector(RouletteLayer::onLevelInfo)
	);
	levelIDItem->setPosition({ 45.f, 17.f });
	levelIDItem->setScale(.5f);
	levelIDItem->setTag(102);
	levelIDItem->setVisible(false);
	m_pButtonMenu->addChild(levelIDItem);

	for (int i = 1; i < 6; i++)
	{
		auto levelDifficultySprite = CCSprite::createWithSpriteFrameName(CCString::createWithFormat("difficulty_%02u_btn_001.png", i)->getCString());
		levelDifficultySprite->setPosition({ -110.f, static_cast<float>(i + 80) });
		levelDifficultySprite->setScale(1.5f);
		levelDifficultySprite->setTag(102 + i);
		levelDifficultySprite->setVisible(false);
		m_pButtonMenu->addChild(levelDifficultySprite);
	}

	for (int i = 6; i < 11; i++)
	{
		auto levelDifficultySprite = CCSprite::createWithSpriteFrameName(CCString::createWithFormat("difficulty_%02u_btn2_001.png", i)->getCString());
		levelDifficultySprite->setPosition({ -110.f, static_cast<float>(i + 80) });
		levelDifficultySprite->setScale(1.5f);
		levelDifficultySprite->setVisible(false);
		if (i == 6)
			levelDifficultySprite->setTag(110);
		else if (i == 9)
			levelDifficultySprite->setTag(111);
		else if (i == 10)
			levelDifficultySprite->setTag(112);
		else
			levelDifficultySprite->setTag(101 + i);
		m_pButtonMenu->addChild(levelDifficultySprite);
	}

	auto levelPlayButtonItem = gd::CCMenuItemSpriteExtra::create(
		CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png"),
		this,
		menu_selector(RouletteLayer::onPlayButton)
	);
	levelPlayButtonItem->setPosition({ -110.f, -53.f });
	levelPlayButtonItem->setVisible(false);
	levelPlayButtonItem->setTag(113);
	m_pButtonMenu->addChild(levelPlayButtonItem);

	for (int i = 1; i < 4; i++)
	{
		auto levelCoinSprite = CCSprite::createWithSpriteFrameName("usercoin_small01_001.png");
		levelCoinSprite->setPosition({ -120.f + 10.f * (i - 1), 13.f });
		levelCoinSprite->setScale(1.5f);
		levelCoinSprite->setVisible(false);
		levelCoinSprite->setTag(113 + i);
		m_pButtonMenu->addChild(levelCoinSprite);
	}


	auto percentageText = CCLabelBMFont::create(
		CCString::createWithFormat("%d%%", RouletteManager.lastLevelPercentage)->getCString(), "goldFont.fnt"
	);
	percentageText->setPosition({ 50.f, -20.f });
	percentageText->setScale(.8f);
	percentageText->setVisible(false);
	percentageText->setTag(117);
	m_pButtonMenu->addChild(percentageText);

	auto percentageBg = cocos2d::extension::CCScale9Sprite::create("square02b_small.png");
	percentageBg->setPosition({ 20.f, -35.f });
	percentageBg->setVisible(false);
	percentageBg->setContentSize({ 60.f, 30.f });
	percentageBg->setAnchorPoint({ .0f, .0f });
	percentageBg->setColor({ 0, 0, 0 });
	percentageBg->setOpacity(100);
	percentageBg->setTag(118);
	m_pButtonMenu->addChild(percentageBg, -1);

	auto skipButton = gd::CCMenuItemSpriteExtra::create(
		gd::ButtonSprite::create("Skip", 50, true, "bigFont.fnt", "GJ_button_06.png", .0f, 1.f),
		this,
		menu_selector(RouletteLayer::onSkipButton)
	);
	skipButton->setPosition({ -20.f, -70.f });
	skipButton->setVisible(false);
	skipButton->setTag(119);
	m_pButtonMenu->addChild(skipButton);

	auto nextButton = gd::CCMenuItemSpriteExtra::create(
		gd::ButtonSprite::create("Next", 50, true, "bigFont.fnt", "GJ_button_01.png", .0f, .8f),
		this,
		menu_selector(RouletteLayer::onNextButton)
	);
	nextButton->setPosition({ 50.f, -70.f });
	nextButton->setVisible(false);
	nextButton->setTag(120);
	m_pButtonMenu->addChild(nextButton);

	auto resetButton = gd::CCMenuItemSpriteExtra::create(
		gd::ButtonSprite::create("Reset", 50, true, "bigFont.fnt", "GJ_button_06.png", .0f, .8f),
		this,
		menu_selector(RouletteLayer::onResetButton)
	);
	resetButton->setPosition({ 120.f, -70.f });
	resetButton->setVisible(false);
	resetButton->setTag(121);
	m_pButtonMenu->addChild(resetButton);


	auto ggText = CCLabelBMFont::create("GG!", "goldFont.fnt");
	ggText->setPosition({ .0f, 60.f });
	ggText->setVisible(false);
	ggText->setTag(122);
	m_pButtonMenu->addChild(ggText);

	auto ggSkipsUsedText = CCLabelBMFont::create("Skips Used: ", "bigFont.fnt");
	ggSkipsUsedText->setPosition({ .0f, 30.f });
	ggSkipsUsedText->setVisible(false);
	ggSkipsUsedText->setTag(123);
	m_pButtonMenu->addChild(ggSkipsUsedText);


	auto errorText = CCLabelBMFont::create("An error has occured", "bigFont.fnt");
	errorText->setPosition({ 10.f, 10.f });
	errorText->setVisible(false);
	errorText->setColor({ 255, 0, 0 });
	errorText->setTag(124);
	m_pButtonMenu->addChild(errorText);


	if (RouletteManager.isPlayingRoulette)
		onStartButton(nullptr);
	

	return true;
}

// called every frame after the ListFetcher thread has been detatched
void RouletteLayer::update(float dt)
{
	static int tries = 0;

	if (ListFetcher::finishedFetching)
	{
		if ((list.size() == 0 || listLevel.size() == 0) && tries <= 5)
		{
			tries++;
			int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
			int diffInd = getIndexOf(RouletteManager.difficultyArr, true);
			int demonInd = getIndexOf(RouletteManager.demonDifficultyArr, true);
			int difficulty = diffInd == 5 ? (demonInd + 6) : (diffInd + 1);

			listType == 0
				? getlistLevel(this, difficulty, std::ref(list))
				: getlistLevel(this, 0, std::ref(listLevel));
			
			return;
		}
		finishLevelRoulette();
		this->unscheduleUpdate();
	}
}

void RouletteLayer::onClose(CCObject* sender)
{
	setKeypadEnabled(false);
	removeFromParentAndCleanup(true);
}

void RouletteLayer::onInfoButton(CCObject* sender)
{
	if (!RouletteManager.isPlayingRoulette)
		RouletteInfoLayer::create()->show();
	else
		this->addChild(gd::TextAlertPopup::create("You are currently in a game of roulette!", 1.2f, .8f));
}

void RouletteLayer::onDifficultyChosen(CCObject* sender)
{
	if (
		auto ind = getIndexOf(RouletteManager.togglesStatesArr, true);
		ind != 0
		)
		return;

	ccColor3B color{};
	auto tag = sender->getTag();
	auto difficultyButton = reinterpret_cast<gd::CCMenuItemSpriteExtra*>(sender);
	auto& diffArr = RouletteManager.difficultyArr;
	auto& demonDiffArr = RouletteManager.demonDifficultyArr;

	// check if difficultyButton is one of the demon types and not a regular difficulty
	if (
		reinterpret_cast<gd::CCMenuItemSpriteExtra*>(m_pButtonMenu->getChildByTag(11))->isVisible() &&
		tag > 5 && (tag < 10 || tag > 5)
		) {
		auto ind = getIndexOf(demonDiffArr, true);
		reinterpret_cast<gd::CCMenuItemSpriteExtra*>(
			m_pButtonMenu->getChildByTag(ind + 6)
		)->setColor({ 125, 125, 125 });

		demonDiffArr.at(ind) = false;
		demonDiffArr.at(tag - 6) = true;

		difficultyButton->setColor({ 255, 255, 255 });
	}
	else
	{
		if (isPlusButtonToggled && tag != 5)
			onPlusButton(nullptr);

		auto ind = getIndexOf(diffArr, true);
		reinterpret_cast<gd::CCMenuItemSpriteExtra*>(
			m_pButtonMenu->getChildByTag(ind)
		)->setColor({ 125, 125, 125 });

		diffArr.at(ind) = false;
		diffArr.at(tag) = true;

		difficultyButton->setColor({ 255, 255, 255 });

		// demon
		m_pButtonMenu->getChildByTag(11)->setVisible(diffArr[5]);
	}
}

void RouletteLayer::onStartButton(CCObject* sender)
{
	if (RouletteManager.isPlayingRoulette)
	{
		for (int i = 0; i < 13; i++)
			m_pButtonMenu->getChildByTag(i)->setVisible(false);

		finishLevelRoulette();
		return;
	}

	for (int i = 0; i < 13; i++)
		m_pButtonMenu->getChildByTag(i)->setVisible(false);

	int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
	int diffInd = getIndexOf(RouletteManager.difficultyArr, true);
	int demonInd = getIndexOf(RouletteManager.demonDifficultyArr, true);
	int difficulty = diffInd == 5 ? (demonInd + 6) : (diffInd + 1);

	getlistLevel(this, difficulty, listType == 0 ? list : listLevel);

	levelLoadingCircle->setVisible(true);
	levelLoadingCircle->runAction(CCRepeatForever::create(cocos2d::CCRotateBy::create(1.f, 360)));

	RouletteManager.isPlayingRoulette = true;
}

void RouletteLayer::onPlusButton(CCObject* sender)
{
	isPlusButtonToggled = !isPlusButtonToggled;

	if (isPlusButtonToggled)
	{
		for (int i = 0; i < 6; i++)
			this->m_pButtonMenu->getChildByTag(i)->setPositionY(i < 3 ? 64.f : 10.f);

		this->m_pButtonMenu->getChildByTag(11)->setPositionY(15.f);
		this->m_pButtonMenu->getChildByTag(12)->setPositionY(-90.f);

		for (int i = 6; i < 11; i++)
			this->m_pButtonMenu->getChildByTag(i)->setVisible(true);
	}
	else
	{
		for (int i = 0; i < 6; i++)
			this->m_pButtonMenu->getChildByTag(i)->setPositionY(i < 3 ? 40.f : -25.f);

		this->m_pButtonMenu->getChildByTag(11)->setPositionY(-20.f);
		this->m_pButtonMenu->getChildByTag(12)->setPositionY(-85.f);

		for (int i = 6; i < 11; i++)
			this->m_pButtonMenu->getChildByTag(i)->setVisible(false);
	}
}

void RouletteLayer::onLevelInfo(CCObject* sender)
{
	auto buttonItem = reinterpret_cast<gd::CCMenuItemSpriteExtra*>(sender);
	int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
	std::string text;
	nlohmann::json level = listType == 0 ? list[lastListIndex] : listLevel;

	switch (buttonItem->getTag())
	{
	case 100:
		text = level["name"].get<std::string>();
		if (text.size() > 15)
			buttonItem->setScale(.75f);
		break;
	case 101:
		buttonItem->setScale(.8f);
		text = level["author"].get<std::string>();
		break;
	case 102:
		buttonItem->setScale(.5f);
		text = level["id"].get<std::string>();
		break;
	}
	
	ImGui::SetClipboardText(text.c_str());
	this->addChild(gd::TextAlertPopup::create("Copied to Clipboard", .5f, .6f));
}

void RouletteLayer::onPlayButton(CCObject* sender)
{
	if (!ListFetcher::finishedFetching)
		return;


	int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
	nlohmann::json levelJson = listType == 0 ? list[lastListIndex] : listLevel;

	auto level = gd::GJGameLevel::create();
	level->levelID_rand = std::stoi(levelJson["id"].get<std::string>()) + 5;
	level->levelID_seed = 5;
	level->levelID = std::stoi(levelJson["id"].get<std::string>());
	level->levelName = levelJson["name"].get<std::string>();
	level->userName = levelJson["author"].get<std::string>();
	level->accountID = std::stoi(levelJson["accountID"].get<std::string>());
	level->userID = std::stoi(levelJson["playerID"].get<std::string>());
	try
	{
		level->songID = levelJson["songID"].get<int>();
	}
	catch (...)
	{
		// song is not a custom one, returns "Level 1", remove "Level "
		level->audioTrack = std::stoi(levelJson["songID"].get<std::string>().substr(6));
	}
	level->levelNotDownloaded = true;
	level->levelType = gd::GJLevelType::kGJLevelTypeSaved;

	auto layer = gd::LevelInfoLayer::create(level);
	layer->downloadLevel();
	auto scene = CCScene::create();
	scene->addChild(layer);
	CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
}

void RouletteLayer::onNextButton(CCObject* sender)
{
	if (!ListFetcher::finishedFetching)
		return;

	if (RouletteManager.levelPercentageGoal == 101)
	{
		for (int i = 0; i < 13; i++)
			m_pButtonMenu->getChildByTag(100 + i)->setVisible(false);

		for (int i = 0; i < 8; i++)
			m_pButtonMenu->getChildByTag(113 + i)->setVisible(false);

		reinterpret_cast<gd::CCMenuItemSpriteExtra*>(m_pButtonMenu->getChildByTag(121))->setPosition({ .0f, -70.f });
		m_pButtonMenu->getChildByTag(122)->setVisible(true);
		reinterpret_cast<CCLabelBMFont*>(m_pButtonMenu->getChildByTag(123))->setString(
			CCString::createWithFormat("Skips Used: %d", RouletteManager.skipsCount)->getCString()
		);
		m_pButtonMenu->getChildByTag(123)->setVisible(true);
		return;
	}

	if (RouletteManager.lastLevelPercentage != 0 && RouletteManager.hasFinishedPreviousLevel)
	{
		RouletteManager.hasFinishedPreviousLevel = false;

		levelLoadingCircle->setPositionY(-125.f);
		levelLoadingCircle->setVisible(true);
		levelLoadingCircle->runAction(CCRepeatForever::create(cocos2d::CCRotateBy::create(1.f, 360)));

		for (int i = 0; i < 13; i++)
			m_pButtonMenu->getChildByTag(100 + i)->setVisible(false);

		for (int i = 0; i < 3; i++)
			m_pButtonMenu->getChildByTag(114 + i)->setVisible(false);

		reinterpret_cast<CCLabelBMFont*>(
			m_pButtonMenu->getChildByTag(117)
		)->setString(CCString::createWithFormat("%d%%", RouletteManager.lastLevelPercentage)->getCString());

		int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
		int diffInd = getIndexOf(RouletteManager.difficultyArr, true);
		int demonInd = getIndexOf(RouletteManager.demonDifficultyArr, true);
		int difficulty = diffInd == 5 ? (demonInd + 6) : (diffInd + 1);

		getlistLevel(this, difficulty, listType == 0 ? list : listLevel);
	}
	else
	{
		this->addChild(gd::TextAlertPopup::create(
			CCString::createWithFormat(
				"You need to get at least %d%%", RouletteManager.levelPercentageGoal
			)->getCString(), 1.2f, .8f
		));
	}
}

void RouletteLayer::onResetButton(CCObject* sender)
{
	RouletteManager.isPlayingRoulette = false;
	RouletteManager.hasFinishedPreviousLevel = false;
	RouletteManager.lastLevelID = 0;
	RouletteManager.lastLevelPercentage = 0;
	RouletteManager.levelPercentageGoal = 1;
	RouletteManager.skipsCount = 0;

	list = {};
	lastListIndex = 0;

	for (int i = 0; i < 6; i++)
	{
		m_pButtonMenu->getChildByTag(i)->setPositionY(i < 3 ? 40.f : -25.f);
		m_pButtonMenu->getChildByTag(i)->setVisible(true);
	}

	for (int i = 0; i < 6; i++)
		m_pButtonMenu->getChildByTag(6 + i)->setVisible(false);

	for (int i = 0; i < 25; i++)
		m_pButtonMenu->getChildByTag(100 + i)->setVisible(false);

	reinterpret_cast<CCLabelBMFont*>(m_pButtonMenu->getChildByTag(117))->setString("0%");
	m_pButtonMenu->getChildByTag(121)->setPosition({ 120.f, -70.f });

	m_pButtonMenu->getChildByTag(11)->setPositionY(-20.f);
	m_pButtonMenu->getChildByTag(12)->setVisible(true);
	m_pButtonMenu->getChildByTag(12)->setPositionY(-85.f);

	if (RouletteManager.difficultyArr[5])
	{
		isPlusButtonToggled = false;
		m_pButtonMenu->getChildByTag(11)->setVisible(true);
	}

	levelLoadingCircle->setPositionY(-170.f);
}

void RouletteLayer::onSkipButton(CCObject* sender)
{
	if (!ListFetcher::finishedFetching)
		return;

	if (RouletteManager.skipsCount < RouletteManager.skipsMax)
	{
		RouletteManager.skipsCount++;
		RouletteManager.hasFinishedPreviousLevel = false;

		levelLoadingCircle->setPositionY(-125.f);
		levelLoadingCircle->setVisible(true);
		levelLoadingCircle->runAction(CCRepeatForever::create(cocos2d::CCRotateBy::create(1.f, 360)));

		for (int i = 0; i < 13; i++)
			m_pButtonMenu->getChildByTag(100 + i)->setVisible(false);

		for (int i = 0; i < 3; i++)
			m_pButtonMenu->getChildByTag(114 + i)->setVisible(false);

		int diffInd = getIndexOf(RouletteManager.difficultyArr, true);
		int demonInd = getIndexOf(RouletteManager.demonDifficultyArr, true);
		int difficulty = diffInd == 5 ? (demonInd + 6) : (diffInd + 1);

		getlistLevel(this, difficulty, list);
	}
	else
	{
		this->addChild(
			gd::TextAlertPopup::create(
				"You don't have any skips left!", 1.2f, .8f
			)
		);
	}
}

void RouletteLayer::finishLevelRoulette()
{
	levelLoadingCircle->stopAllActions();
	levelLoadingCircle->setVisible(false);

	int listType = getIndexOf(RouletteManager.togglesStatesArr, true);
	nlohmann::json level;

	if (list.size() == 0 || listLevel.size() == 0)
	{
		for (int i = 0; i < 11; i++)
			m_pButtonMenu->getChildByTag(113 + i)->setVisible(false);

		m_pButtonMenu->getChildByTag(121)->setVisible(true);
		m_pButtonMenu->getChildByTag(124)->setVisible(true);
		return;
	}
	
	if (!RouletteManager.isPlayingRoulette)
		lastListIndex = Hacks::randomInt(0, list.size() - 1);

	level = listType == 0 ? list[lastListIndex] : listLevel;

	RouletteManager.lastLevelID = std::stoi(level["id"].get<std::string>());

	if (level["name"].get<std::string>().size() > 15)
	{
		reinterpret_cast<CCLabelBMFont*>(
			m_pButtonMenu->getChildByTag(100)->getChildren()->objectAtIndex(0)
		)->setScale(.75f);
	}
	reinterpret_cast<CCLabelBMFont*>(
		m_pButtonMenu->getChildByTag(100)->getChildren()->objectAtIndex(0)
	)->setString(level["name"].get<std::string>().c_str());
	reinterpret_cast<CCLabelBMFont*>(
		m_pButtonMenu->getChildByTag(101)->getChildren()->objectAtIndex(0)
	)->setString(("by " + level["author"].get<std::string>()).c_str());
	reinterpret_cast<CCLabelBMFont*>(
		m_pButtonMenu->getChildByTag(102)->getChildren()->objectAtIndex(0)
	)->setString(("ID: " + level["id"].get<std::string>()).c_str());

	for (int i = 0; i < 3; i++)
		m_pButtonMenu->getChildByTag(100 + i)->setVisible(true);

	m_pButtonMenu->getChildByTag(113)->setVisible(true);

	for (int i = 0; i < 5; i++)
		m_pButtonMenu->getChildByTag(117 + i)->setVisible(true);

	auto diffInd = getIndexOf(RouletteManager.difficultyArr, true);
	auto demonInd = getIndexOf(RouletteManager.demonDifficultyArr, true);
	int difficultyTag = difficultyToTag[level["difficulty"].get<std::string>()];

	if (int coins = level["coins"].get<int>(); coins > 0)
	{
		m_pButtonMenu->getChildByTag(difficultyTag)->setPositionY(55.f);

		for (int i = 0; i < 3; i++)
			m_pButtonMenu->getChildByTag(113 + i + 1)->setPosition({ -120.f + 10.f * i, 13.f });

		switch (coins)
		{
		case 1:
			m_pButtonMenu->getChildByTag(115)->setVisible(true);
			if (diffInd == 5)
				m_pButtonMenu->getChildByTag(115)->setPositionY(0.f);
			break;
		case 2:
			m_pButtonMenu->getChildByTag(114)->setVisible(true);
			m_pButtonMenu->getChildByTag(115)->setVisible(true);

			m_pButtonMenu->getChildByTag(114)->setPositionX(-115.f);
			m_pButtonMenu->getChildByTag(115)->setPositionX(-105.f);

			if (diffInd == 5)
			{
				m_pButtonMenu->getChildByTag(114)->setPositionY(0.f);
				m_pButtonMenu->getChildByTag(115)->setPositionY(0.f);
			}
			break;
		case 3:
			for (int i = 0; i < coins; i++)
			{
				m_pButtonMenu->getChildByTag(113 + i + 1)->setVisible(true);
				if (diffInd == 5)
					m_pButtonMenu->getChildByTag(113 + i + 1)->setPositionY(0.f);
			}
			break;
		}
	}
	else
		m_pButtonMenu->getChildByTag(difficultyTag)->setPositionY(40.f);

	m_pButtonMenu->getChildByTag(difficultyTag)->setVisible(true);
}


gd::CCMenuItemSpriteExtra* RouletteLayer::createDifficultyButton(int tag, CCNode* sprite, CCPoint point, float scale, bool isDemon, bool visible)
{
	sprite->setScale(scale);
	auto button = gd::CCMenuItemSpriteExtra::create(
		sprite,
		this,
		menu_selector(RouletteLayer::onDifficultyChosen)
	);
	button->setPosition(point);
	button->setTag(tag);
	if (isDemon)
	{
		if (!RouletteManager.difficultyArr[tag - 6])
			button->setColor({ 125, 125, 125 });
	}
	else
	{
		if (!RouletteManager.difficultyArr[tag])
			button->setColor({ 125, 125, 125 });
	}
	button->setVisible(visible);
	m_pButtonMenu->addChild(button);

	return button;
}

gd::CCMenuItemToggler* RouletteInfoLayer::createToggler(int tag, const char* labelText, CCPoint point, bool visible)
{
	auto buttonSpriteOn = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
	auto buttonSpriteOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
	buttonSpriteOn->setScale(.8f);
	buttonSpriteOff->setScale(.8f);

	auto button = gd::CCMenuItemToggler::create(
		buttonSpriteOff,
		buttonSpriteOn,
		this,
		menu_selector(RouletteInfoLayer::onToggleButton)
	);
	button->setPosition(point);
	button->setSizeMult(1.2f);
	button->setTag(tag);
	button->setVisible(visible);
	button->toggle(RouletteManager.togglesStatesArr.at(tag));
	m_pButtonMenu->addChild(button);

	auto label = createTextLabel(labelText, { point.x + 20, point.y }, .5f, m_pButtonMenu);
	label->setAnchorPoint({.0f, .5f });
	label->limitLabelWidth( 80.f, .5f, 0);
}
