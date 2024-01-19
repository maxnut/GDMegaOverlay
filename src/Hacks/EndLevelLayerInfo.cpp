#include <cocos2d.h>
#include "EndLeveLayerInfo.h"
#include "Labels.h"
#include "../Settings.h"
#include "../Common.h"
#include "../utils.hpp"

void EndLevelLayerInfo::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0xE74F0), endLevelLayerCustomSetupHook,
				  reinterpret_cast<void**>(&endLevelLayerCustomSetup));
}

void __fastcall EndLevelLayerInfo::endLevelLayerCustomSetupHook(cocos2d::CCLayer* self, void*)
{
	endLevelLayerCustomSetup(self);

	auto layer = reinterpret_cast<cocos2d::CCLayer*>(self->getChildren()->objectAtIndex(0));
	auto playLayer = MBO(cocos2d::CCLayer*, Common::gameManager, 0x198);
	cocos2d::CCPoint textPosition{ 0.f, 171.f }; // Y pos is that of Jumps label

	if (!Settings::get<bool>("level/endlevellayerinfo/enabled")) return;

	int j = 0;
	for (unsigned int i = 0; i < layer->getChildrenCount(); i++)
	{
		if (
			auto node = reinterpret_cast<cocos2d::CCNode*>(layer->getChildren()->objectAtIndex(i));
			utils::getClassName(node) == "cocos2d::CCLabelBMFont"
		) {
			textPosition.x = node->getPositionX();
			reinterpret_cast<cocos2d::CCLabelBMFont*>(node)->setPositionX(textPosition.x - 90.f);
			j++;
		}

		// prevents moving the EndLevelLayer completion string
		if (j > 2)
			break;
	}


	auto noclipAccuracyLabelELL = cocos2d::CCLabelBMFont::create(
		cocos2d::CCString::createWithFormat(
			"Accuracy: %.2f%%",
			(static_cast<float>(Labels::frames - Labels::deaths) / static_cast<float>(Labels::frames)) * 100.f
		)->getCString(),
		"goldFont.fnt"
	);
	noclipAccuracyLabelELL->limitLabelWidth(180.f, .8f, .5f);
	noclipAccuracyLabelELL->setPosition({ textPosition.x + 80.f, textPosition.y + 15.f });
	layer->addChild(noclipAccuracyLabelELL);

	auto noclipDeathsLabelELL = cocos2d::CCLabelBMFont::create(
		cocos2d::CCString::createWithFormat("Deaths: %i", Labels::deaths)->getCString(),
		"goldFont.fnt"
	);
	noclipDeathsLabelELL->limitLabelWidth(180.f, .8f, .5f);
	noclipDeathsLabelELL->setPosition({ textPosition.x + 80.f, textPosition.y - 15.f });
	layer->addChild(noclipDeathsLabelELL);
}
