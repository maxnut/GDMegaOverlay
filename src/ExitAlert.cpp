#include "ExitAlert.h"
#include "PlayLayer.h"

void ExitAlert::FLAlert_Clicked(gd::FLAlertLayer *layer, bool btn2)
{
    if(btn2)
    {
        PlayLayer::Quit();
    }
}