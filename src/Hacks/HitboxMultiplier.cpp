#include "HitboxMultiplier.h"

#include "Settings.hpp"

void scaleRect(cocos2d::CCRect* r, float a, float b)
{
	r->origin.x += r->size.width / 2;
	r->origin.y += r->size.height / 2;
	r->size.width *= a;
	r->size.height *= b;
	r->origin.x -= r->size.width / 2;
	r->origin.y -= r->size.height / 2;
}

cocos2d::CCRect* HitboxMultiplier::GameObjectGetObjectRect(GameObject* obj)
{
    bool enabled = Settings::get<bool>("level/hitbox_multiplier", false);

    if(!enabled)
        return reinterpret_cast<cocos2d::CCRect*(__thiscall*)(GameObject*)>(base::get() + 0x13a570)(obj);

    bool all = Settings::get<bool>("level/hitbox_multiplier/scale_all", false);

    float x = 1, y = 1;
    
    float scaleSlopes[2] = {Settings::get<float>("level/hitbox_multiplier/scale_slopes/x", 1.f), Settings::get<float>("level/hitbox_multiplier/scale_slopes/y", 1.f)};
	float scaleHazards[2] = {Settings::get<float>("level/hitbox_multiplier/scale_hazards/x", 1.f), Settings::get<float>("level/hitbox_multiplier/scale_hazards/y", 1.f)};
	float scalePlayer[2] = {Settings::get<float>("level/hitbox_multiplier/scale_player/x", 1.f), Settings::get<float>("level/hitbox_multiplier/scale_player/y", 1.f)};

    if(obj == GameManager::get()->getGameLayer()->m_player1 || obj == GameManager::get()->getGameLayer()->m_player2)
    {
        x = scalePlayer[0];
        y = scalePlayer[1];
    }
    else
    {
        switch(obj->m_objectType)
        {
            default:
                return reinterpret_cast<cocos2d::CCRect*(__thiscall*)(GameObject*)>(base::get() + 0x13a570)(obj);
            case GameObjectType::Hazard:
            case GameObjectType::AnimatedHazard:
                x = scaleHazards[0];
                y = scaleHazards[1];
                break;
            case GameObjectType::Slope:
                x = scaleSlopes[0];
                y = scaleSlopes[1];
                break;
        }
    }

    cocos2d::CCRect rect = *reinterpret_cast<cocos2d::CCRect*(__thiscall*)(GameObject*)>(base::get() + 0x13a570)(obj);
    scaleRect(&rect, x, y);
    rectForObject[obj] = rect;
    return &rectForObject[obj];
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void *>(base::get() + 0x13a570), &HitboxMultiplier::GameObjectGetObjectRect, "GameObject::getObjectRect", tulip::hook::TulipConvention::Thiscall);
}