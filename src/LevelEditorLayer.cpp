#include "LevelEditorLayer.h"
#include "bools.h"
#include "Hacks.h"
#include "HitboxNode.hpp"

using namespace porcodio;

extern struct HacksStr hacks;

void __fastcall LevelEditorLayer::drawHook(gd::LevelEditorLayer *self, void *)
{
	HitboxNode::getInstance()->clear();
	LevelEditorLayer::draw(self);

	if (hacks.showHitboxes)
	{
		HitboxNode::getInstance()->setVisible(true);
		if (self->m_pPlayer1)
		{
			HitboxNode::getInstance()->drawForPlayer1(self->m_pPlayer1);
		}
		if (self->m_pPlayer2)
		{
			HitboxNode::getInstance()->drawForPlayer2(self->m_pPlayer2);
		}

		if (self->m_pObjectLayer)
		{
			auto cacca = static_cast<CCLayer *>(self->getChildren()->objectAtIndex(2));
			float xp = -cacca->getPositionX() / cacca->getScale();
			for (int s = self->sectionForPos(xp) - (5 / cacca->getScale()); s < self->sectionForPos(xp) + (6 / cacca->getScale()); ++s)
			{
				if (s < 0)
					continue;
				if (s >= self->m_sectionObjects->count())
					break;
				auto section = static_cast<CCArray *>(self->m_sectionObjects->objectAtIndex(s));
				for (size_t i = 0; i < section->count(); ++i)
				{
					auto obj = static_cast<gd::GameObject *>(section->objectAtIndex(i));

					if (hacks.hitboxOnly)
						obj->setOpacity(0);
					if (obj->m_nObjectID != 749 && obj->getObjType() == gd::GameObjectType::kGameObjectTypeDecoration)
						continue;
					if (!obj->m_bActive)
						continue;

					HitboxNode::getInstance()->drawForObject(obj);
				}
			}
		}
	}
}