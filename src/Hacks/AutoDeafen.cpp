#include "AutoDeafen.h"
#include "../Settings.hpp"
#include <WinUser.h>

#include "ConstData.h"

#include <Geode/modify/PlayLayer.hpp>
using namespace geode::prelude;

void AutoDeafen::toggleDeafen()
{
    int muteKey = Settings::get<int>("level/auto_deafen/mute_key", 0);
    muteKey = ConvertImGuiKeyToEnum((ImGuiKey)muteKey);
	keybd_event(VK_MENU, 0x38, 0, 0);
	keybd_event(muteKey, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event(muteKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
}

class $modify(PlayLayer)
{
    void postUpdate(float dt)
    {
        PlayLayer::postUpdate(dt);

        bool enabled = Settings::get<bool>("level/auto_deafen/enabled", false);

        if(!enabled)
            return;

        float deafenPercent = Settings::get<float>("level/auto_deafen/percent", 50.f);

        float percent = PlayLayer::getCurrentPercent();

        if(!this->m_player1->m_isDead && !this->m_player2->m_isDead && percent > deafenPercent && !AutoDeafen::deafened)
        {
            AutoDeafen::toggleDeafen();
            AutoDeafen::deafened = true;
        }
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1)
    {
        PlayLayer::destroyPlayer(p0, p1);

        if(p0 && p0->m_isDead && AutoDeafen::deafened)
        {
            AutoDeafen::toggleDeafen();
            AutoDeafen::deafened = false;
        }
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();

        if(AutoDeafen::deafened)
        {
            AutoDeafen::toggleDeafen();
            AutoDeafen::deafened = false;
        }
    }

    void levelComplete()
    {
        PlayLayer::levelComplete();

        if(AutoDeafen::deafened)
        {
            AutoDeafen::toggleDeafen();
            AutoDeafen::deafened = false;
        }
    }

    void onQuit()
    {
        PlayLayer::onQuit();

        if(AutoDeafen::deafened)
        {
            AutoDeafen::toggleDeafen();
            AutoDeafen::deafened = false;
        }
    }
};