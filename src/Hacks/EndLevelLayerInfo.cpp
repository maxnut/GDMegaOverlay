#include "EndLevelLayerInfo.h"
#include "Labels.h"
#include <cstring>

#include <Geode/modify/EndLevelLayer.hpp>

#include <Geode/Utils.hpp>
#include <Geode/platform/windows.hpp>

#include <Geode/Geode.hpp>

#include "../util.hpp"
#include "../Settings.hpp"

using namespace geode::prelude;
using namespace EndLevelLayerInfo;

template<typename T, typename U> constexpr size_t offsetOf(U T::*member)
{
	return (char*)&((T*)nullptr->*member) - (char*)nullptr;
}

// move to utils.hpp if other files need it in the future
namespace
{
	// https://github.com/matcool/CocosExplorer
	inline const char* getFrameName(cocos2d::CCSprite* sprite_node)
	{
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

class $modify(EndLevelLayer)
{
	void showLayer(bool unk)
	{
		EndLevelLayer::showLayer(unk);

		if (!Settings::get<bool>("level/endlevellayerinfo/enabled")) return;

		auto layer = reinterpret_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));


		float coinsPositions3[3]{ 150.f, 207.f, 265.f };
		float coinsPositions2[2]{ 170.f, 227.f };
		float coinsPositions1[1]{ 194.f };
		std::array<cocos2d::CCSprite*, 3> coins{
			nullptr, nullptr, nullptr
		};
		std::array<cocos2d::CCSprite*, 3> coinsBg{
			nullptr, nullptr, nullptr
		};
		std::array<cocos2d::CCNode*, 4> nodes{
			// stars
			nullptr,
			// orbs
			nullptr,
			// diamonds
			nullptr,
			// keys
			nullptr
		};
		cocos2d::CCLabelBMFont* completionLabel = nullptr;
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
			auto node = reinterpret_cast<CCNode*>(layer->getChildren()->objectAtIndex(i));

			// typeinfo_cast prevents from considering the restart, edit and menu buttons as a CCNode
			if (node && node->getChildrenCount() == 2 && typeinfo_cast<CCSprite*>(node->getChildren()->objectAtIndex(0)))
			{
				nodes.at(nodesCount) = node;
				nodesCount++;
			}

			if (auto sprite = typeinfo_cast<CCSprite*>(node); sprite)
			{
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

			if (auto label = typeinfo_cast<CCLabelBMFont*>(node); label)
			{
				std::string labelText = label->getString();

				// wtf
				if (
					!labelText.starts_with("Attempts: ") && !labelText.starts_with("Jumps: ") &&
					!labelText.starts_with("Time: ") && !labelText.starts_with("Accuracy: ") &&
					!labelText.starts_with("Deaths: ")
					)
					completionLabel = label;
			}
		}


		if (util::getElementCount(nodes, nullptr) <= 2)
		{
			switch (util::getElementCount(coins, nullptr))
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

			case 3:
				if (completionLabel)
				{
					completionLabel->setPositionX(200.f);
					completionLabel->limitLabelWidth(200.f, .9f, .3f);
				}

				break;

			default: break;
			}
		}

		if (nodes.at(0))
			nodes.at(0)->setPosition({ 335.f, 96.f });

		if (nodes.at(1))
			nodes.at(1)->setPosition({ 420.f, 96.f });

		// TODO: these aren't getting found?
		if (nodes.at(2) && nodes.at(3))
			nodes.at(2)->setPosition({ 335.f, 125.f });
		else if (nodes.at(2))
			nodes.at(2)->setPosition({ 475.f, 125.f });

		if (nodes.at(3) && nodes.at(2))
			nodes.at(3)->setPosition({ 420.f, 125.f });
		else if (nodes.at(3))
			nodes.at(3)->setPosition({ 475.f, 125.f });
	}

	void customSetup()
	{
		EndLevelLayer::customSetup();

		if (!Settings::get<bool>("level/endlevellayerinfo/enabled")) return;

		auto layer = reinterpret_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
		auto playLayer = PlayLayer::get();


		CCPoint textPosition{ 0.f, 171.f }; // Y pos is that of Jumps label

		// prevents moving the EndLevelLayer completion string
		int labelsCount = 0;
		int labelsCountLimit = 3;
		for (unsigned int i = 0; i < layer->getChildrenCount(); i++)
		{
			auto node = reinterpret_cast<CCNode*>(layer->getChildren()->objectAtIndex(i));

			if (labelsCount < labelsCountLimit && typeinfo_cast<CCLabelBMFont*>(node))
			{
				if (playLayer->m_player1->m_isPlatformer)
				{
					// unk m_unkPoints; [[EndLevelLayer + 0x1E0] + 0x5D8]
					if (MBO(int, MBO(void*, this, 0x1E0), 0x5D8))
					{
						// manual centering of time/points label (this sucks)
						node->setPositionY(node->getPositionY() + (labelsCount == 0 ? 3.f : -3.f));

						labelsCountLimit = 2;
					}
					else
					{
						node->setPositionY(node->getPositionY() - 13.f);
						labelsCountLimit = 1;
					}
				}

				textPosition.x = node->getPositionX();
				node->setPositionX(textPosition.x - 90.f);
				labelsCount++;
			}
		}


		auto noclipAccuracyLabelELL = CCLabelBMFont::create(
			CCString::createWithFormat(
				"Accuracy: %.2f%%",
				(static_cast<float>(Labels::frames - Labels::deaths) / static_cast<float>(Labels::frames)) * 100.f
			)->getCString(),
			"goldFont.fnt"
		);
		noclipAccuracyLabelELL->limitLabelWidth(180.f, .8f, .5f);
		noclipAccuracyLabelELL->setPosition({ textPosition.x + 80.f, textPosition.y + 15.f });
		layer->addChild(noclipAccuracyLabelELL);

		auto noclipDeathsLabelELL = CCLabelBMFont::create(
			CCString::createWithFormat("Deaths: %i", Labels::realDeaths)->getCString(),
			"goldFont.fnt"
		);
		noclipDeathsLabelELL->limitLabelWidth(180.f, .8f, .5f);
		noclipDeathsLabelELL->setPosition({ textPosition.x + 80.f, textPosition.y - 15.f });
		layer->addChild(noclipDeathsLabelELL);
	}
};
