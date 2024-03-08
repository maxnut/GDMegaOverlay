#pragma once

#include <Geode/modify/GameObject.hpp>

using namespace geode::prelude;

namespace HitboxMultiplier
{
	std::unordered_map<GameObject*, cocos2d::CCRect> memberRectForObject;

	void scaleHazard(GameObject* hazard);
};