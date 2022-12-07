#include "ExitAlert.h"
#include "PlayLayer.h"

void ExitAlert::FLAlert_Clicked(gd::FLAlertLayer *layer, bool btn2)
{
    if(btn2)
    {
        PlayLayer::Quit();
    }
    else if(gd::GameManager::sharedState()->getPlayLayer()->m_isDead)
    {
        PlayLayer::resetLevelHook(gd::GameManager::sharedState()->getPlayLayer(), 0);
    }
}