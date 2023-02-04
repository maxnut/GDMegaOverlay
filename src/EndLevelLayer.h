#pragma once
#include "pch.h"
#include "PlayLayer.h"

namespace EndLevelLayer
{
	static bool nameChildFind(cocos2d::CCObject *obj, const char *name)
	{
		std::string nameChild = (typeid(*obj).name() + 6);
		return nameChild == name;
	}

	extern int deaths;
	extern float accuracy;

	inline bool(__thiscall *customSetup)(CCLayer *self);
	bool __fastcall customSetupHook(CCLayer *self, void *);

}

class ButtonsClass final : public gd::FLAlertLayer, public cocos2d::CCTextFieldDelegate, public gd::FLAlertLayerProtocol
{
public:
	void ResetInPractice(CCObject *pSender)
	{

		auto layerAEndScreen = (cocos2d::CCLayer *)this->getParent();
		auto layerEndScreen = (gd::GJDropDownLayer *)layerAEndScreen->getParent();

		cocos2d::CCMenu *buttonModEndLayer = {};

		for (unsigned int i = 0; i < layerAEndScreen->getChildrenCount(); i++)
		{
			auto menuSus = (cocos2d::CCMenu *)layerAEndScreen->getChildren()->objectAtIndex(i);
			if (EndLevelLayer::nameChildFind(menuSus, "cocos2d::CCMenu"))
			{
				if (menuSus->getChildrenCount() > 2)
				{
					buttonModEndLayer = menuSus;
					break;
				}
			}
		}
		auto buttonModItem = ((gd::CCMenuItemSpriteExtra *)(buttonModEndLayer->getChildren()->lastObject()));
		buttonModEndLayer->setEnabled(false);
		layerEndScreen->exitLayer(buttonModItem);
		auto resetCallButton = cocos2d::CCCallFunc::create(nullptr, (cocos2d::SEL_CallFunc)&ButtonsClass::Reset);
		gd::GameSoundManager::sharedState()->stopBackgroundMusic();
		CCDirector::sharedDirector()->getRunningScene()->runAction(cocos2d::CCSequence::create((cocos2d::CCFiniteTimeAction *)cocos2d::CCDelayTime::create(0.6f), (cocos2d::CCFiniteTimeAction *)resetCallButton, nullptr));
	}

	void Reset();

	gd::CCMenuItemSpriteExtra *buttonModItemUni = nullptr;

	void EndLayerXMod(CCObject *pSender)
	{

		auto winSize = CCDirector::sharedDirector()->getWinSize();
		auto director = CCDirector::sharedDirector();

		auto layerAEndScreen = (cocos2d::CCLayer *)this->getParent();
		auto layerEndScreen = (gd::GJDropDownLayer *)layerAEndScreen->getParent();
		cocos2d::CCMenu *buttonModEndLayer = {};

		CCPoint pointScene = {100, 320};

		for (unsigned int i = 0; i < layerAEndScreen->getChildrenCount(); i++)
		{
			auto menuSus = (cocos2d::CCMenu *)layerAEndScreen->getChildren()->objectAtIndex(i);
			if (EndLevelLayer::nameChildFind(menuSus, "cocos2d::CCMenu"))
			{
				if (menuSus->getChildrenCount() > 2)
				{
					buttonModEndLayer = menuSus;
					break;
				}
			}
		}

		auto buttonModItem = ((gd::CCMenuItemSpriteExtra *)(buttonModEndLayer->getChildren()->lastObject()));
		buttonModItemUni = buttonModItem;
		buttonModItemUni->setEnabled(true);

		if (layerAEndScreen->getPositionY() != winSize.height)
		{
			auto hideLayerCodeAnimation = cocos2d::CCEaseInOut::create((cocos2d::CCActionInterval *)cocos2d::CCMoveTo::create(0.5f, {0.f, winSize.height}), 2.f);
			layerAEndScreen->runAction(cocos2d::CCSequence::create((cocos2d::CCFiniteTimeAction *)hideLayerCodeAnimation, nullptr));
			layerEndScreen->runAction(cocos2d::CCFadeTo::create(0.5f, (GLubyte)0));

			auto animationButtonRe = cocos2d::CCEaseExponentialOut::create((cocos2d::CCActionInterval *)cocos2d::CCMoveTo::create(0.5f, {0.f, (((pointScene.y * -1.f) / 2.f) - 10.f)}));
			buttonModItem->runAction(cocos2d::CCSequence::create((cocos2d::CCFiniteTimeAction *)cocos2d::CCDelayTime::create(0.5f), cocos2d::CCRotateTo::create(0.f, 180.f), animationButtonRe, nullptr, 2.f, nullptr));
			buttonModItem->useAnimationType(gd::MenuAnimationType::kMenuAnimationTypeMove);
			buttonModItem->setDestination({0, 2.f});
			buttonModItemUni->setEnabled(true);
		}
		else
		{
			auto animationButtonRe = cocos2d::CCEaseExponentialOut::create((cocos2d::CCActionInterval *)cocos2d::CCRotateTo::create(0.3f, 0.f));
			buttonModItem->runAction(cocos2d::CCSequence::create((cocos2d::CCFiniteTimeAction *)cocos2d::CCMoveTo::create(1.f, {0.f, 128.f}), animationButtonRe, nullptr));
			buttonModItem->useAnimationType(gd::MenuAnimationType::kMenuAnimationTypeScale);
			auto showLayerCodeAnimation = cocos2d::CCEaseBounceOut::create((cocos2d::CCActionInterval *)cocos2d::CCMoveTo::create(1.f, {0.f, 5.f}));
			layerAEndScreen->runAction(cocos2d::CCSequence::create((cocos2d::CCFiniteTimeAction *)cocos2d::CCDelayTime::create(0.5f), (cocos2d::CCFiniteTimeAction *)showLayerCodeAnimation, nullptr));
			layerEndScreen->runAction(cocos2d::CCSequence::create((cocos2d::CCFiniteTimeAction *)cocos2d::CCDelayTime::create(1.f), cocos2d::CCFadeTo::create(0.5f, (GLubyte)100), nullptr, nullptr));
			buttonModItemUni->setEnabled(true);
		}
	}
};
