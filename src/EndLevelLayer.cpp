#include "EndLevelLayer.h"
#include "bools.h"

int EndLevelLayer::deaths = 0;
std::string EndLevelLayer::accuracy = "";
extern struct HacksStr hacks;

void __fastcall EndLevelLayer::customSetupHook(CCLayer* self, void*)
{
    EndLevelLayer::customSetup(self);

    if(deaths <= 0 || !hacks.showExtraInfo) return;

    auto deathsLabel = CCLabelBMFont::create(("Deaths: " + std::to_string(deaths)).c_str(), "goldFont-uhd.fnt");
    auto accLabel = CCLabelBMFont::create(accuracy.c_str(), "goldFont-uhd.fnt");

    auto layer = static_cast<CCLayer*>(self->getChildren()->objectAtIndex(0));
    auto atts = static_cast<CCLabelBMFont*>(layer->getChildren()->objectAtIndex(5));
    auto jumps = static_cast<CCLabelBMFont*>(layer->getChildren()->objectAtIndex(6));
    auto time = static_cast<CCLabelBMFont*>(layer->getChildren()->objectAtIndex(7));

    atts->setAnchorPoint({0.0f, 0.5f});
    jumps->setAnchorPoint({0.0f, 0.5f});
    time->setAnchorPoint({0.0f, 0.5f});

    atts->setPositionX(time->getPositionX() - 140);
    jumps->setPositionX(time->getPositionX() - 140);
    time->setPositionX(time->getPositionX() - 140);

    deathsLabel->setPosition({atts->getPositionX() + 290, atts->getPositionY()});
    deathsLabel->setScale(0.8f);
    deathsLabel->setAnchorPoint({1.0f, 0.5f});

    accLabel->setPosition({jumps->getPositionX() + 290, jumps->getPositionY()});
    accLabel->setScale(0.65f);
    accLabel->setAnchorPoint({1.0f, 0.5f});

    layer->addChild(deathsLabel);
    layer->addChild(accLabel);
}