#pragma once

struct HacksStr
{

    bool start = false;

    float fps = 60;
    float percentage = 0;
    float noClipAccuracyLimit;

    float speed = 1;
    bool showHitboxes, onlyOnDeath, showDecorations, hideattempts;

    int iconIds[12];

    bool startPosSwitcher;
    int muteKey;
    float waveSize;
    float respawnTime = 1.2f;
    bool lastCheckpoint = false, autoclicker, frameStep;
    float clickTime, releaseTime;
    int stepCount, stepIndex;
    bool botTextEnabled;
    int actionStart;
    float hitboxThickness = 0.5f, menuSize = 1;
    float titleColor[4] = {0.75f, 0.157f, 0.157f, 1.0f}, borderColor[4] = {0, 0, 0, 1.0f}, borderSize = 0, windowRounding;
    bool autoDeafen, fixPractice, onlyInRuns, hitboxOnly;
    char message[30];
    bool rainbowIcons, layoutMode;
    float rainbowSpeed;
    bool clickbot = false;
    float baseVolume = 1.0f, minPitch = 0.97f, maxPitch = 1.03f, minTimeDifference = 0.03f;
    double playMediumClicksAt = 0.07;
    bool confirmQuit, autoSyncMusic, hidePause = false;
    int musicMaxDesync = 80, hitboxOpacity = 200, borderOpacity = 255, hitboxKeyIndex;
    float clickColor[3] = {1.0f, 0.0f, 0.0f};
    bool rainbowOutline = false, replaceMenuMusic = false, randomMusic = false;
    int musicIndex, randomMusicIndex;

    bool recording = false;
    int videoDimenstions[2] = {1280, 720};
    int videoFps = 60;
    float renderMusicVolume = 0.7f, renderClickVolume = 0.5f;
    bool includeClicks = true;
    char bitrate[8] = "30M";
    int clickSoundChunkSize = 30;
    float afterEndDuration = 1.0f;

    bool solidWavePulse = false, hitboxTrail = false;
    float hitboxTrailLength = 50.0f, hitboxMultiplier, noclipRedRate = 1.0f, noclipRedLimit = 100.0f, noclipRedRateDown = 1.0f;
    bool noclipRed = false;
    float noclipColor[3] = {1.0f, 0.0f, 0.0f};
    bool showExtraInfo = true;
    int windowSnap = 0, menuKey = 9;
    bool snow = true, smartStartPos = false, gravityDetection = false, voidClick = true;
    int priority = 2;

    float version = 1.0f;

    bool accumulateRuns = false, rainbowMenu = false, dockSpace = false;
    float menuRainbowSpeed = 1, menuRainbowBrightness = 0.8f;

    bool hbmSolid = false, hmbHazard = true;

    char codec[20], extraArgs[60], pitchId[10], menuSongId[10];
    bool preventInput = false, rainbowPlayerC1 = false, rainbowPlayerC2 = false, rainbowPlayerVehicle = false;
    char extraArgsAfter[60];

    bool safeModeEndscreen, practiceButtonEndscreen, cheatIndicatorEndscreen;

    float tpsBypass = 60, screenFPS = 60;

    bool discordRPC, enableHitboxMultiplier;
    float hitboxSolids, hitboxSpecial;
    bool twoPlayerOneKey;
    float pastel;
    bool holdAdvance, trajectory = false;
    float musicSpeed = 1;

    bool tpsBypassBool = false, drawDivideBool = false, lockCursor = false, coinTracker = false;
};

enum position {TR, TL, BR, BL};

struct Labels
{
    bool statuses[14];
    float scale[14];
    float opacity[14];
    int fonts[14];
    char styles[4][20] = {"%.0f/%.0f", "%i/%i", "Accuracy: %.2f%%", "Deaths: %i"};
    position positions[14] = {TR,TR,TR,TR,TR,TR,TR,TR,TR,TR,TR,TR,TR,TR};
    float labelSpacing = 20.0f;
    bool hideLabels, rainbowLabels = false;
    float rainbowSpeed = 1.0f;
    float fpsUpdate = 0.5f;
};

struct Windows
{
    struct Vec2I
    {
        int x, y;
    };
    Vec2I positions[12];
};

struct Debug
{
    bool enabled = false;
    float debugNumber;
    std::string debugString;
};