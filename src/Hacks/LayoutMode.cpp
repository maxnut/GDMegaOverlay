#include "../Common.h"

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EffectGameObject.hpp>
#include <Geode/modify/ShaderLayer.hpp>

#include <Geode/modify/GameObject.hpp>

#include "Settings.hpp"

using namespace geode::prelude;

void hideObject(GameObject* obj)
{
	if ((obj->getParent() && !obj->isVisible()) || obj->m_objectID == 44 || obj->m_objectID == 749 || obj->m_objectID == 12 || obj->m_objectID == 38 ||
					 obj->m_objectID == 47 || obj->m_objectID == 111 || obj->m_objectID == 8 || obj->m_objectID == 13 ||
					 obj->m_objectID == 660 || obj->m_objectID == 745 || obj->m_objectID == 101 || obj->m_objectID == 99 ||
					 obj->m_objectID == 1331)
		return;

	//setupLayers
	CCSpriteBatchNode* t4 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 1640);
	CCSpriteBatchNode* t3 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 1696);
	CCSpriteBatchNode* t2 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 1752);
	CCSpriteBatchNode* t1 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 1808);
	CCSpriteBatchNode* b1 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 1888);
	CCSpriteBatchNode* b2 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 1944);
	CCSpriteBatchNode* b3 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 2000);
	CCSpriteBatchNode* b4 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 2056);
	CCSpriteBatchNode* b5 = MBO(CCSpriteBatchNode*, PlayLayer::get(), 2092);

	switch(obj->m_objectType)
	{
		case GameObjectType::Decoration:
			obj->setVisible(false);
			break;
		case GameObjectType::Solid:
		case GameObjectType::Hazard:
		case GameObjectType::AnimatedHazard:
		case GameObjectType::Slope:
			obj->setOpacity(255);
			obj->setVisible(true);

			if (!obj->getParent())
			{
				switch((int)obj->m_zLayer)
				{
					case -5:
						b5->addChild(obj);
						break;
					case -3:
						b4->addChild(obj);
						break;
					case -1:
						b3->addChild(obj);
						break;
					case 1:
						b2->addChild(obj);
						break;
					default:
					case 3:
						b1->addChild(obj);
						break;
					case 5:
						t1->addChild(obj);
						break;
					case 7:
						t2->addChild(obj);
						break;
					case 9:
						t3->addChild(obj);
						break;
					case 11:
						t4->addChild(obj);
						break;
				}
			}
			break;
	}
}

class $modify(PlayLayer)
{
	void postUpdate(float dt)
	{
		PlayLayer::postUpdate(dt);

		if (!Settings::get<bool>("level/layout_mode", false))
			return;

		//found in gjbasegamelayer::createbackground

		CCSprite* bg = MBO(CCSprite*, this, 2500);
		bg->setColor({40, 62, 255});

		//found in gjbasegamelayer::createmiddleground

		GJMGLayer* mg = MBO(GJMGLayer*, this, 2516);

		//idk how to set color of this womp womp

		//found in gjbasegamelayer::creategroundlayer

		GJGroundLayer* gl1 = MBO(GJGroundLayer*, this, 2508);
		gl1->updateGround01Color({40, 62, 255});
		gl1->updateGround02Color({40, 62, 255});

		GJGroundLayer* gl2 = MBO(GJGroundLayer*, this, 2512);
		gl2->updateGround01Color({40, 62, 255});
		gl2->updateGround02Color({40, 62, 255});
	}
};

class $modify(ShaderLayer)
{
	void visit()
	{
		bool enabled = Settings::get<bool>("level/layout_mode", false);
		if (enabled)
		{
			CCNode::visit();
			return;
		}

		ShaderLayer::visit();
	}
};

class $modify(EffectGameObject)
{
	void triggerObject(GJBaseGameLayer* p0, int p1, gd::vector<int> const* p2)
	{
		if (!Settings::get<bool>("level/layout_mode", false))
			return EffectGameObject::triggerObject(p0, p1, p2);

		int id = this->m_objectID;

		if (id == 899 || id == 1006 || id == 1007 || id == 105 || id == 29 || id == 56 || id == 915 || id == 30 || id == 58)
			return;

		EffectGameObject::triggerObject(p0, p1, p2);
	}
};

void lightningFlash(GJBaseGameLayer* self, cocos2d::CCPoint* a, cocos2d::CCPoint* b, cocos2d::ccColor3B* c, float d, float e, int f, bool g, float h)
{
	if (Settings::get<bool>("level/layout_mode", false))
		return;
	
	reinterpret_cast<void(__thiscall *)(GJBaseGameLayer *, cocos2d::CCPoint*, cocos2d::CCPoint*, cocos2d::ccColor3B*, float, float, int, bool, float)>(base::get() + 0x1C8D50)(self, a, b, c, d, e, f, g, h);
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void *>(base::get() + 0x1C8D50), &lightningFlash, "GJBaseGameLayer::lightningFlash", tulip::hook::TulipConvention::Thiscall);
	Common::sectionLoopFunctions.push_back({hideObject, "level/layout_mode"});
}