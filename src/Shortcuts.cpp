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
    debug.debugNumber = index;
    switch (index)
    {
    case 0:
        hacks.noclip = !hacks.noclip;
        Hacks::noclip(hacks.noclip);
        Hacks::SaveSettings();
        break;
    case 1:
        hacks.freezeplayer = !hacks.freezeplayer;
        Hacks::freezeplayer(hacks.freezeplayer);
        Hacks::SaveSettings();
        break;
    case 2:
        hacks.jumphack = !hacks.jumphack;
        Hacks::jumphack(hacks.jumphack);
        Hacks::SaveSettings();
        break;
    case 3:
        hacks.hideattempts = !hacks.hideattempts;
        Hacks::SaveSettings();
        break;
    case 4:
        hacks.practicemusic = !hacks.practicemusic;
        Hacks::practicemusic(hacks.practicemusic);
        Hacks::SaveSettings();
        break;
    case 5:
        hacks.nopulse = !hacks.nopulse;
        Hacks::nopulse(hacks.nopulse);
        Hacks::SaveSettings();
        break;
    case 6:
        hacks.ignoreesc = !hacks.ignoreesc;
        Hacks::IgnoreESC(hacks.ignoreesc);
        Hacks::SaveSettings();
        break;
    case 7:
        hacks.accuratepercentage = !hacks.accuratepercentage;
        Hacks::accuratepercentage(hacks.accuratepercentage);
        Hacks::SaveSettings();
        break;
    case 8:
        hacks.respawnBlink = !hacks.respawnBlink;
        Hacks::disablerespawnblink(hacks.respawnBlink);
        Hacks::SaveSettings();
        break;
    case 9:
        hacks.deathEffect = !hacks.deathEffect;
        Hacks::disabledeatheffect(hacks.deathEffect);
        Hacks::SaveSettings();
        break;
    case 10:
        hacks.coinsPractice = !hacks.coinsPractice;
        Hacks::coinsonpractice(hacks.coinsPractice);
        Hacks::SaveSettings();
        break;
    case 11:
        hacks.instantcomplete = !hacks.instantcomplete;
        Hacks::InstantComplete(hacks.instantcomplete);
        Hacks::SaveSettings();
        break;
    case 12:
        hacks.solidwavetrail = !hacks.solidwavetrail;
        Hacks::solidwavetrail(hacks.solidwavetrail);
        Hacks::SaveSettings();
        break;
    case 13:
        hacks.noParticles = !hacks.noParticles;
        Hacks::noparticles(hacks.noParticles);
        Hacks::SaveSettings();
        break;
    case 14:
        hacks.sameDualColor = !hacks.sameDualColor;
        Hacks::samedualcolor(hacks.sameDualColor);
        Hacks::SaveSettings();
        break;
    case 15:
        hacks.notransition = !hacks.notransition;
        Hacks::noTransition(hacks.notransition);
        Hacks::SaveSettings();
        break;
    case 16:
        hacks.sliderLimit = !hacks.sliderLimit;
        Hacks::sliderLimit(hacks.sliderLimit);
        Hacks::SaveSettings();
        break;
    case 17:
        hacks.textLength = !hacks.textLength;
        Hacks::TextLength(hacks.textLength);
        Hacks::SaveSettings();
        break;
    case 18:
        hacks.filterBypass = !hacks.filterBypass;
        Hacks::FilterBypass(hacks.filterBypass);
        Hacks::SaveSettings();
        break;
    case 19:
        hacks.noglow = !hacks.noglow;
        Hacks::noGlow(hacks.noglow);
        Hacks::SaveSettings();
        break;
    case 20:
        hacks.fastalttab = !hacks.fastalttab;
        Hacks::fastalttab(hacks.fastalttab);
        Hacks::SaveSettings();
        break;
    case 21:
        hacks.safemode = !hacks.safemode;
        Hacks::safemode(hacks.safemode);
        Hacks::SaveSettings();
        break;
    case 22:
        hacks.practicepulse = !hacks.practicepulse;
        Hacks::practicepulse(hacks.practicepulse);
        Hacks::SaveSettings();
        break;
    case 23:
        hacks.noprogressbar = !hacks.noprogressbar;
        Hacks::freezeplayer(hacks.freezeplayer);
        Hacks::SaveSettings();
        break;
    case 24:
        hacks.freeResize = !hacks.freeResize;
        Hacks::freeResize(hacks.freeResize);
        Hacks::SaveSettings();
        break;
    case 25:
        hacks.copyhack = !hacks.copyhack;
        Hacks::copyhack(hacks.copyhack);
        Hacks::SaveSettings();
        break;
    case 26:
        hacks.leveledit = !hacks.leveledit;
        Hacks::leveledit(hacks.leveledit);
        Hacks::SaveSettings();
        break;
    case 27:
        hacks.nocmark = !hacks.nocmark;
        Hacks::nocmark(hacks.nocmark);
        Hacks::SaveSettings();
        break;
    case 28:
        hacks.objectlimit = !hacks.objectlimit;
        Hacks::objectlimit(hacks.objectlimit);
        Hacks::SaveSettings();
        break;
    case 29:
        hacks.zoomhack = !hacks.zoomhack;
        Hacks::zoomhack(hacks.zoomhack);
        Hacks::SaveSettings();
        break;
    case 30:
        hacks.verifyhack = !hacks.verifyhack;
        Hacks::verifyhack(hacks.verifyhack);
        Hacks::SaveSettings();
        break;
    case 31:
        hacks.defaultsong = !hacks.defaultsong;
        Hacks::defaultsong(hacks.defaultsong);
        Hacks::SaveSettings();
        break;
    case 32:
        hacks.extension = !hacks.extension;
        Hacks::editorextension(hacks.extension);
        Hacks::SaveSettings();
        break;
    case 33:
        hacks.placeover = !hacks.placeover;
        Hacks::placeover(hacks.placeover);
        Hacks::SaveSettings();
        break;
    case 34:
        hacks.hideui = !hacks.hideui;
        Hacks::hideui(hacks.hideui);
        Hacks::SaveSettings();
        break;
    case 35:
        hacks.showHitboxes = !hacks.showHitboxes;
        Hacks::SaveSettings();
        break;
    case 36:
        hacks.onlyOnDeath = !hacks.onlyOnDeath;
        Hacks::SaveSettings();
        break;
    case 37:
        hacks.showDecorations = !hacks.showDecorations;
        Hacks::SaveSettings();
        break;
    case 38:
        hacks.objectHack = !hacks.objectHack;
        Hacks::SaveSettings();
        break;
    case 39:
        hacks.startPosSwitcher = !hacks.startPosSwitcher;
        Hacks::SaveSettings();
        break;
    case 40:
        hacks.transparentList = !hacks.transparentList;
        Hacks::transparentLists(hacks.transparentList);
        Hacks::SaveSettings();
        break;
    case 41:
        hacks.transparentMenus = !hacks.transparentMenus;
        Hacks::transparentMenus(hacks.transparentMenus);
        Hacks::SaveSettings();
        break;
    case 42:
        hacks.dashOrbFix = !hacks.dashOrbFix;
        Hacks::SaveSettings();
        break;
    case 43:
        hacks.lastCheckpoint = !hacks.lastCheckpoint;
        Hacks::SaveSettings();
        break;
    case 44:
        hacks.autoclicker = !hacks.autoclicker;
        Hacks::SaveSettings();
        break;
    case 45:
        hacks.frameStep = !hacks.frameStep;
        Hacks::SaveSettings();
        break;
    case 46:
        hacks.botTextEnabled = !hacks.botTextEnabled;
        Hacks::SaveSettings();
        break;
    case 47:
        hacks.autoDeafen = !hacks.autoDeafen;
        Hacks::SaveSettings();
        break;
    case 48:
        hacks.fixPractice = !hacks.fixPractice;
        Hacks::SaveSettings();
        break;
    case 49:
        hacks.onlyInRuns = !hacks.onlyInRuns;
        Hacks::SaveSettings();
        break;
    case 50:
        hacks.hitboxOnly = !hacks.hitboxOnly;
        Hacks::SaveSettings();
        break;
    case 51:
        hacks.rainbowIcons = !hacks.rainbowIcons;
        Hacks::SaveSettings();
        break;
    case 52:
        hacks.hideTestmode = !hacks.hideTestmode;
        Hacks::SaveSettings();
        break;
    case 53:
        hacks.layoutMode = !hacks.layoutMode;
        Hacks::SaveSettings();
        break;
    case 54:
        hacks.clickbot = !hacks.clickbot;
        Hacks::SaveSettings();
        break;
    case 55:
        hacks.confirmQuit = !hacks.confirmQuit;
        Hacks::SaveSettings();
        break;
    case 56:
        hacks.autoSyncMusic = !hacks.autoSyncMusic;
        Hacks::SaveSettings();
        break;
    case 57:
        hacks.hidePause = !hacks.hidePause;
        Hacks::SaveSettings();
        break;
    case 58:
        hacks.onlyRainbowOutline = !hacks.onlyRainbowOutline;
        Hacks::SaveSettings();
        break;
    case 59:
        PlayLayer::resetLevelHook(gd::GameManager::sharedState()->getPlayLayer(), 0);
        Hacks::SaveSettings();
        break;
    case 60:
        labels.statuses[0] = !labels.statuses[0];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 61:
        labels.statuses[1] = !labels.statuses[1];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 62:
        labels.statuses[2] = !labels.statuses[2];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 63:
        labels.statuses[3] = !labels.statuses[3];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 64:
        labels.statuses[4] = !labels.statuses[4];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 65:
        labels.statuses[5] = !labels.statuses[5];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 66:
        labels.statuses[6] = !labels.statuses[6];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 67:
        labels.statuses[7] = !labels.statuses[7];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 68:
        labels.statuses[8] = !labels.statuses[8];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 69:
        labels.statuses[9] = !labels.statuses[9];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    case 70:
        labels.statuses[10] = !labels.statuses[10];
        for (size_t i = 0; i < 11; i++)
            PlayLayer::UpdatePositions(i);
        Hacks::SaveSettings();
        break;
    }
}