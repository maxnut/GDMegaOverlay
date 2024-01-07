#include "Hitbox.h"
#include "../Common.h"

#include <MinHook.h>

void __fastcall Hitbox::playLayerPostUpdateHook(cocos2d::CCLayer* self, void*, float dt)
{
	playLayerPostUpdate(self, dt);

	HitboxNode::getInstance()->clear();
	cocos2d::CCArray* array = MBO(cocos2d::CCArray*, self, 2172);

    float delta = (MBO(cocos2d::CCNode*, self, 2160))->getPositionX() - lastPos;
    lastPos = (MBO(cocos2d::CCNode*, self, 2160))->getPositionX();
    HitboxNode::getInstance()->setPositionX(HitboxNode::getInstance()->getPositionX() - delta);

	for (size_t i = 0; i < array->count(); i++)
	{
		auto obj = (cocos2d::CCNode*)array->objectAtIndex(i);
		if (obj->getParent())
		{
			auto rect =
				reinterpret_cast<cocos2d::CCRect*(__thiscall*)(cocos2d::CCNode*)>(utils::gd_base + 0x1397C0)(obj);

			HitboxNode::getInstance()->drawRectangleHitbox(*rect, {255, 0, 0, 255});
		}
	}
}

bool __fastcall Hitbox::playLayerInitHook(cocos2d::CCLayer* self, void*, void* level, bool idk1, bool idk2)
{
	bool res = playLayerInit(self, level, idk1, idk2);

    HitboxNode::getInstance()->setVisible(true);
    auto objectLayer = MBO(cocos2d::CCLayer*, self, 2476);
	objectLayer->addChild(HitboxNode::getInstance(), 9999);

	return res;
}

void Hitbox::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2E5310), playLayerPostUpdateHook,
				  reinterpret_cast<void**>(&playLayerPostUpdate));

    MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2DA660), playLayerInitHook,
				  reinterpret_cast<void**>(&playLayerInit));
}