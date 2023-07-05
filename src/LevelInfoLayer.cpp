#include "LevelInfoLayer.h"
#define DECLAREROULETTEMANAGER
#include "RouletteManager.h"
#include "RouletteLayer.h"

class CustomDirector : public CCDirector
{
public:
	CCScene* getPreviousScene()
	{
		return reinterpret_cast<CCScene*>(m_pobScenesStack->objectAtIndex(m_pobScenesStack->count() - 2));
	}
};

namespace LevelInfoLayer
{
	void __fastcall LevelInfoLayer::onBackHook(gd::LevelInfoLayer* self, void*, CCObject* sender)
	{
		CustomDirector* director = reinterpret_cast<CustomDirector*>(CCDirector::sharedDirector());

		if (!RouletteManager.isPlayingRoulette || self->m_pLevel->levelID != RouletteManager.lastLevelID)
			onBack(self, sender);
		else
		{
			if (
				CCScene* prevScene = director->getPreviousScene();
				prevScene->getChildrenCount() == 2
				) {
				if (
					std::string_view name = (typeid(*prevScene->getChildren()->objectAtIndex(1)).name() + 6);
					name == "RouletteLayer"
					) {
					CCLayer* rouletteLayer = reinterpret_cast<CCLayer*>(
						director->getPreviousScene()->getChildren()->objectAtIndex(1)
						);

					CCMenu* rouletteMenu = reinterpret_cast<CCMenu*>(
						reinterpret_cast<CCLayer*>(
							rouletteLayer->getChildren()->objectAtIndex(0)
							)->getChildren()->objectAtIndex(1)
						);

					reinterpret_cast<CCLabelBMFont*>(
						rouletteMenu->getChildByTag(117)
						)->setString(CCString::createWithFormat("%d%%", RouletteManager.lastLevelPercentage)->getCString());
				}
			}

			onBack(self, sender);
		}
	}
}

