#pragma once

struct HacksStr
{

    bool start = false;

    float fps = 360;
    bool noclip, freezeplayer, jumphack, hideattempts, practicemusic, nopulse, ignoreesc, accuratepercentage, respawnBlink, deathEffect, coinsPractice, instantcomplete;
    int hitboxType, trailType;
    bool solidwavetrail, noFlash, noParticles, sameDualColor;
    bool notransition, icons, sliderLimit, mainLevels, textLength, filterBypass;
    bool noglow, fastalttab, safemode, practicepulse, noprogressbar;
    float percentage = 0;
    bool freeResize;
    bool copyhack, leveledit, nocmark, objectlimit, zoomhack, verifyhack, zoom, defaultsong, extension, placeover, hideui;
    float noClipAccuracyLimit;

    float speed = 1;
    bool showHitboxes, onlyOnDeath, showDecorations, objectHack;

    int iconIds[12];

    bool startPosSwitcher;
    char mute = 'A';
    int curChar;
    float waveSize;
    bool platformer, transparentList, transparentMenus, dashOrbFix;
    float respawnTime = 1.2f;
    int respawnIndex;
    bool lastCheckpoint, autoclicker, frameStep;
    float clickTime, releaseTime;
    int stepCount, stepIndex;
    bool botTextEnabled;
    int actionStart;
    float hitboxThickness = 0.5f, menuSize = 1;
    float titleColor[4] = {0.75f, 0.157f, 0.157f, 1.0f}, borderColor[4] = {0, 0, 0, 1.0f}, borderSize = 0, windowRounding;
    bool autoDeafen, fixPractice, onlyInRuns, hitboxOnly;
    char message[30];
    bool rainbowIcons, hideTestmode, layoutMode;
    float rainbowSpeed;
    int messageIndex;
    bool clickbot = false;
    float baseVolume = 1.0f, minPitch = 0.97f, maxPitch = 1.03f, minTimeDifference = 0.03f;
    double playMediumClicksAt = 0.07;
    bool confirmQuit, autoSyncMusic, hidePause = false;
    int musicMaxDesync = 80, hitboxOpacity = 200, borderOpacity = 255, hitboxKeyIndex;
    float clickColor[3] = {1.0f, 0.0f, 0.0f};
    bool onlyRainbowOutline = false, replaceMenuMusic = false, randomMusic = false;
    int musicIndex, randomMusicIndex;

    bool recording = false;
    int videoDimenstions[2] = {1280, 720};
    int videoFps = 60;
    float renderMusicVolume = 0.45f, renderClickVolume = 0.74f;
    bool includeClicks;
    char bitrate[8] = "30M";
    int clickSoundChunkSize = 20;
    float afterEndDuration = 1.0f;

    bool solidWavePulse = false, hitboxTrail = false;
    float hitboxTrailLength = 50.0f, hitboxMultiplier, noclipRedRate = 1.0f, noclipRedLimit = 100.0f, noclipRedRateDown = 1.0f;
    bool noclipRed = false;
    float noclipColor[3] = {1.0f, 0.0f, 0.0f};
    bool noMirrorPortal = false, zorder = false, guardVault = false, keymasterVault = false, keymasterBasement = false, basementBypass = false, challengeBypass = false, treasureRoom = false, potbarShop = false, scratchShop = false, gatekeeperVault = false, backupStar = false
    , showExtraInfo = true;
};

enum position {TR, TL, BR, BL};

struct Labels
{
    bool statuses[11];
    float scale[11];
    float opacity[11];
    int fonts[11];
    int styles[2];
    position positions[11];
    float labelSpacing = 20.0f;
};

struct Debug
{
    bool enabled = false;
    float debugNumber;
    std::string debugString;
};