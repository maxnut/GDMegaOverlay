#pragma once
#include "DiscordManager.h"
#include "bools.h"
#include "json.hpp"
#include "pch.h"

using json = nlohmann::json;

namespace ExternData
{
    extern float screenSize, oldScreenSize;

    extern std::vector<std::string> musicPaths;
    extern std::filesystem::path path;

    extern std::vector<std::function<void()>> imguiFuncs, openFuncs, closeFuncs;

    extern DiscordManager ds;

    extern json bypass, creator, global, level, player, variables;

    extern int amountOfClicks, amountOfReleases, amountOfMediumClicks, steps;

    extern float tps, screenFps;

    extern bool show, fake, isCheating, holdingAdvance;
    extern char searchbar[30];
    extern std::string hackName;

    extern bool resetWindows, repositionWindows, saveWindows;

    extern Windows windowPositions;

    extern std::map<std::string, nlohmann::json> settingFiles;
} // namespace ExternData