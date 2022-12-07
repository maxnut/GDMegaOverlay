#pragma once
#include "pch.h"

static const char *manualHackNames[] = {"Autoclicker", "Frame Step", "Auto Deafen", "Hide Pause", "Rainbow Icons", "StartPos Switcher", "Smart StartPos", "Show Hitboxes", "Hitbox Only On Death", "Hitbox Show Deco", "Hitbox Trail", "Layout Mode", "Replay Last Checkpoint", "Auto Sync Music", "Cheat Indicator", "FPS Counter", "CPS Counter", "NoClip Accuracy", "NoClip Deaths", "Clock", "Best Run", "Attempts", "From%", "Current Attempt", "LevelID", "Jumps", "Respawn"};

static std::vector<std::string> hackNamesString;
static std::vector<const char *> hackNames;

extern int hackAmts[];

class Shortcuts
{

public:
    struct Shortcut
    {
        int key, shortcutIndex;
    };
    static std::vector<Shortcut> shortcuts;

    static void OnPress(int index);
    static void Save();
    static void Load();
};