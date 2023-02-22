#include "LevelEditorLayer.h"
#include "bools.h"
#include "Hacks.h"
#include "HitboxNode.hpp"


extern struct HacksStr hacks;

void __fastcall LevelEditorLayer::drawHook(gd::LevelEditorLayer *self, void *)
{
	HitboxNode::getInstance()->clear();
	LevelEditorLayer::draw(self);

	if (hacks.showHitboxes && !hacks.onlyOnDeath)
	{
		HitboxNode::getInstance()->setVisible(true);
		if (self->m_pPlayer1)
		{
			HitboxNode::getInstance()->addToPlayer1Queue(self->m_pPlayer1->getObjectRect());
			HitboxNode::getInstance()->drawForPlayer1(self->m_pPlayer1);
		}
		if (self->m_pPlayer2)
		{
			HitboxNode::getInstance()->addToPlayer2Queue(self->m_pPlayer2->getObjectRect());
			HitboxNode::getInstance()->drawForPlayer2(self->m_pPlayer2);
		}

		if (self->m_pObjectLayer)
		{
			auto layer = static_cast<CCLayer *>(self->getChildren()->objectAtIndex(2));
			float xp = -layer->getPositionX() / layer->getScale();
			for (int s = self->sectionForPos(xp) - (5 / layer->getScale()); s < self->sectionForPos(xp) + (6 / layer->getScale()); ++s)
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

void __fastcall LevelEditorLayer::onPlaytestHook(gd::LevelEditorLayer* self, void*)
{
	HitboxNode::getInstance()->clearQueue();
	LevelEditorLayer::onPlaytest(self);
}

void __fastcall LevelEditorLayer::exitHook(CCLayer *self, void *, CCObject* sender)
{
	LevelEditorLayer::exit(self, sender);
	Hacks::MenuMusic();
	HitboxNode::getInstance()->clearQueue();
	Hacks::UpdateRichPresence(2);
}

void __fastcall LevelEditorLayer::fadeMusicHook(gd::GameManager* self, void*, char* idk)
{
	if(!hacks.replaceMenuMusic) LevelEditorLayer::fadeMusic(self, idk);
}