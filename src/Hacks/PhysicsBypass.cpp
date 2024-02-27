#include "PhysicsBypass.h"
#include "Settings.hpp"
#include "Macrobot/Macrobot.h"
#include <Geode/modify/PlayLayer.hpp>
#include "Common.h"

class $modify(PlayLayer)
{
    bool init(GJGameLevel* p0, bool p1, bool p2)
    {
        PhysicsBypass::timestamp = p0->m_timestamp;

        return PlayLayer::init(p0, p1, p2);
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();
        PhysicsBypass::calculateTickrate();
    }
};

void PhysicsBypass::calculateTickrate()
{
    float tps = Common::getTPS();
    util::Write<float>(base::get() + 0x49D548, 1.f / tps);
    
    /* if(tps == 240.f)
        return; */
    
    if(GameManager::get()->getPlayLayer() && GameManager::get()->getPlayLayer()->m_level->m_timestamp > 0)
    {
        float timestampMultiplier = (tps / 240.f);
        float stepsMultiplier = (timestamp * timestampMultiplier) / GameManager::get()->getPlayLayer()->m_level->m_timestamp;
        GameManager::get()->getPlayLayer()->m_level->m_timestamp = timestamp * timestampMultiplier;
        GameManager::get()->getPlayLayer()->m_gameState.m_unk1f8 *= stepsMultiplier;
    }
}