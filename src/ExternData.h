#pragma once
#include "DiscordManager.h"
#include "bools.h"
#include "json.hpp"
#include "pch.h"

using json = nlohmann::json;

namespace ExternData
{
    extern float screenSizeX, screenSizeY;

    extern std::vector<std::string> musicPaths;
    extern std::filesystem::path path;

    extern std::vector<std::function<void()>> imguiFuncs, openFuncs, closeFuncs;

    extern DiscordManager ds;

    extern json bypass, creator, global, level, player, variables, dlls;

    extern int amountOfClicks, amountOfReleases, amountOfMediumClicks, steps;

    extern float tps, screenFps;

    extern bool show, fake, isCheating, holdingAdvance;
    extern char searchbar[30], replayName[30];
    extern std::string hackName;

    extern bool resetWindows, repositionWindows;

    extern json windowPositions;

    extern std::map<std::string, nlohmann::json> settingFiles;
    extern std::vector<std::string> dllNames;

    extern bool hasSaiModPack, animationDone;
    extern CCAction* animationAction;
    extern float animation;

    extern int randomDirection;
} // namespace ExternData