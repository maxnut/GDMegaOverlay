#pragma once

#include <Geode/modify/GameObject.hpp>

using namespace geode::prelude;

namespace HitboxMultiplier
{
    //TODO: replace this with a custom rect field in m_fields when geode fixes getobjectrect return type
    std::unordered_map<GameObject*, cocos2d::CCRect> rectForObject;
    std::unordered_map<GameObject*, cocos2d::CCRect> memberRectForObject;

    cocos2d::CCRect* GameObjectGetObjectRect(GameObject* obj);

    void scaleHazard(GameObject* hazard);
};