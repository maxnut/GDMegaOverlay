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
	float titleColor[4] = { 0.75f, 0.157f, 0.157f, 1.0f }, borderColor[4] = { 0, 0, 0, 1.0f }, borderSize = 0,
		windowRounding;
	bool autoDeafen, fixPractice, onlyInRuns, hitboxOnly;
	char message[30];
	bool rainbowIcons, layoutMode;
	float rainbowSpeed;
	bool clickbot = false;
	float baseVolume = 1.0f, minPitch = 0.97f, maxPitch = 1.03f, minTimeDifference = 0.03f;
	double playMediumClicksAt = 0.07;
	bool confirmQuit, autoSyncMusic, hidePause = false;
	int musicMaxDesync = 80, hitboxOpacity = 200, borderOpacity = 255, hitboxKeyIndex;
	float clickColor[3] = { 1.0f, 0.0f, 0.0f };
	bool rainbowOutline = false, replaceMenuMusic = false, randomMusic = false;
	int musicIndex, randomMusicIndex;
	bool playSongButton = false, copySongButton = false;

	bool recording = false;
	int videoDimenstions[2] = { 1280, 720 };
	int videoFps = 60;
	float renderMusicVolume = 0.7f, renderClickVolume = 0.5f;
	bool includeClicks = true;
	char bitrate[8] = "30M";
	int clickSoundChunkSize = 30;
	float afterEndDuration = 1.0f;

	bool solidWavePulse = false, hitboxTrail = false;
	float hitboxTrailLength = 50.0f, hitboxMultiplier, noclipRedRate = 1.0f, noclipRedLimit = 100.0f,
		noclipRedRateDown = 1.0f;
	bool noclipRed = false;
	float noclipColor[3] = { 1.0f, 0.0f, 0.0f };
	bool showExtraInfo = true;
	int windowSnap = 0, menuKey = 9;
	bool snow = true, smartStartPos = false, gravityDetection = false, voidClick = true;
	int priority = 2;

	float version = 1.0f;

	bool accumulateRuns = false, rainbowMenu = false, dockSpace = false;
	float menuRainbowSpeed = 1, menuRainbowBrightness = 0.8f;

	bool hbmSolid = false, hmbHazard = true;

	char codec[20], extraArgs[60], pitchId[10], menuSongId[10];
	bool preventInput = false, rainbowPlayerC1 = false, rainbowPlayerC2 = false, rainbowWaveTrail = false, rainbowPlayerVehicle = false;
	char extraArgsAfter[60];

	bool safeModeEndscreen, practiceButtonEndscreen, cheatIndicatorEndscreen;

	float tpsBypass = 60, screenFPS = 60;

	bool discordRPC, enableHitboxMultiplier;
	float hitboxSolids, hitboxSpecial;
	bool twoPlayerOneKey;
	float pastel;
	bool holdAdvance, trajectory = false;
	float musicSpeed = 1;
	bool verticalSync, showCursor;

	bool tpsBypassBool = false, drawDivideBool = false, lockCursor = false, coinTracker = false, speedhackBool = false,
		autoSafeMode = false, disableBotCorrection = false, experimentalFeatures = false, tieMusicToSpeed;
	char levelPlayDetail[60] = "{name} - {author} ({best}%)", levelPlayState[60] = "{stars}* ({id})";
	char editorDetail[60] = "Editing {name}", editorState[60] = "{objects} objects";
	char menuDetail[60] = "Browsing Menus", menuState[60] = "";
	bool enableWaveTrailColor;

	float waveTrailColor[3];

	bool accuracySound, autoUpdateRespawn;

	float solidHitboxColor[3] = { 0, 0, 1 }, slopeHitboxColor[3] = { 0, 0, 1 }, hazardHitboxColor[3] = { 1, 0, 0 }, portalHitboxColor[3] = { 1, 0.498f, 1 }, padHitboxColor[3] = { 0, 1, 1 },
		ringHitboxColor[3] = { 0, 1, 1 }, collectibleHitboxColor[3] = { 0.87f, 0.87f, 0.87f }, modifierHitboxColor[3] = { 1, 1, 1 }, playerHitboxColor[3] = { 1, 0.247f, 0.247f },
		rotatedHitboxColor[3] = { 0.498f, 0, 0 }, centerHitboxColor[3] = { 0, 1, 0 };

	float backgroundColor[3] = { 0, 1, 1 }, blocksColor[3] = { 0, 1, 1 };

	float menuAnimationLength = 0.25f;

	float windowBgColor[3] = { 0.06f, 0.05f, 0.07f };

	int replayMode = 1;
};

enum position
{
	TR,
	TL,
	BR,
	BL
};

struct Labels
{
	bool statuses[14];
	float scale[14];
	float opacity[14];
	int fonts[14];
	char styles[4][20] = { "%.0f/%.0f", "%i/%i", "Accuracy: %.2f%%", "Deaths: %i" };
	position positions[14] = { TR, TR, TR, TR, TR, TR, TR, TR, TR, TR, TR, TR, TR, TR };
	float labelSpacing = 20.0f;
	bool hideLabels, rainbowLabels = false;
	float rainbowSpeed = 1.0f;
	float fpsUpdate = 0.5f;
};

struct Debug
{
	bool enabled = true;
	float debugNumber;
	std::string debugString;
};