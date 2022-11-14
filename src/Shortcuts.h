#pragma once
#include "pch.h"

static const char *hackNames[] = {"Noclip", "Freeze Player", "Jump Hack", "Hide Attempts", "Practice Music", "No Pulse", "Ignore Esc", "Accurate Percentage", "Respawn Blink", "Death Effect", "Coins In Practice", "Instant Complete", "Solid Wave Trail", "No Particles", "Same Dual Color", "No Transitions", "Slider Limit", "Text Length", "Filter Bypass"
, "No Glow", "Fast AltTab", "Safe Mode", "Practice Pulse", "No Progress Bar", "Free Resize", "Copy Hack", "Level Edit Hack", "No C Mark", "Object Limit Bypass", "Zoom Hack", "Verify Hack", "Default Song Bypass", "Editor Extension", "Place Over", "Hide Editor UI"
, "Show Hitboxes", "Hitbox Only on death", "Hitbox show decoration", "Custom Object Limit", "StartPos Switcher", "Transparent Lists", "Transparent Menus" , "Dash Orb Color Fix", "Replay From Last Checkpoint", "Autoclicker", "Frame Step", "Show Replay Label", "Auto Deafen", "Practice Fix", "From percent only in runs", "Hitboxes only", "Rainbow Icons", "Hide Testmode", "Layout Mode", "Clickbot", "Confirm Quit", "Auto Sync Music", "Hide Pause", "Only Rainbow Glow", "Respawn", "Cheat Indicator", "FPS Counter", "CPS Counter", "Noclip Accuracy", "Noclip Deaths", "Clock", "Best Run", "Attempts", "From%", "Message Status", "Current Attempt"};

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