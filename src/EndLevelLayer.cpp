#include "EndLevelLayer.h"
#include "bools.h"
#include "PlayLayer.h"
#include "Hacks.h"

int EndLevelLayer::deaths = 0;
float EndLevelLayer::accuracy = 0;
extern struct HacksStr hacks;

const char *getCCSwitchIntFloat(const char *txtFloat, const char *txtInt, float nf, bool isAcurr)
{
    int SafeDecimal = (int)(std::floor(nf));
    auto ret = isAcurr ? cocos2d::CCString::createWithFormat(txtFloat, nf) : cocos2d::CCString::createWithFormat(txtInt, SafeDecimal);
    return ret->getCString();
}

std::string getPerStrConeAcurr(const char *txt)
{
    std::stringstream stream;
    stream << txt << "%i%%";
    return stream.str();
}

void ButtonsClass::Reset()
{
    hacks.lastCheckpoint = true;
    PlayLayer::resetLevelHook(gd::GameManager::sharedState()->getPlayLayer(), 0);
    hacks.lastCheckpoint = false;
}

bool __fastcall EndLevelLayer::customSetupHook(CCLayer *self, void *)
{
    auto ret = EndLevelLayer::customSetup(self);

    if(!hacks.showExtraInfo)return ret;

    auto layerMain = (cocos2d::CCLayer *)self->getChildren()->objectAtIndex(0);
    cocos2d::CCMenu *returnButton = {};
    cocos2d::CCMenu *buttonsMains = {};
    bool isFindTextArea = 0;
    gd::TextArea *textAreaEndScreem = {};
    auto attemptsLabel = (cocos2d::CCLabelBMFont *)layerMain->getChildren()->objectAtIndex(5);
    auto jumpsLabel = (cocos2d::CCLabelBMFont *)layerMain->getChildren()->objectAtIndex(6);
    auto timeLabel = (cocos2d::CCLabelBMFont *)layerMain->getChildren()->objectAtIndex(7);
    auto messageLabelNormalMode = (cocos2d::CCLabelBMFont *)layerMain->getChildren()->objectAtIndex(8);

    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto director = CCDirector::sharedDirector();
    auto playLayer = gd::GameManager::sharedState()->getPlayLayer();

    for (unsigned int i = 0; i < layerMain->getChildrenCount(); i++)
    {
        auto menuSus = (cocos2d::CCMenu *)layerMain->getChildren()->objectAtIndex(i);
        if (nameChildFind(menuSus, "cocos2d::CCMenu"))
        {
            if (menuSus->getChildrenCount() == 1)
                returnButton = menuSus;
            else
                buttonsMains = menuSus;
        }
        if (nameChildFind(menuSus, "TextArea"))
        {
            textAreaEndScreem = (gd::TextArea *)layerMain->getChildren()->objectAtIndex(i);
            isFindTextArea = 1;
        }
    }

    if (hacks.safeModeEndscreen && ExternData::level["mods"][24]["toggle"])
    {
        if (isFindTextArea)
        {
            textAreaEndScreem->setString("- Safe Mode -");
            textAreaEndScreem->setScale(0.9f);
        }
        else if (nameChildFind(messageLabelNormalMode, "cocos2d::CCLabelBMFont"))
        {
            messageLabelNormalMode->setString("- Safe Mode -");
            messageLabelNormalMode->setScale(0.9f);
        }
        else
        {
            auto safeModeLabelTmp = cocos2d::CCLabelBMFont::create("- Safe Mode -", "bigFont.fnt");
            safeModeLabelTmp->setScale(0.5f);
            safeModeLabelTmp->setPosition({winSize.width / 2.f, (winSize.height / 2) + 102.f}); // winSize().height - 58.f
            layerMain->addChild(safeModeLabelTmp);
        }
    }

    if (hacks.practiceButtonEndscreen)
    {
        if (false)
        {
            auto buttonOriginal = ((gd::CCMenuItemSpriteExtra *)(returnButton->getChildren()->objectAtIndex(0)));
            buttonOriginal->setVisible(true);
        }

        if (playLayer->m_isPracticeMode)
        {
            auto buttonsReset = cocos2d::CCArray::create();
            const int childrenCountSafe = buttonsMains->getChildrenCount();
            float padHorRespec = childrenCountSafe == 2 ? 38.f : 15.f;

            for (int i = 0; i < childrenCountSafe; i++)
            {
                if (i + 1 != childrenCountSafe)
                {
                    auto objTmpExtra = (gd::CCMenuItemSpriteExtra *)buttonsMains->getChildren()->lastObject();
                    buttonsReset->insertObject(objTmpExtra, i);
                    buttonsMains->removeChild(objTmpExtra, false);
                }
                else
                {
                    auto practiceImageEndScreen = cocos2d::CCSprite::createWithSpriteFrameName("GJ_practiceBtn_001.png");
                    practiceImageEndScreen->setScale(1.f);
                    auto practiceButtonEndScreen = gd::CCMenuItemSpriteExtra::create(practiceImageEndScreen, buttonsMains, (cocos2d::SEL_MenuHandler)&ButtonsClass::ResetInPractice);
                    buttonsMains->addChild(practiceButtonEndScreen);
                    for (size_t j = 0; j < buttonsReset->count(); j++)
                    {
                        auto objTmpExtra = (gd::CCMenuItemSpriteExtra *)buttonsReset->objectAtIndex(j);
                        buttonsMains->addChild(objTmpExtra);
                    }
                    buttonsMains->alignItemsHorizontallyWithPadding(padHorRespec);
                }
            }
            for (int i = 0; i < childrenCountSafe + 1; i++)
            {
                auto objTmpExtra = (gd::CCMenuItemSpriteExtra *)buttonsMains->getChildren()->objectAtIndex(i);
                objTmpExtra->setPositionY(-125.f);
            }
        }

        auto backgroundButton = cocos2d::CCSprite::createWithSpriteFrameName("GJ_longBtn02_001.png");
        auto flecha = cocos2d::CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
        flecha->setRotation(-90.f);
        flecha->setScaleX(0.6f);
        flecha->setScaleY(0.7f);
        flecha->setPosition({46.f, 16.f});
        backgroundButton->addChild(flecha);
        backgroundButton->setScaleX(0.5f);
        backgroundButton->setScaleY(0.7f);
        auto buttonExtraItem = gd::CCMenuItemSpriteExtra::create(backgroundButton, buttonsMains, (cocos2d::SEL_MenuHandler)&ButtonsClass::EndLayerXMod);
        buttonExtraItem->setPositionY(128.f);
        buttonsMains->addChild(buttonExtraItem);
    }

    if (hacks.cheatIndicatorEndscreen)
    {

        auto cheatIndicatorNow = cocos2d::CCLabelBMFont::create(".", "bigFont.fnt");
        cheatIndicatorNow->setScale(1.f);
        cheatIndicatorNow->setOpacity((GLubyte)100);
        cheatIndicatorNow->setPosition({(winSize.width / 2.f) + 168.f, (winSize.height / 2) + 110.f}); // 452.5f, 270.f
        PlayLayer::IsCheating() ? cheatIndicatorNow->setColor({255, 0, 0}) : cheatIndicatorNow->setColor({0, 255, 0});
        layerMain->addChild(cheatIndicatorNow);

        auto cheatIndicatorPass = cocos2d::CCLabelBMFont::create(".", "bigFont.fnt");
        cheatIndicatorPass->setScale(1.f);
        cheatIndicatorPass->setOpacity((GLubyte)100);
        cheatIndicatorPass->setPosition({(winSize.width / 2.f) + 158.f, (winSize.height / 2) + 110.f}); // 442.5f, 270.f
        cheatIndicatorPass->setColor({0, 255, 0});
        PlayLayer::IsCheating() ? cheatIndicatorPass->setColor({255, 0, 0}) : cheatIndicatorPass->setColor({0, 255, 0});
        layerMain->addChild(cheatIndicatorPass);
    }

    if (hacks.showExtraInfo)
    {

        auto checkpointsCounterLabelES = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");
        auto practiceStartLabelES = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");
        auto startPosStartLabelES = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");
        auto noclipAccuLabelES = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");
        auto noclipDeathLabelES = cocos2d::CCLabelBMFont::create("", "goldFont.fnt");

        std::vector<cocos2d::CCLabelBMFont *> labelsExtras = {checkpointsCounterLabelES, practiceStartLabelES, startPosStartLabelES, noclipAccuLabelES, noclipDeathLabelES};
        std::vector<cocos2d::CCLabelBMFont *> labelsOfficial = {attemptsLabel, jumpsLabel, timeLabel};

        for (size_t i = 0; i < labelsExtras.size(); i++)
        {
            labelsExtras[i]->setScale(0.8f);
            layerMain->addChild(labelsExtras[i], 3);
        }

        auto winSizeMediumW = winSize.width / 2.f;
        auto winSizeMediumH = winSize.height / 2.f;
        auto PosIzqES = winSizeMediumW - 78.f;
        auto PosDerES = winSizeMediumW + 85.f;

        auto checkpointsNow = playLayer->m_checkpoints->count();
        bool ReconfLabels = true;

        auto txtPrac = getCCSwitchIntFloat(getPerStrConeAcurr("Practice: ").c_str(), "Practice: %i%%", PlayLayer::GetStartPercent(), false);
        auto txtStarP = getCCSwitchIntFloat(getPerStrConeAcurr("StartPos: ").c_str(), "StartPos: %i%%", PlayLayer::GetStartPercent(), false);

        bool NcButtonConfirm = false;

        if (ExternData::player["mods"][0]["toggle"] || ExternData::player["mods"][2]["toggle"])
        {
            if (playLayer->m_isTestMode)
            {
                if (playLayer->m_isPracticeMode)
                {
                    attemptsLabel->setPosition(PosIzqES, winSizeMediumH + 46.f);
                    jumpsLabel->setPosition(PosIzqES, winSizeMediumH + 24.f);
                    timeLabel->setPosition(PosIzqES, winSizeMediumH + 2.f);
                    startPosStartLabelES->setPosition(PosIzqES, winSizeMediumH - 20.f);
                    noclipAccuLabelES->setPosition(PosDerES, winSizeMediumH + 46.f);
                    noclipDeathLabelES->setPosition(PosDerES, winSizeMediumH + 24.f);
                    practiceStartLabelES->setPosition(PosDerES, winSizeMediumH + 2.f);
                    checkpointsCounterLabelES->setPosition(PosDerES, winSizeMediumH - 20.f);
                    practiceStartLabelES->setString(txtPrac);
                    checkpointsCounterLabelES->setString(cocos2d::CCString::createWithFormat("CheckPoints: %i", checkpointsNow)->getCString());
                }
                else
                {
                    attemptsLabel->setPosition(PosIzqES, winSizeMediumH + 35.f);
                    jumpsLabel->setPosition(PosIzqES, winSizeMediumH + 11.f);
                    timeLabel->setPosition(PosIzqES, winSizeMediumH - 13.f);
                    startPosStartLabelES->setPosition(PosDerES, winSizeMediumH + 35.f);
                    noclipAccuLabelES->setPosition(PosDerES, winSizeMediumH + 11.f);
                    noclipDeathLabelES->setPosition(PosDerES, winSizeMediumH - 13.f);
                }
                startPosStartLabelES->setString(txtStarP);
            }
            else if (playLayer->m_isPracticeMode)
            {
                attemptsLabel->setPosition(PosIzqES, winSizeMediumH + 40.f);
                jumpsLabel->setPosition(PosIzqES, winSizeMediumH + 18.f);
                timeLabel->setPosition(PosIzqES, winSizeMediumH - 4.f);
                checkpointsCounterLabelES->setPosition(winSizeMediumW, winSizeMediumH - 26.f);
                practiceStartLabelES->setPosition(PosDerES, winSizeMediumH - 4.f);
                noclipAccuLabelES->setPosition(PosDerES, winSizeMediumH + 18.f);
                noclipDeathLabelES->setPosition(PosDerES, winSizeMediumH + 40.f);
                practiceStartLabelES->setString(txtPrac);
                checkpointsCounterLabelES->setString(cocos2d::CCString::createWithFormat("CheckPoints: %i", checkpointsNow)->getCString());
            }
            else
            {
                attemptsLabel->setPosition(PosIzqES, winSizeMediumH + 35.f);
                jumpsLabel->setPosition(PosIzqES, winSizeMediumH + 11.f);
                timeLabel->setPosition(PosDerES, winSizeMediumH + 35.f);
                noclipAccuLabelES->setPosition(winSizeMediumW, winSizeMediumH - 13.f);
                noclipDeathLabelES->setPosition(PosDerES, winSizeMediumH + 11.f);
            }

            noclipAccuLabelES->setString(cocos2d::CCString::createWithFormat("NoClip: %.2f%%", accuracy)->getCString());
            NcButtonConfirm = true;
            noclipDeathLabelES->setString(cocos2d::CCString::createWithFormat("Deaths: %i", deaths)->getCString());
        }
        else
        {
            if (playLayer->m_isTestMode)
            {
                if (playLayer->m_isPracticeMode)
                {
                    attemptsLabel->setPosition(PosIzqES, winSizeMediumH + 35.f);
                    jumpsLabel->setPosition(PosIzqES, winSizeMediumH + 11.f);
                    timeLabel->setPosition(PosIzqES, winSizeMediumH - 13.f);
                    checkpointsCounterLabelES->setPosition(PosDerES, winSizeMediumH - 13.f);
                    practiceStartLabelES->setPosition(PosDerES, winSizeMediumH + 11.f);
                    startPosStartLabelES->setPosition(PosDerES, winSizeMediumH + 35.f);
                    practiceStartLabelES->setString(txtPrac);
                    checkpointsCounterLabelES->setString(cocos2d::CCString::createWithFormat("Checkpoints: %i", checkpointsNow)->getCString());
                }
                else
                {
                    attemptsLabel->setPosition(winSizeMediumW, winSizeMediumH + 45.f);
                    jumpsLabel->setPosition(winSizeMediumW, winSizeMediumH + 23.f);
                    timeLabel->setPosition(winSizeMediumW, winSizeMediumH + 1.f);
                    startPosStartLabelES->setPosition(winSizeMediumW, winSizeMediumH - 21.f);
                }
                startPosStartLabelES->setString(txtStarP);
            }
            else if (playLayer->m_isPracticeMode)
            {
                attemptsLabel->setPosition(PosIzqES, winSizeMediumH + 35.f);
                jumpsLabel->setPosition(PosIzqES, winSizeMediumH + 11.f);
                timeLabel->setPosition(PosDerES, winSizeMediumH + 35.f);
                checkpointsCounterLabelES->setPosition(winSizeMediumW, winSizeMediumH - 13.f);
                practiceStartLabelES->setPosition(PosDerES, winSizeMediumH + 11.f);
                practiceStartLabelES->setString(txtPrac);
                checkpointsCounterLabelES->setString(cocos2d::CCString::createWithFormat("Checkpoints: %i", checkpointsNow)->getCString());
            }
            else
                ReconfLabels = false;
        }
        if (ReconfLabels)
        {
            for (size_t i = 0; i < labelsOfficial.size(); i++)
            {
                labelsOfficial[i]->setScale(0.7f);
                labelsOfficial[i]->limitLabelWidth(150.f, 0.7f, 0.f);
            }
            for (size_t i = 0; i < labelsExtras.size(); i++)
            {
                labelsExtras[i]->setScale(0.7f);
                labelsExtras[i]->limitLabelWidth(150.f, 0.7f, 0.f);
            }
        }
    }

    return ret;
}