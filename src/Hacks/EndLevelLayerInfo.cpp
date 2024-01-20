#include <cstring>
#include <cocos2d.h>
#include "EndLevelLayerInfo.h"
#include "Labels.h"
#include "../Settings.h"
#include "../Common.h"
#include "../utils.hpp"

// move to utils.hpp if other files need it in the future
namespace
{
	// https://github.com/matcool/CocosExplorer
	inline const char* getFrameName(cocos2d::CCSprite* sprite_node)
	{
		using namespace cocos2d; // CCDICT_FOREACH breaks without this

		auto* texture = sprite_node->getTexture();

		CCDictElement* el;

		auto* frame_cache = CCSpriteFrameCache::sharedSpriteFrameCache();
		auto* cached_frames = public_cast(frame_cache, m_pSpriteFrames);
		const auto rect = sprite_node->getTextureRect();
		CCDICT_FOREACH(cached_frames, el)
		{
			auto* frame = static_cast<CCSpriteFrame*>(el->getObject());

			if (frame->getTexture() == texture && frame->getRect() == rect)
				return el->getStrKey();
		};

		return "none";
	}
}

void EndLevelLayerInfo::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0xE74F0), endLevelLayerCustomSetupHook,
				  reinterpret_cast<void**>(&endLevelLayerCustomSetup));
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0xE8C20), endLevelLayerPlayEndEffectHook,
				  reinterpret_cast<void**>(&endLevelLayerPlayEndEffect));
}

void __fastcall EndLevelLayerInfo::endLevelLayerCustomSetupHook(cocos2d::CCLayer* self, void*)
{
	endLevelLayerCustomSetup(self);

	if (!Settings::get<bool>("level/endlevellayerinfo/enabled")) return;

	auto layer = reinterpret_cast<cocos2d::CCLayer*>(self->getChildren()->objectAtIndex(0));
	auto playLayer = MBO(cocos2d::CCLayer*, Common::gameManager, 0x198);


	cocos2d::CCPoint textPosition{ 0.f, 171.f }; // Y pos is that of Jumps label

	// prevents moving the EndLevelLayer completion string
	int labelsCount = 0;
	int labelsCountLimit = 3;
	for (unsigned int i = 0; i < layer->getChildrenCount(); i++)
	{
		auto node = reinterpret_cast<cocos2d::CCNode*>(layer->getChildren()->objectAtIndex(i));

		if (labelsCount < labelsCountLimit && utils::getClassName(node) == "cocos2d::CCLabelBMFont")
		{
			// bool m_isPlatformer; [[PlayLayer + 0x878] + 0x920]
			if (MBO(bool, MBO(cocos2d::CCNode*, playLayer, 0x878), 0x920))
			{
				labelsCountLimit = 1;
				node->setPositionY(node->getPositionY() - 10.f);
			}

			textPosition.x = node->getPositionX();
			node->setPositionX(textPosition.x - 90.f);
			labelsCount++;
		}
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

void __fastcall EndLevelLayerInfo::endLevelLayerPlayEndEffectHook(cocos2d::CCLayer* self, void*, bool unk)
{
	endLevelLayerPlayEndEffect(self, unk);

	if (!Settings::get<bool>("level/endlevellayerinfo/enabled")) return;

	auto layer = reinterpret_cast<cocos2d::CCLayer*>(self->getChildren()->objectAtIndex(0));


	float coinsPositions3[3]{ 150.f, 207.f, 265.f };
	float coinsPositions2[2]{ 170.f, 227.f };
	float coinsPositions1[1]{ 194.f };
	std::array<cocos2d::CCSprite*, 3> coins{
		nullptr, nullptr, nullptr
	};
	std::array<cocos2d::CCSprite*, 3> coinsBg{
		nullptr, nullptr, nullptr
	};
	std::array<cocos2d::CCNode*, 3> nodes{
		// stars // orbs  // keys
		nullptr, nullptr, nullptr
	};
	auto setCoinXPos = [&](std::size_t index, float position)
	{
		if (coins.at(index))
			coins.at(index)->setPositionX(position);
		if (coinsBg.at(index))
			coinsBg.at(index)->setPositionX(position);
	};

	int nodesCount = 0;
	int coinsCount = 0;
	int coinsBgCount = 0;
	for (unsigned int i = 0; i < layer->getChildrenCount(); i++)
	{
		auto node = reinterpret_cast<cocos2d::CCNode*>(layer->getChildren()->objectAtIndex(i));

		if (utils::getClassName(node) == "cocos2d::CCNode")
		{
			nodes.at(nodesCount) = reinterpret_cast<cocos2d::CCNode*>(node);
			nodesCount++;
		}

		if (utils::getClassName(node) == "cocos2d::CCSprite")
		{
			auto sprite = reinterpret_cast<cocos2d::CCSprite*>(node);

			if (
				std::strcmp(getFrameName(sprite), "secretCoin_2_b_01_001.png") == 0 ||
				std::strcmp(getFrameName(sprite), "secretCoin_b_01_001.png") == 0
			) {
				coins.at(coinsCount) = sprite;
				coinsCount++;
			}
			else if (
				std::strcmp(getFrameName(sprite), "secretCoinUI2_001.png") == 0 ||
				std::strcmp(getFrameName(sprite), "secretCoinUI_001.png") == 0
			) {
				coinsBg.at(coinsBgCount) = sprite;
				coinsBgCount++;
			}
		}
	}


	if (utils::getElementCount(nodes, nullptr) <= 2)
	{
		switch (utils::getElementCount(coins, nullptr))
		{
		case 0:
			for (std::size_t i = 0; i < coins.size(); i++)
				setCoinXPos(i, coinsPositions3[i]);

			break;

		case 1:
			for (std::size_t i = 0; i < coins.size(); i++)
				setCoinXPos(i, coinsPositions2[i]);

			break;

		case 2:
			for (std::size_t i = 0; i < coins.size(); i++)
				setCoinXPos(i, coinsPositions1[i]);

			break;

		default: break;
		}
	}

	if (nodes.at(0))
		nodes.at(0)->setPosition({ 335.f, 96.f });

	if (nodes.at(1))
		nodes.at(1)->setPosition({ 420.f, 96.f });

	// TODO; where the hell is this going
	if (nodes.at(2))
		nodes.at(2)->setPosition({ 475.f, 125.f });
}
