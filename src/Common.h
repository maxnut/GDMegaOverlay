#pragma once

#include <string>
#include <vector>
#include "utils.hpp"

namespace cocos2d
{
    class CCLayer;
}

namespace Common
{
    inline void* gameManager = reinterpret_cast<void*(__stdcall*)()>(utils::gd_base + 0x11f720)();
    inline void* FMODAudioEngine = reinterpret_cast<void*(__stdcall*)()>(utils::gd_base + 0x32b80)();

    cocos2d::CCLayer* getBGL();

    inline const char*(__thiscall* splashString)();
    const char* __fastcall splashStringHook();

    void initHooks();
    void calculateFramerate();
    void setPriority();
    void onAudioSpeedChange();
    void onAudioPitchChange();
};
