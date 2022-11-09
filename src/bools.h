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
    float hitboxThickness = 0.5f, fontSize = 1;
    float titleColor[4] = {0.75f, 0.157f, 0.157f, 1.0f}, borderColor[4] = {0, 0, 0, 1.0f}, borderSize = 0, windowRounding;
    bool autoDeafen, fixPractice, onlyInRuns, hitboxOnly;
    char message[30];
    bool rainbowIcons, hideTestmode, layoutMode;
    float rainbowSpeed;
    int messageIndex;
    bool clickbot = false;
    float baseVolume = 1.0f, minPitch = 0.97f, maxPitch = 1.03f, minTimeDifference = 0.03f;
    double playMediumClicksAt = 0.07;
    bool confirmQuit, autoSyncMusic, hidePause;
    int musicMaxDesync = 80, hitboxOpacity = 200, borderOpacity = 255, hitboxKeyIndex;
    float clickColor[3] = {1.0f, 0.0f, 0.0f};
};

struct Labels
{
    bool statuses[11];
    float poscale[11][3];
    float opacity[11];
    int fonts[11];
    int styles[2];
};