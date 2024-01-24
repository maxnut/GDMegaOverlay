#pragma once

#include <string>
#include <vector>
#include "util.hpp"

namespace cocos2d
{
    class CCLayer;
}

namespace Common
{
    void calculateFramerate();
    void setPriority();
    void onAudioSpeedChange();
    void onAudioPitchChange();
    void saveIcons();
    void loadIcons();
};
