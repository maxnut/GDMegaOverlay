#include "Shortcuts.h"
#include "bools.h"
#include "Hacks.h"
#include "PlayLayer.h"

extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

std::vector<Shortcuts::Shortcut> Shortcuts::shortcuts;

void Shortcuts::Save()
{
    if (!std::filesystem::is_directory("GDMenu") || !std::filesystem::exists("GDMenu"))
        std::filesystem::create_directory("GDMenu");

    std::ofstream file("GDMenu/shortcuts.bin", std::ios::out | std::ios::binary);
    for (const auto &s : shortcuts)
    {
        file.write((char *)&s.key, sizeof(int));
        file.write((char *)&s.shortcutIndex, sizeof(int));
    }
    file.close();
}

void Shortcuts::Load()
{
    if(shortcuts.size() > 0) return;
    std::ifstream file("GDMenu/shortcuts.bin", std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        return;
    }
    file.seekg(0, std::ios::end);
    size_t left = static_cast<size_t>(file.tellg());
    file.seekg(0);
    for (size_t _ = 0; _ < left / sizeof(Shortcut); ++_)
    {
        Shortcut s;
        file.read((char *)&s.key, sizeof(int));
        file.read((char *)&s.shortcutIndex, sizeof(int));
        shortcuts.push_back(s);
    }
    file.close();
}

void Shortcuts::OnPress(int index)
{
    if (index < hackAmts[0])
    {
        Hacks::ToggleJSONHack(Hacks::bypass, index, true);
        Hacks::SaveSettings();
        return;
    }
    else if (index < hackAmts[0] + hackAmts[1])
    {
        Hacks::ToggleJSONHack(Hacks::creator, index - hackAmts[0], true);
        Hacks::SaveSettings();
        return;
    }
    else if (index < hackAmts[0] + hackAmts[1] + hackAmts[2])
    {
        Hacks::ToggleJSONHack(Hacks::global, index - hackAmts[0] - hackAmts[1], true);
        Hacks::SaveSettings();
        return;
    }
    else if (index < hackAmts[0] + hackAmts[1] + hackAmts[2] + hackAmts[3])
    {
        Hacks::ToggleJSONHack(Hacks::level, index - hackAmts[0] - hackAmts[1] - hackAmts[2], true);
        Hacks::SaveSettings();
        return;
    }
    else if (index < hackAmts[0] + hackAmts[1] + hackAmts[2] + hackAmts[3] + hackAmts[4])
    {
        Hacks::ToggleJSONHack(Hacks::player, index - hackAmts[0] - hackAmts[1] - hackAmts[2] - hackAmts[3], true);
        Hacks::SaveSettings();
        return;
    }

    int subIndex = index - (hackAmts[0] + hackAmts[1] + hackAmts[2] + hackAmts[3] + hackAmts[4]) + 1;

    switch (subIndex)
    {
        case 1:
            hacks.autoclicker = !hacks.autoclicker;
            break;
        case 2:
            hacks.frameStep = !hacks.frameStep;
            break;
        case 3:
            hacks.autoDeafen = !hacks.autoDeafen;
            break;
        case 4:
            hacks.hidePause = !hacks.hidePause;
            break;
        case 5:
            hacks.rainbowIcons = !hacks.rainbowIcons;
            break;
        case 6:
            hacks.startPosSwitcher = !hacks.startPosSwitcher;
            break;
        case 7:
            hacks.smartStartPos = !hacks.smartStartPos;
            break;
        case 8:
            hacks.showHitboxes = !hacks.showHitboxes;
            break;
        case 9:
            hacks.onlyOnDeath = !hacks.onlyOnDeath;
            break;
        case 10:
            hacks.showDecorations = !hacks.showDecorations;
            break;
        case 11:
            hacks.hitboxTrail = !hacks.hitboxTrail;
            break;
        case 12:
            hacks.layoutMode = !hacks.layoutMode;
            break;
        case 13:
            hacks.lastCheckpoint = !hacks.lastCheckpoint;
            break;
        case 14:
            hacks.autoSyncMusic = !hacks.autoSyncMusic;
            break;
        case 15:
            labels.statuses[0] = !labels.statuses[0];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 16:
            labels.statuses[1] = !labels.statuses[1];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 17:
            labels.statuses[2] = !labels.statuses[2];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 18:
            labels.statuses[3] = !labels.statuses[3];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 19:
            labels.statuses[4] = !labels.statuses[4];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 20:
            labels.statuses[5] = !labels.statuses[5];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 21:
            labels.statuses[6] = !labels.statuses[6];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 22:
            labels.statuses[7] = !labels.statuses[7];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 23:
            labels.statuses[8] = !labels.statuses[8];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 24:
            labels.statuses[9] = !labels.statuses[9];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 25:
            labels.statuses[10] = !labels.statuses[10];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 26:
            labels.statuses[11] = !labels.statuses[11];
            for(size_t i = 0; i < STATUSSIZE; i++) PlayLayer::UpdatePositions(i);
            break;
        case 27:
            auto pl = gd::GameManager::sharedState()->getPlayLayer();
            if(pl) 
            {
                PlayLayer::resetLevelHook(pl, 0);
                if (pl->m_bIsPaused)
		            gd::GameSoundManager::sharedState()->stopBackgroundMusic();
            }
            break;
    }

    Hacks::SaveSettings();
}