#include "PlayLayer.h"
#include "EndLevelLayer.h"
#include "ReplayPlayer.h"
#include "Hacks.h"
#include "bools.h"
#include <imgui.h>
#include <vector>
#include <chrono>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <sstream>
#include "ExitAlert.h"
#include "Shortcuts.h"

extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

ExitAlert a;

bool PlayLayer::hadAction = false;
int PlayLayer::respawnAction = 0, PlayLayer::respawnAction2 = 0;

HitboxNode *drawer;
gd::PlayLayer *playlayer;

float percent, startPercent = 0, endPercent = 0, maxRun = 0, delta = -1, prevP = 0, pressTimer = 0, releaseTimer = 0, opacity = 0, p = 1;

std::string bestRun = "Best Run: none", text;

std::vector<float> clicksArr;
std::vector<gd::StartPosObject *> sp;
std::vector<gd::GameObject *> gravityPortals, dualPortals, gamemodePortals, miniPortals, speedChanges, mirrorPortals;
std::vector<bool> willFlip;

bool add = false, pressing, dead = false, hitboxDead = false, clickType = true, pressed = false, reset = true, holdingAdvance = false;

bool simulating = false, diedWhileSimulating = false;

ccColor3B iconCol, secIconCol;

int totalClicks, startPosIndex, noClipDeaths, actualDeaths, frames, steps = 0, bestRunRepeat;
unsigned int songLength;
int hackAmts[5];

CCNode *statuses[STATUSSIZE];
CCLabelBMFont *startPosText, *macroText;
CCScheduler *scheduler;
CCSize size, pixelSize;
CCSprite *noclipRed;

std::vector<gd::GameObject *> coinPos;

ReplayPlayer *replayPlayer;

std::chrono::time_point<std::chrono::high_resolution_clock> previous_frame, last_update;
float avg = 0.f;
int frame_count = 0;

void UpdateLabels(gd::PlayLayer *self);
void Change();

void SetupLabel(gd::PlayLayer *self, int index)
{
	auto st = static_cast<CCLabelBMFont *>(statuses[index]);
	st = CCLabelBMFont::create("", "bigFont.fnt");
	st->setZOrder(1000);
	st->setScale(0.5f);
	st->setOpacity(150.0f);
	self->m_uiLayer->addChild(st);
	statuses[index] = st;
}

void SmartStartPosSetup(gd::PlayLayer *self)
{
	willFlip.resize(gravityPortals.size());
	for (gd::StartPosObject *startPos : sp)
	{
		for (size_t i = 0; i < gravityPortals.size(); i++)
		{
			if (gravityPortals[i]->getPositionX() - 10 > startPos->getPositionX())
				break;
			if (gravityPortals[i]->getPositionX() - 10 < startPos->getPositionX())
				willFlip[i] = gravityPortals[i]->m_nObjectID == 11;
		}

		startPos->m_levelSettings->m_startDual = self->m_pLevelSettings->m_startDual;
		for (size_t i = 0; i < dualPortals.size(); i++)
		{
			if (dualPortals[i]->getPositionX() - 10 > startPos->getPositionX())
				break;
			if (dualPortals[i]->getPositionX() - 10 < startPos->getPositionX())
				startPos->m_levelSettings->m_startDual = dualPortals[i]->m_nObjectID == 286;
		}

		startPos->m_levelSettings->m_startGamemode = self->m_pLevelSettings->m_startGamemode;
		for (size_t i = 0; i < gamemodePortals.size(); i++)
		{
			if (gamemodePortals[i]->getPositionX() - 10 > startPos->getPositionX())
				break;
			if (gamemodePortals[i]->getPositionX() - 10 < startPos->getPositionX())
			{
				switch (gamemodePortals[i]->m_nObjectID)
				{
				case 12:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeCube;
					break;
				case 13:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeShip;
					break;
				case 47:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeBall;
					break;
				case 111:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeUfo;
					break;
				case 660:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeWave;
					break;
				case 745:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeRobot;
					break;
				case 1331:
					startPos->m_levelSettings->m_startGamemode = gd::Gamemode::kGamemodeSpider;
					break;
				}
			}
		}

		startPos->m_levelSettings->m_startMini = self->m_pLevelSettings->m_startMini;
		for (size_t i = 0; i < miniPortals.size(); i++)
		{
			if (miniPortals[i]->getPositionX() - 10 > startPos->getPositionX())
				break;
			if (miniPortals[i]->getPositionX() - 10 < startPos->getPositionX())
				startPos->m_levelSettings->m_startMini = miniPortals[i]->m_nObjectID == 101;
		}

		startPos->m_levelSettings->m_startSpeed = self->m_pLevelSettings->m_startSpeed;
		for (size_t i = 0; i < speedChanges.size(); i++)
		{
			if (speedChanges[i]->getPositionX() - 50 > startPos->getPositionX())
				break;
			if (speedChanges[i]->getPositionX() - 50 < startPos->getPositionX())
			{
				switch (speedChanges[i]->m_nObjectID)
				{
				case 200:
					startPos->m_levelSettings->m_startSpeed = gd::Speed::kSpeedSlow;
					break;
				case 201:
					startPos->m_levelSettings->m_startSpeed = gd::Speed::kSpeedNormal;
					break;
				case 202:
					startPos->m_levelSettings->m_startSpeed = gd::Speed::kSpeedFast;
					break;
				case 203:
					startPos->m_levelSettings->m_startSpeed = gd::Speed::kSpeedFaster;
					break;
				case 1334:
					startPos->m_levelSettings->m_startSpeed = gd::Speed::kSpeedFastest;
					break;
				}
			}
		}
	}
}

bool __fastcall PlayLayer::initHook(gd::PlayLayer *self, void *, gd::GJGameLevel *level)
{
	drawer = HitboxNode::getInstance();

	bool result = PlayLayer::init(self, level);

	playlayer = self;

	Hacks::AnticheatBypass();

	auto director = CCDirector::sharedDirector();
	size = director->getWinSize();
	pixelSize = CCEGLView::sharedOpenGLView()->getFrameSize();
	pixelSize.width /= 2;
	pixelSize.height /= 2;

	sp.clear();
	gamemodePortals.clear();
	mirrorPortals.clear();
	miniPortals.clear();
	dualPortals.clear();
	speedChanges.clear();
	gravityPortals.clear();
	willFlip.clear();
	hitboxDead = false;
	CCObject *obje;

	Hacks::cheatCheck.clear();

	for (size_t i = 0; i < Hacks::bypass["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(), Hacks::bypass["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < Hacks::creator["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(), Hacks::creator["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < Hacks::global["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(), Hacks::global["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < Hacks::level["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(), Hacks::level["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < Hacks::player["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(), Hacks::player["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	CCARRAY_FOREACH(self->m_pObjects, obje)
	{
		auto g = reinterpret_cast<gd::GameObject *>(obje);

		if (g->m_nObjectID == 31)
			sp.push_back(reinterpret_cast<gd::StartPosObject *>(obje));

		if (g->m_nObjectID == 1329 || g->m_nObjectID == 142)
		{
			coinPos.push_back(g);
		}

		if (hacks.smartStartPos)
		{
			switch (g->m_nObjectID)
			{
			case 10:
			case 11:
				gravityPortals.push_back(g);
				break;
			case 12:
			case 13:
			case 47:
			case 111:
			case 660:
			case 745:
			case 1331:
				gamemodePortals.push_back(g);
				break;
			case 45:
			case 46:
				mirrorPortals.push_back(g);
				break;
			case 99:
			case 101:
				miniPortals.push_back(g);
				break;
			case 286:
			case 287:
				dualPortals.push_back(g);
				break;
			case 200:
			case 201:
			case 202:
			case 203:
			case 1334:
				speedChanges.push_back(g);
				break;
			}
		}
	}

	if (hacks.smartStartPos)
		SmartStartPosSetup(self);

	startPosIndex = sp.size() - 1;

	percent = 0;
	endPercent = 0;
	maxRun = 0;
	bestRun = "Best Run: none";
	text = CCString::createWithFormat(labels.styles[2], 100.0f)->getCString();
	EndLevelLayer::accuracy = 100.0f;
	noClipDeaths = 0;
	totalClicks = 0;
	actualDeaths = 0;
	EndLevelLayer::deaths = 0;
	delta = 0;
	dead = false;
	clicksArr.clear();
	prevP = 0;

	pressTimer = 0;
	releaseTimer = hacks.releaseTime;

	const ccColor3B noclipColor = {hacks.noclipColor[0] * 255, hacks.noclipColor[1] * 255, hacks.noclipColor[2] * 255};
	noclipRed = CCSprite::createWithSpriteFrameName("block005b_05_001.png");
	noclipRed->setPosition({size.width / 2, size.height / 2});
	noclipRed->setScale(1000.0f);
	noclipRed->setColor(noclipColor);
	noclipRed->setOpacity(0);
	noclipRed->setZOrder(1000);
	self->m_uiLayer->addChild(noclipRed);

	for (size_t i = 0; i < STATUSSIZE; i++)
		SetupLabel(self, i);

	macroText = CCLabelBMFont::create("ciao", "bigFont.fnt");
	macroText->setZOrder(1000);
	macroText->setScale(0.5f);
	macroText->setOpacity(150.0f);
	macroText->setAnchorPoint(CCPointMake(0, macroText->getAnchorPoint().y));
	macroText->setPosition(CCPointMake(5, 15));
	self->m_uiLayer->addChild(macroText);

	for (size_t i = 0; i < STATUSSIZE; i++)
		UpdatePositions(i);

	if (sp.size() > 0)
	{
		startPosText = CCLabelBMFont::create("", "bigFont.fnt");
		startPosText->setZOrder(1000);
		startPosText->setScale(0.5f);
		startPosText->setOpacity(150.0f);
		startPosText->setPosition(size.width / 2, 15);
		self->m_uiLayer->addChild(startPosText);
		pressed = false;
	}

	UpdateLabels(self);

	auto gm = gd::GameManager::sharedState();

	iconCol = gm->colorForIdx(gm->getPlayerColor());
	secIconCol = gm->colorForIdx(gm->getPlayerColor2());

	scheduler = CCDirector::sharedDirector()->getScheduler();

	replayPlayer = &ReplayPlayer::getInstance();
	replayPlayer->StartPlaying(self);

	if (drawer)
		self->m_pObjectLayer->addChild(drawer, 32);

	if (hacks.smartStartPos && sp.size() > 0)
		Change();

	if (Hacks::ds.core && hacks.discordRPC)
	{
		discord::Activity activity{};
		activity.SetDetails(("Playing " + self->m_level->levelName).c_str());
		if (self->m_level->userName != "")
			activity.SetState(("by " + self->m_level->userName + " (" + std::to_string(self->m_level->normalPercent) + "%)").c_str());
		else
			activity.SetState((std::to_string(self->m_level->normalPercent) + "%").c_str());
		activity.GetAssets().SetLargeImage("cool");
		activity.GetAssets().SetLargeText(gd::GameManager::sharedState()->m_sPlayerName.c_str());
		activity.GetTimestamps().SetStart(Hacks::ds.timeStart);
		activity.SetType(discord::ActivityType::Playing);
		activity.GetAssets().SetSmallImage(Hacks::getDifficultyName(*self->m_level).c_str());
		Hacks::ds.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}

	return result;
}

bool PlayLayer::isBot = false;

bool __fastcall PlayLayer::pushButtonHook(gd::PlayerObject *self, void *, int PlayerButton)
{
	if (playlayer && !playlayer->m_hasCompletedLevel && replayPlayer && replayPlayer->IsRecording())
	{
		// if(hadAction) return true;
		if (self == playlayer->m_pPlayer2 && playlayer->m_pLevelSettings->m_twoPlayerMode || self == playlayer->m_pPlayer1)
		{
			replayPlayer->RecordAction(true, self, self == playlayer->m_pPlayer1);
		}
	}
	else if (playlayer && replayPlayer && replayPlayer->IsPlaying() && hacks.preventInput && !PlayLayer::isBot)
		return true;

	if (playlayer && playlayer->m_level->twoPlayerMode && hacks.twoPlayerOneKey && self == playlayer->m_pPlayer1)
	{
		PlayLayer::releaseButtonHook(playlayer->m_pPlayer2, 0, 0);
	}

	add = true;
	pressing = true;
	hadAction = true;

	return PlayLayer::pushButton(self, PlayerButton);
}

bool __fastcall PlayLayer::releaseButtonHook(gd::PlayerObject *self, void *, int PlayerButton)
{
	if (playlayer && !playlayer->m_hasCompletedLevel && replayPlayer && replayPlayer->IsRecording())
	{
		if (self == playlayer->m_pPlayer2 && delta > 0 && playlayer->m_bIsDualMode && playlayer->m_pLevelSettings->m_twoPlayerMode || self == playlayer->m_pPlayer1)
		{
			replayPlayer->RecordAction(false, self, self == playlayer->m_pPlayer1);
		}
	}
	else if (playlayer && replayPlayer && replayPlayer->IsPlaying() && hacks.preventInput && !PlayLayer::isBot)
		return true;

	if (playlayer && playlayer->m_level->twoPlayerMode && hacks.twoPlayerOneKey && self == playlayer->m_pPlayer1)
	{
		PlayLayer::pushButtonHook(playlayer->m_pPlayer2, 0, 0);
	}

	pressing = false;
	hadAction = true;

	return PlayLayer::releaseButton(self, PlayerButton);
}

std::string oldRun = "";

void __fastcall PlayLayer::destroyPlayer_H(gd::PlayLayer *self, void *, gd::PlayerObject *player, gd::GameObject *obj)
{
	if (simulating)
	{
		diedWhileSimulating = true;
		return;
	}
	if (delta > 0.2f && !Hacks::player["mods"][0]["toggle"] && !Hacks::player["mods"][2]["toggle"] && !self->m_isDead)
	{
		float run = ((player->getPositionX() / self->m_levelLength) * 100.0f) - startPercent;
		endPercent = (player->getPositionX() / self->m_levelLength) * 100.0f;
		std::string runStr = (int)startPercent != 0 ? "Best Run: " + std::to_string((int)startPercent) + "-" + std::to_string((int)endPercent) : "Best Run: " + std::to_string((int)endPercent) + "%";
		if ((int)startPercent != (int)endPercent && run > maxRun)
		{
			bestRunRepeat = 0;
			maxRun = run;
			bestRun.clear();
			bestRun = "";
			bestRun = runStr;
			oldRun = runStr;
		}
		else if (runStr == oldRun && hacks.accumulateRuns)
		{
			bestRunRepeat++;
			if (bestRunRepeat > 1)
			{
				for (size_t i = 0; i < 1 + bestRunRepeat / 10; i++)
					bestRun.pop_back();
				bestRun.pop_back();
			}
			bestRun += "x" + std::to_string(bestRunRepeat + 1);
		}
	}
	PlayLayer::destroyPlayer(self, player, obj);
}

gd::GameSoundManager *__fastcall PlayLayer::levelCompleteHook(gd::PlayLayer *self)
{
	float run = 100.0f - startPercent;
	endPercent = 100.0f;
	std::string runStr = (int)startPercent != 0 ? "Best Run: " + std::to_string((int)startPercent) + "-" + std::to_string((int)endPercent) : "Best Run: " + std::to_string((int)endPercent) + "%";
	if (run > maxRun)
	{
		bestRunRepeat = 0;
		maxRun = run;
		bestRun.clear();
		bestRun = "";
		bestRun = runStr;
		oldRun = runStr;
	}
	else if (runStr == oldRun && hacks.accumulateRuns)
	{
		bestRunRepeat++;
		if (bestRunRepeat > 1)
		{
			bestRun.pop_back();
			bestRun.pop_back();
		}
		bestRun += "x" + std::to_string(bestRunRepeat);
	}

	if (pressed)
	{
		pressed = false;
		keybd_event(VK_MENU, 0x38, 0, 0);
		keybd_event(hacks.muteKey, 0x50, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(hacks.muteKey, 0x50, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
	}

	return PlayLayer::levelComplete(self);
}

void __fastcall PlayLayer::hkDeath(void *self, void *, void *go, void *powerrangers)
{
	if (simulating)
	{
		diedWhileSimulating = true;
		return;
	}
	if (delta > 0.1f)
	{
		dead = true;
		hitboxDead = true;
	}

	PlayLayer::death(self, go, powerrangers);
}

bool PlayLayer::IsCheating()
{
	size_t total = 0;
	for (size_t i = 0; i < Hacks::bypass["mods"].size(); i++)
	{
		if (Hacks::bypass["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < Hacks::creator["mods"].size(); i++)
	{
		if (Hacks::creator["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < Hacks::global["mods"].size(); i++)
	{
		if (Hacks::global["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < Hacks::level["mods"].size(); i++)
	{
		if (Hacks::level["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < Hacks::player["mods"].size(); i++)
	{
		if (Hacks::player["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	if (hacks.fps > 360.0f || hacks.screenFPS > 360.0f || hacks.tpsBypass > 360.0f || hacks.autoclicker || hacks.frameStep || scheduler->getTimeScale() != 1 || replayPlayer->IsPlaying() || hacks.layoutMode || hacks.enableHitboxMultiplier || hacks.showHitboxes && !hacks.onlyOnDeath)
		return true;

	return false;
}

void UpdateLabels(gd::PlayLayer *self)
{

	if (labels.hideLabels)
	{
		self->m_uiLayer->setVisible(false);
		return;
	}
	self->m_uiLayer->setVisible(true);

	const ccColor3B red = {255, 0, 0};
	const ccColor3B white = {255, 255, 255};
	const ccColor3B green = {0, 255, 0};
	const ccColor3B yellow = {255, 165, 0};

	auto fontPtr = static_cast<CCLabelBMFont *>(statuses[0]);

	float r, g, b;
	if (labels.rainbowLabels)
		ImGui::ColorConvertHSVtoRGB(ImGui::GetTime() * labels.rainbowSpeed, 1, 1, r, g, b);
	const ccColor3B rainbow = {r * 255, g * 255, b * 255};

	if (labels.statuses[0] && scheduler)
	{
		fontPtr->setString(".");
		fontPtr->setOpacity(labels.opacity[0]);
		if (PlayLayer::IsCheating())
			fontPtr->setColor(red);
		else if (Hacks::level["mods"][24]["toggle"])
			fontPtr->setColor(yellow);
		else
			fontPtr->setColor(green);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[1]);

	if (labels.statuses[1])
	{
		const auto now = std::chrono::high_resolution_clock::now();

		const std::chrono::duration<float> diff = now - previous_frame;
		avg += diff.count();
		frame_count++;

		if (std::chrono::duration<float>(now - last_update).count() > labels.fpsUpdate)
		{
			last_update = now;
			const auto fps = static_cast<float>(frame_count) / avg;
			avg = 0.f;
			frame_count = 0;

			std::string text;

			fontPtr->setString(CCString::createWithFormat(labels.styles[0], ImGui::GetIO().Framerate, fps)->getCString());
		}

		previous_frame = now;

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[2]);

	if (labels.statuses[2])
	{
		if (clicksArr.size() > 0)
		{
			float currentTime = GetTickCount();
			for (int i = 0; i < clicksArr.size(); i++)
			{
				if ((currentTime - clicksArr[i]) / 1000.0f >= 1)
					clicksArr.erase(clicksArr.begin() + i);
			}
		}

		fontPtr->setString(CCString::createWithFormat(labels.styles[1], clicksArr.size(), totalClicks)->getCString());
		const ccColor3B pressed = {hacks.clickColor[0] * 255, hacks.clickColor[1] * 255, hacks.clickColor[2] * 255};
		fontPtr->setColor(pressing ? pressed : labels.rainbowLabels ? rainbow
																	: white);
	}
	else
	{
		fontPtr->setString("");
		clicksArr.clear();
	}

	fontPtr = static_cast<CCLabelBMFont *>(statuses[3]);

	if (labels.statuses[3])
	{
		fontPtr->setString(text.c_str());
		fontPtr->setColor(dead ? red : labels.rainbowLabels ? rainbow
															: white);

		if (hacks.noClipAccuracyLimit > 0 && p * 100.0f < hacks.noClipAccuracyLimit)
		{
			p = 1;
			bool t = Hacks::player["mods"][0]["toggle"];
			Hacks::player["mods"][0]["toggle"] = false;
			Hacks::ToggleJSONHack(Hacks::player, 0, false);
			Hacks::player["mods"][0]["toggle"] = t;
		}
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[4]);

	if (labels.statuses[4])
	{
		fontPtr->setString(CCString::createWithFormat(labels.styles[3], actualDeaths)->getCString());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[5]);

	if (labels.statuses[5])
	{
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream s;
		s << std::put_time(&tm, "%H:%M:%S");

		fontPtr->setString(s.str().c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[6]);

	if (labels.statuses[6])
	{
		fontPtr->setString(bestRun.c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[7]);

	if (labels.statuses[7])
	{
		fontPtr->setString((std::to_string(self->m_level->attempts) + " Attempts").c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[8]);

	if (labels.statuses[8] && !hacks.onlyInRuns || labels.statuses[8] && (self->m_isPracticeMode || self->m_isTestMode) && hacks.onlyInRuns)
	{
		fontPtr->setString(("From " + std::to_string((int)startPercent) + "%").c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[9]);

	if (labels.statuses[9])
	{
		fontPtr->setString(hacks.message);

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[10]);

	if (labels.statuses[10])
	{
		fontPtr->setString(("Attempt " + std::to_string(self->m_currentAttempt)).c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[11]);

	if (labels.statuses[11])
	{
		fontPtr->setString(("Level ID: " + std::to_string(self->m_level->levelID)).c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[12]);

	if (labels.statuses[12])
	{
		if (!self->m_hasCompletedLevel)
			fontPtr->setString((std::to_string(self->m_attemptJumpCount) + " Jumps").c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[13]);

	if (labels.statuses[13])
	{
		if (!self->m_hasCompletedLevel)
			fontPtr->setString(("Frame " + std::to_string(frames)).c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	if (replayPlayer && hacks.botTextEnabled && macroText)
	{
		if (replayPlayer->IsRecording())
			macroText->setString(("Recording: " + std::to_string(replayPlayer->GetActionsSize())).c_str());
		else if (replayPlayer->IsPlaying())
			macroText->setString(("Playing: " + std::to_string(replayPlayer->GetActionIndex()) + "/" + std::to_string(replayPlayer->GetActionsSize())).c_str());
		else
			macroText->setString("");
	}
	else
		macroText->setString("");

	if (sp.size() > 0 && hacks.startPosSwitcher && startPosText)
	{
		startPosText->setString((std::to_string(startPosIndex + 1) + "/" + std::to_string(sp.size())).c_str());
	}
	else if (sp.size() > 0 && startPosText)
		startPosText->setString("");
}

const char *actualFonts[] = {"bigFont.fnt", "chatFont.fnt", "gjFont01.fnt", "gjFont02.fnt", "gjFont03.fnt", "gjFont04.fnt", "gjFont05.fnt", "gjFont06.fnt", "gjFont07.fnt", "gjFont08.fnt", "gjFont09.fnt", "gjFont10.fnt", "gjFont11.fnt", "goldFont.fnt"};

void PlayLayer::UpdatePositions(int index)
{
	if (playlayer == nullptr)
		return;

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	auto ptr = static_cast<CCLabelBMFont *>(statuses[index]);

	float tr = 0, tl = 0, br = 0, bl = 0, thisLabel;

	for (size_t i = 0; i < STATUSSIZE; i++)
	{
		if (!labels.statuses[i] || i == 8 && hacks.onlyInRuns && !(playlayer->m_isTestMode || playlayer->m_isPracticeMode))
			continue;

		switch (labels.positions[i])
		{
		case TR:
			if (index == i)
				thisLabel = tr;
			tr += 1.0f * labels.scale[i];
			break;
		case TL:
			if (index == i)
				thisLabel = tl;
			tl += 1.0f * labels.scale[i];
			break;
		case BR:
			if (index == i)
				thisLabel = br;
			br += 1.0f * labels.scale[i];
			break;
		case BL:
			if (index == i)
				thisLabel = bl;
			bl += 1.0f * labels.scale[i];
			break;
		}
	}

	float height = 0, x = 0;

	switch (labels.positions[index])
	{
	case TR:
		height = size.height - 10 - (thisLabel * labels.labelSpacing);
		x = size.width - 5;
		break;
	case TL:
		height = size.height - 10 - (thisLabel * labels.labelSpacing);
		x = 5.0f;
		break;
	case BR:
		height = 10.0f + (thisLabel * labels.labelSpacing);
		x = size.width - 5;
		break;
	case BL:
		height = 10.0f + (thisLabel * labels.labelSpacing);
		x = 5.0f;
		break;
	}

	statuses[index]->setPosition(x, height);

	if (index != 0)
	{
		ptr->setFntFile(actualFonts[labels.fonts[index]]);
		ptr->setOpacity(labels.opacity[index]);
	}
	if (labels.scale[index] < 0.1f || labels.scale[index] > 1.5f)
		labels.scale[index] = 1.0f;

	float sc = labels.scale[index] * 0.45f;
	if (index == 0)
	{
		sc *= 1.8f;
		statuses[index]->setPosition(x, height + 10);
	}

	switch (labels.fonts[index])
	{
	case 1:
		sc *= 2.2f; // xd
		statuses[index]->setPosition(x, height + 10);
		break;
	case 13:
		sc *= 1.1f;
		break;
	}

	statuses[index]->setScale(sc);
	if (labels.opacity[index] < 1)
		labels.opacity[index] = 255;
	else if (labels.opacity[index] > 255)
		labels.opacity[index] = 255;
	statuses[index]->setAnchorPoint(statuses[index]->getPositionX() > 284.5f ? CCPointMake(1.0f, statuses[index]->getAnchorPoint().y) : CCPointMake(0.0f, statuses[index]->getAnchorPoint().y));
}

void PlayLayer::SyncMusic()
{
	if (!playlayer)
		return;
	float f = playlayer->timeForXPos(playlayer->m_pPlayer1->getPositionX());
	unsigned int p;
	unsigned int *pos = &p;
	float offset = playlayer->m_pLevelSettings->m_songStartOffset * 1000;
	gd::FMODAudioEngine::sharedEngine()->m_pGlobalChannel->getPosition(pos, FMOD_TIMEUNIT_MS);
	if (std::abs((int)(f * 1000) - (int)p + offset) > hacks.musicMaxDesync && !playlayer->m_hasCompletedLevel)
	{
		gd::FMODAudioEngine::sharedEngine()->m_pGlobalChannel->setPosition(static_cast<uint32_t>(f * 1000) + static_cast<uint32_t>(offset), FMOD_TIMEUNIT_MS);
	}
}

bool lastFrameDead = false;

void Update(gd::PlayLayer *self, float dt)
{
	percent = (self->m_pPlayer1->getPositionX() / self->m_levelLength) * 100.0f;
	self->m_attemptLabel->setVisible(!hacks.hideattempts);
	PlayLayer::hadAction = false;

	if (hacks.autoSyncMusic)
	{
		PlayLayer::SyncMusic();
	}

	if (drawer && !hacks.trajectory)
		drawer->clear();

	if (hacks.autoDeafen && !pressed && hacks.percentage > startPercent && percent > hacks.percentage)
	{
		pressed = true;
		keybd_event(VK_MENU, 0x38, 0, 0);
		keybd_event(hacks.muteKey, 0, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(hacks.muteKey, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
	}

	if (hacks.autoclicker)
	{

		if (pressTimer <= 0 && clickType)
		{
			clickType = false;
			releaseTimer = hacks.clickTime;
			playlayer->pushButton(0, false);
		}
		if (releaseTimer <= 0 && !clickType)
		{
			clickType = true;
			pressTimer = hacks.releaseTime;
			playlayer->releaseButton(0, false);
		}

		if (clickType)
		{
			pressTimer -= dt;
		}
		else
		{
			releaseTimer -= dt;
		}
	}
	else
	{
		pressTimer = hacks.clickTime;
		releaseTimer = hacks.releaseTime;
		clickType = true;
	}

	if (add)
	{
		add = false;
		clicksArr.push_back(GetTickCount());
		totalClicks++;
	}

	if (self->m_pPlayer1->getPositionX() != prevP)
	{
		delta += dt;
		frames++;
		if (dead && !lastFrameDead)
		{
			actualDeaths++;
			EndLevelLayer::deaths++;
		}

		lastFrameDead = dead;

		if (dead && !self->m_hasCompletedLevel && Hacks::player["mods"][0]["toggle"] || dead && !self->m_hasCompletedLevel && Hacks::player["mods"][2]["toggle"])
		{
			noClipDeaths++;
			if (opacity < hacks.noclipRedLimit)
				opacity += hacks.noclipRedRate;
			else
				opacity = hacks.noclipRedLimit;
		}
		else
		{
			if (opacity > 0)
				opacity -= hacks.noclipRedRateDown;
			else
				opacity = 0;
		}

		if (!hacks.noclipRed)
			opacity = 0;

		noclipRed->setOpacity((int)opacity);
	}
	prevP = self->m_pPlayer1->getPositionX();

	bool cheat = PlayLayer::IsCheating();
	if (cheat != (bool)Hacks::level["mods"][24]["toggle"] && hacks.autoSafeMode)
	{
		Hacks::level["mods"][24]["toggle"] = cheat;
		Hacks::ToggleJSONHack(Hacks::level, 24, false);
	}

	if (noClipDeaths == 0)
	{
		text = CCString::createWithFormat(labels.styles[2], 100.0f)->getCString();
		EndLevelLayer::accuracy = 100.0f;
	}
	else if (labels.statuses[3] || hacks.showExtraInfo)
	{
		p = (float)(frames - noClipDeaths) / (float)frames;
		float acc = p * 100.f;
		text = CCString::createWithFormat(labels.styles[2], acc)->getCString();
		EndLevelLayer::accuracy = p * 100.f;
	}

	if (hacks.rainbowIcons)
	{
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(self->m_totalTime * hacks.rainbowSpeed, hacks.pastel, 1, r, g, b);
		const ccColor3B col = {(GLubyte)(r * 255.0f), (GLubyte)(g * 255.0f), (GLubyte)(b * 255.0f)};

		if (hacks.rainbowPlayerC1)
		{
			self->m_pPlayer1->setColor(col);
			self->m_pPlayer2->setColor(col);
			self->m_pPlayer1->m_spiderSprite->setColor(col);
			self->m_pPlayer1->m_robotSprite->setColor(col);
			self->m_pPlayer2->m_spiderSprite->setColor(col);
			self->m_pPlayer2->m_robotSprite->setColor(col);
		}
		if (hacks.rainbowPlayerC2)
		{
			self->m_pPlayer1->m_iconSprite->setColor(col);
			self->m_pPlayer2->m_iconSprite->setColor(col);
			self->m_pPlayer1->m_iconSpriteSecondary->setColor(col);
			self->m_pPlayer2->m_iconSpriteSecondary->setColor(col);
			self->m_pPlayer1->m_iconSpriteWhitener->setColor(col);
			self->m_pPlayer2->m_iconSpriteWhitener->setColor(col);
			self->m_pPlayer1->m_unk500->setColor(col);
			self->m_pPlayer2->m_unk500->setColor(col);
		}
		if (hacks.rainbowPlayerVehicle)
		{
			self->m_pPlayer1->m_vehicleSprite->setColor(col);
			self->m_pPlayer2->m_vehicleSprite->setColor(col);
			self->m_pPlayer1->m_vehicleSpriteWhitener->setColor(col);
			self->m_pPlayer2->m_vehicleSpriteWhitener->setColor(col);
			self->m_pPlayer1->m_vehicleSpriteSecondary->setColor(col);
			self->m_pPlayer2->m_vehicleSpriteSecondary->setColor(col);
			reinterpret_cast<CCNodeRGBA *>(self->m_pPlayer1->m_waveTrail)->setColor(col);
			reinterpret_cast<CCNodeRGBA *>(self->m_pPlayer2->m_waveTrail)->setColor(col);
		}

		if (hacks.rainbowOutline)
		{
			self->m_pPlayer1->m_iconGlow->setColor(col);
			self->m_pPlayer2->m_iconGlow->setColor(col);
			self->m_pPlayer1->m_vehicleGlow->setChildColor(col);
			self->m_pPlayer2->m_vehicleGlow->setChildColor(col);
		}
	}

	UpdateLabels(self);

	if (hacks.lockCursor && !Hacks::show && !self->m_hasCompletedLevel && !self->m_isDead)
		SetCursorPos(pixelSize.width, pixelSize.height);

	dead = false;

	if (replayPlayer)
		replayPlayer->Update(self);

	if (replayPlayer->recorder.m_recording)
		replayPlayer->recorder.handle_recording(self, dt);

	float xp = self->m_pPlayer1->getPositionX();

	if (debug.enabled)
	{
		float prevYAccel = self->m_pPlayer1->m_yAccel;
		float prex = self->m_pPlayer1->m_position.x;
		std::stringstream strstr;
		strstr << " yaccel:" << self->m_pPlayer1->m_yAccel << " xaccel:" << self->m_pPlayer1->m_xAccel << " playerspeed:" << self->m_pPlayer1->m_playerSpeed;

		PlayLayer::update(self, dt);

		strstr << " yacceldiff:" << self->m_pPlayer1->m_yAccel - prevYAccel << " xdiff:" << self->m_pPlayer1->m_position.x - prex << " scale:" << self->m_pPlayer1->getScale();
		debug.debugString = strstr.str();
		debug.debugNumber = self->m_pPlayer1->getRotation();
	}
	else
		PlayLayer::update(self, dt);

	if (self->m_pPlayer1->m_waveTrail && hacks.solidWavePulse)
		self->m_pPlayer1->m_waveTrail->m_pulseSize = hacks.waveSize;
	if (self->m_pPlayer2->m_waveTrail && hacks.solidWavePulse)
		self->m_pPlayer2->m_waveTrail->m_pulseSize = hacks.waveSize;

	if (hacks.layoutMode)
	{
		auto p = self->getChildren()->objectAtIndex(0);
		if (frames > 10)
			static_cast<CCSprite *>(p)->setColor({50, 50, 255});

		for (int s = self->sectionForPos(xp) - 5; s < self->sectionForPos(xp) + 6; ++s)
		{
			if (s < 0)
				continue;
			if (s >= self->m_sectionObjects->count())
				break;
			auto section = static_cast<CCArray *>(self->m_sectionObjects->objectAtIndex(s));
			for (size_t i = 0; i < section->count(); ++i)
			{
				auto o = static_cast<gd::GameObject *>(section->objectAtIndex(i));

				if (o->getType() == gd::GameObjectType::kGameObjectTypeDecoration && o->isVisible() &&
					(o->m_nObjectID != 44 && o->m_nObjectID != 749 && o->m_nObjectID != 12 && o->m_nObjectID != 38 && o->m_nObjectID != 47 && o->m_nObjectID != 111 && o->m_nObjectID != 8 && o->m_nObjectID != 13 && o->m_nObjectID != 660 && o->m_nObjectID != 745 && o->m_nObjectID != 101 && o->m_nObjectID != 99 && o->m_nObjectID != 1331))
				{
					o->setVisible(false);
				}
			}
		}
	}

	if (drawer && hacks.showHitboxes && !hacks.onlyOnDeath || drawer && hacks.showHitboxes && hacks.onlyOnDeath && hitboxDead && playlayer->m_isDead)
	{
		drawer->setVisible(true);

		if (hacks.coinTracker)
		{
			for (size_t i = 0; i < coinPos.size(); i++)
			{
				if (coinPos[i] && self->m_pPlayer1->getPositionX() <= coinPos[i]->getPositionX())
					drawer->drawSegment(self->m_pPlayer1->getPosition(), coinPos[i]->getPosition(), 0.5f, ccc4f(0, 1, 0, 1));
			}
		}

		if (self->m_pPlayer1)
		{
			if (hacks.hitboxTrail)
				drawer->addToPlayer1Queue(self->m_pPlayer1->getObjectRect());
			drawer->drawForPlayer1(self->m_pPlayer1);
		}
		if (self->m_pPlayer2)
		{
			if (hacks.hitboxTrail)
				drawer->addToPlayer2Queue(self->m_pPlayer2->getObjectRect());
			drawer->drawForPlayer2(self->m_pPlayer2);
		}

		for (int s = self->sectionForPos(xp) - 5; s < self->sectionForPos(xp) + 6; ++s)
		{
			if (s < 0)
				continue;
			if (s >= self->m_sectionObjects->count())
				break;
			auto section = static_cast<CCArray *>(self->m_sectionObjects->objectAtIndex(s));
			for (size_t i = 0; i < section->count(); ++i)
			{
				auto obj = static_cast<gd::GameObject *>(section->objectAtIndex(i));

				if (hacks.hitboxOnly)
					obj->setOpacity(0);
				if (obj->m_nObjectID != 749 && obj->getObjType() == gd::GameObjectType::kGameObjectTypeDecoration)
					continue;
				if (!obj->m_bActive)
					continue;

				drawer->drawForObject(obj);
			}
		}
	}
}

float PlayLayer::GetStartPercent()
{
	return startPercent;
}

void __fastcall PlayLayer::pauseGameHook(gd::PlayLayer *self, void *, bool idk)
{
	if (replayPlayer && replayPlayer->IsRecording())
	{
		replayPlayer->RecordAction(false, self->m_pPlayer1, true);
		replayPlayer->RecordAction(false, self->m_pPlayer2, false);
	}
	PlayLayer::pauseGame(self, idk);
}

cocos2d::CCTouch *_touch = nullptr;
cocos2d::CCEvent *_event = nullptr;

void __fastcall PlayLayer::uiOnPauseHook(gd::UILayer *self, void *, CCObject *obj)
{
	if (hacks.voidClick && _touch)
		self->ccTouchEnded(_touch, _event);
	PlayLayer::uiOnPause(self, obj);
}

void __fastcall PlayLayer::uiTouchBeganHook(gd::UILayer *self, void *, cocos2d::CCTouch *touch, cocos2d::CCEvent *evnt)
{
	_touch = touch;
	_event = evnt;
	PlayLayer::uiTouchBegan(self, touch, evnt);
}

gd::GameObject *__fastcall PlayLayer::hasBeenActivatedByPlayerHook(gd::GameObject *self, void *, gd::GameObject *other)
{
	if (simulating && self->getType() != gd::GameObjectType::kGameObjectTypeSlope && self->getType() != gd::GameObjectType::kGameObjectTypeSolid)
		return other;

	return PlayLayer::hasBeenActivatedByPlayer(self, other);
}

void __fastcall PlayLayer::addPointHook(gd::HardStreak *self, void *, CCPoint point)
{
	if (simulating)
		return;
	PlayLayer::addPoint(self, point);
}

void ProcessUpdate(gd::PlayLayer *self, float dt)
{
	if (hacks.trajectory)
	{
		drawer->setVisible(true);
		drawer->clear();
		CheckpointData cd = CheckpointData::fromPlayer(self->m_pPlayer1);
		auto pos = self->m_pPlayer1->getPosition();

		simulating = true;
		diedWhileSimulating = false;

		PlayLayer::releaseButton(self->m_pPlayer1, 0);

		for (int i = 0; i < 240; i++)
		{
			PlayLayer::checkCollisionsHook(self, 0, self->m_pPlayer1);
			auto prevPos = self->m_pPlayer1->getPosition();
			self->m_pPlayer1->update(0.25f);
			if (i == 239 || diedWhileSimulating)
				drawer->drawForPlayer1(self->m_pPlayer1);
			else
				drawer->drawSegment(prevPos, self->m_pPlayer1->getPosition(), 0.65f, ccc4f(0, 1, 0, 1));
			if (diedWhileSimulating)
				break;
		}

		cd.Apply(self->m_pPlayer1, false);
		self->m_pPlayer1->setPosition(pos);
		diedWhileSimulating = false;

		PlayLayer::pushButton(self->m_pPlayer1, 0);

		for (int i = 0; i < 240; i++)
		{
			PlayLayer::checkCollisionsHook(self, 0, self->m_pPlayer1);
			auto prevPos = self->m_pPlayer1->getPosition();
			self->m_pPlayer1->update(0.25f);
			if (i == 239 || diedWhileSimulating)
				drawer->drawForPlayer1(self->m_pPlayer1);
			else
				drawer->drawSegment(prevPos, self->m_pPlayer1->getPosition(), 0.65f, ccc4f(0, 1, 0, 1));
			if (diedWhileSimulating)
				break;
		}

		simulating = false;

		self->m_pPlayer1->setPosition(pos);
		cd.Apply(self->m_pPlayer1, false);
		Update(self, dt);
	}
	else
		Update(self, dt);
}

void __fastcall PlayLayer::updateHook(gd::PlayLayer *self, void *, float dt)
{
	if (hacks.frameStep)
	{
		if ((steps > 0 || holdingAdvance) && drawer)
		{
			ProcessUpdate(self, dt);
			steps--;
		}
	}
	else
		ProcessUpdate(self, dt);

	return;
}

void __fastcall PlayLayer::triggerObjectHook(gd::EffectGameObject *self, void *, gd::GJBaseGameLayer *idk)
{
	if (simulating)
		return;
	auto id = self->m_nObjectID;
	if (hacks.layoutMode && (id == 899 || id == 1006 || id == 1007 || id == 105 || id == 29 || id == 56 || id == 915 || id == 30 || id == 58))
		return;
	PlayLayer::triggerObject(self, idk);
}

void Change()
{
	auto opacityPulseBegin = CCFadeTo::create(0, 255);
	auto opacityPulseWait = CCFadeTo::create(0.4, 255);
	auto opacityPulseEnd = CCFadeTo::create(0.3, 0);

	CCArray actions;
	actions.addObject(opacityPulseBegin);
	actions.addObject(opacityPulseWait);
	actions.addObject(opacityPulseEnd);

	startPosText->runAction(CCSequence::create(&actions));

	if (playlayer->unk330)
		playlayer->unk330->release();
	playlayer->unk330 = nullptr;

	if (startPosIndex >= 0)
	{
		playlayer->m_isTestMode = true;
		playlayer->m_startPos = sp[startPosIndex];
		playlayer->m_playerStartPosition = CCPointMake(sp[startPosIndex]->getPositionX(), sp[startPosIndex]->getPositionY());
	}
	else
	{
		playlayer->m_isTestMode = false;
		playlayer->m_startPos = nullptr;
		playlayer->m_playerStartPosition = CCPointMake(0, 105);
	}

	PlayLayer::resetLevelHook(playlayer, nullptr);

	if (playlayer->m_bIsPaused)
		gd::GameSoundManager::sharedState()->stopBackgroundMusic();
}

void __fastcall PlayLayer::resetLevelHook(gd::PlayLayer *self, void *)
{
	if (replayPlayer && replayPlayer->IsRecording())
	{
		replayPlayer->UpdateFrameOffset();
		replayPlayer->GetReplay()->RemoveActionsAfter(replayPlayer->GetFrame());
	}

	hitboxDead = false;

	// reset stuff
	totalClicks = 0;
	actualDeaths = 0;
	EndLevelLayer::deaths = 0;
	lastFrameDead = false;
	noClipDeaths = 0;
	delta = 0;
	steps = 0;
	pressTimer = 0;
	opacity = 0;
	releaseTimer = hacks.releaseTime;
	clickType = true;
	dead = false;
	frames = 0;
	self->m_attemptJumpCount = 0;
	Hacks::ToggleJSONHack(Hacks::player, 0, false);
	clicksArr.clear();
	if (drawer)
	{
		drawer->clearQueue();
		drawer->m_isMini1 = self->m_pLevelSettings->m_startMini;
		drawer->m_isMini2 = self->m_pLevelSettings->m_startMini;
	}

	simulating = false;
	diedWhileSimulating = false;

	for (size_t i = 0; i < STATUSSIZE; i++)
		UpdatePositions(i);

	// unmute
	if (pressed)
	{
		pressed = false;
		keybd_event(VK_MENU, 0x38, 0, 0);
		keybd_event(hacks.muteKey, 0x50, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(hacks.muteKey, 0x50, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
	}

	// calculate startpercent
	if (self->m_checkpoints->count() > 0)
	{
		auto porcodio = self->m_checkpoints->lastObject();
		gd::CheckpointObject *go = reinterpret_cast<gd::CheckpointObject *>(porcodio);
		startPercent = (go->m_player1->m_position.x / self->m_levelLength) * 100.0f;
	}
	else
	{
		startPercent = (self->m_playerStartPosition.x / self->m_levelLength) * 100.0f;
	}

	PlayLayer::resetLevel(self);

	if (playlayer)
	{
		self->m_pPlayer1->setColor(iconCol);
		self->m_pPlayer1->m_iconSprite->setColor(secIconCol);
		self->m_pPlayer1->m_vehicleSprite->setColor(secIconCol);
		self->m_pPlayer1->m_spiderSprite->setColor(secIconCol);
		self->m_pPlayer1->m_robotSprite->setColor(secIconCol);
		self->m_pPlayer2->setColor(secIconCol);
		self->m_pPlayer2->m_iconSprite->setColor(iconCol);
		self->m_pPlayer2->m_vehicleSprite->setColor(iconCol);
		self->m_pPlayer2->m_spiderSprite->setColor(iconCol);
		self->m_pPlayer2->m_robotSprite->setColor(iconCol);
		reinterpret_cast<CCNodeRGBA *>(self->m_pPlayer1->m_waveTrail)->setColor(iconCol);
		reinterpret_cast<CCNodeRGBA *>(self->m_pPlayer2->m_waveTrail)->setColor(iconCol);
	}

	if (hacks.gravityDetection && startPosIndex >= 0 && gravityPortals.size() > 0 && willFlip[startPosIndex])
	{
		self->m_pPlayer1->flipGravity(true, false);
		self->m_pPlayer2->flipGravity(true, true);
	}

	if (replayPlayer)
	{
		replayPlayer->Reset(self);
		replayPlayer->recorder.update_song_offset(self);
	}
}

void __fastcall PlayLayer::togglePracticeModeHook(gd::PlayLayer *self, void *edx, bool on)
{
	if (percent >= 100.0f && hacks.lastCheckpoint)
	{
		PlayLayer::togglePracticeMode(self, true);
		PlayLayer::resetLevelHook(self, nullptr);
	}
	else
	{
		PlayLayer::togglePracticeMode(self, on);
	}
}

void __fastcall PlayLayer::onQuitHook(gd::PlayLayer *self, void *)
{
	if (hacks.confirmQuit && !self->m_hasCompletedLevel)
		gd::FLAlertLayer::create(&a, "Confirm", "No", "Yes", "Do you really want to quit?")->show();
	else
	{
		PlayLayer::Quit();
	}
}

void *__fastcall PlayLayer::getObjectRectHook(cocos2d::CCNode *obj, void *, gd::GameObject *self, float w, float h)
{
	if (hacks.enableHitboxMultiplier && !EndLevelLayer::nameChildFind(obj, "PlayerObject"))
	{
		switch ((reinterpret_cast<gd::GameObject *>(obj))->getType())
		{
		case gd::GameObjectType::kGameObjectTypeSolid:
		case gd::GameObjectType::kGameObjectTypeSlope:
		{
			w *= hacks.hitboxSolids;
			h *= hacks.hitboxSolids;
			break;
		}
		case gd::GameObjectType::kGameObjectTypeModifier:
		case gd::GameObjectType::kGameObjectTypeSpecial:
		case gd::GameObjectType::kGameObjectTypeTeleportPortal:
		case gd::GameObjectType::kGameObjectTypeDropRing:
		case gd::GameObjectType::kGameObjectTypeInverseGravityPortal:
		case gd::GameObjectType::kGameObjectTypeYellowJumpPad:
		case gd::GameObjectType::kGameObjectTypeInverseMirrorPortal:
		case gd::GameObjectType::kGameObjectTypeDualPortal:
		case gd::GameObjectType::kGameObjectTypeNormalGravityPortal:
		case gd::GameObjectType::kGameObjectTypeNormalMirrorPortal:
		case gd::GameObjectType::kGameObjectTypeSoloPortal:
		case gd::GameObjectType::kGameObjectTypePinkJumpPad:
		case gd::GameObjectType::kGameObjectTypeGravityPad:
		case gd::GameObjectType::kGameObjectTypeRedJumpPad:
		case gd::GameObjectType::kGameObjectTypeRegularSizePortal:
		case gd::GameObjectType::kGameObjectTypeMiniSizePortal:
		case gd::GameObjectType::kGameObjectTypeCubePortal:
		case gd::GameObjectType::kGameObjectTypeShipPortal:
		case gd::GameObjectType::kGameObjectTypeBallPortal:
		case gd::GameObjectType::kGameObjectTypeUfoPortal:
		case gd::GameObjectType::kGameObjectTypeWavePortal:
		case gd::GameObjectType::kGameObjectTypeRobotPortal:
		case gd::GameObjectType::kGameObjectTypeSpiderPortal:
		case gd::GameObjectType::kGameObjectTypeYellowJumpRing:
		case gd::GameObjectType::kGameObjectTypePinkJumpRing:
		case gd::GameObjectType::kGameObjectTypeGravityRing:
		case gd::GameObjectType::kGameObjectTypeRedJumpRing:
		case gd::GameObjectType::kGameObjectTypeGreenRing:
		case gd::GameObjectType::kGameObjectTypeDashRing:
		case gd::GameObjectType::kGameObjectTypeGravityDashRing:
		case gd::GameObjectType::kGameObjectTypeSecretCoin:
		case gd::GameObjectType::kGameObjectTypeUserCoin:
		case gd::GameObjectType::kGameObjectTypeCustomRing:
		case gd::GameObjectType::kGameObjectTypeCollectible:
		{
			w *= hacks.hitboxSpecial;
			h *= hacks.hitboxSpecial;
			break;
		}
		case gd::GameObjectType::kGameObjectTypeHazard:
		{
			if ((reinterpret_cast<gd::GameObject *>(obj))->getObjectRadius() <= 0)
			{
				w *= hacks.hitboxMultiplier;
				h *= hacks.hitboxMultiplier;
			}
			else if (!(reinterpret_cast<gd::GameObject *>(obj))->m_unk368)
			{
				(reinterpret_cast<gd::GameObject *>(obj))->m_objectRadius *= hacks.hitboxMultiplier;
				(reinterpret_cast<gd::GameObject *>(obj))->m_unk368 = true;
			}
			break;
		}
		default:
			break;
		}
	}
	return PlayLayer::getObjectRect(obj, self, w, h);
}

void *__fastcall PlayLayer::getObjectRectHook2(cocos2d::CCNode *obj, void *, float w, float h)
{
	if (hacks.enableHitboxMultiplier && !EndLevelLayer::nameChildFind(obj, "PlayerObject"))
	{
		switch ((reinterpret_cast<gd::GameObject *>(obj))->getType())
		{
		case gd::GameObjectType::kGameObjectTypeSolid:
		case gd::GameObjectType::kGameObjectTypeSlope:
		{
			w *= hacks.hitboxSolids;
			h *= hacks.hitboxSolids;
			break;
		}
		case gd::GameObjectType::kGameObjectTypeModifier:
		case gd::GameObjectType::kGameObjectTypeSpecial:
		case gd::GameObjectType::kGameObjectTypeTeleportPortal:
		case gd::GameObjectType::kGameObjectTypeDropRing:
		case gd::GameObjectType::kGameObjectTypeInverseGravityPortal:
		case gd::GameObjectType::kGameObjectTypeYellowJumpPad:
		case gd::GameObjectType::kGameObjectTypeInverseMirrorPortal:
		case gd::GameObjectType::kGameObjectTypeDualPortal:
		case gd::GameObjectType::kGameObjectTypeNormalGravityPortal:
		case gd::GameObjectType::kGameObjectTypeNormalMirrorPortal:
		case gd::GameObjectType::kGameObjectTypeSoloPortal:
		case gd::GameObjectType::kGameObjectTypePinkJumpPad:
		case gd::GameObjectType::kGameObjectTypeGravityPad:
		case gd::GameObjectType::kGameObjectTypeRedJumpPad:
		case gd::GameObjectType::kGameObjectTypeRegularSizePortal:
		case gd::GameObjectType::kGameObjectTypeMiniSizePortal:
		case gd::GameObjectType::kGameObjectTypeCubePortal:
		case gd::GameObjectType::kGameObjectTypeShipPortal:
		case gd::GameObjectType::kGameObjectTypeBallPortal:
		case gd::GameObjectType::kGameObjectTypeUfoPortal:
		case gd::GameObjectType::kGameObjectTypeWavePortal:
		case gd::GameObjectType::kGameObjectTypeRobotPortal:
		case gd::GameObjectType::kGameObjectTypeSpiderPortal:
		case gd::GameObjectType::kGameObjectTypeYellowJumpRing:
		case gd::GameObjectType::kGameObjectTypePinkJumpRing:
		case gd::GameObjectType::kGameObjectTypeGravityRing:
		case gd::GameObjectType::kGameObjectTypeRedJumpRing:
		case gd::GameObjectType::kGameObjectTypeGreenRing:
		case gd::GameObjectType::kGameObjectTypeDashRing:
		case gd::GameObjectType::kGameObjectTypeGravityDashRing:
		case gd::GameObjectType::kGameObjectTypeSecretCoin:
		case gd::GameObjectType::kGameObjectTypeUserCoin:
		case gd::GameObjectType::kGameObjectTypeCustomRing:
		case gd::GameObjectType::kGameObjectTypeCollectible:
		{
			w *= hacks.hitboxSpecial;
			h *= hacks.hitboxSpecial;
			break;
		}
		case gd::GameObjectType::kGameObjectTypeHazard:
		{
			if ((reinterpret_cast<gd::GameObject *>(obj))->getObjectRadius() <= 0)
			{
				w *= hacks.hitboxMultiplier;
				h *= hacks.hitboxMultiplier;
			}
			else if (!(reinterpret_cast<gd::GameObject *>(obj))->m_unk368)
			{
				(reinterpret_cast<gd::GameObject *>(obj))->m_objectRadius *= hacks.hitboxMultiplier;
				(reinterpret_cast<gd::GameObject *>(obj))->m_unk368 = true;
			}
			break;
		}
		default:
			break;
		}
	}
	return PlayLayer::getObjectRect2(obj, w, h);
}

void PlayLayer::Quit()
{
	PlayLayer::onQuit(playlayer);
	playlayer = nullptr;
	startPosText = nullptr;
	Hacks::MenuMusic();
	drawer->clearQueue();
	if (Hacks::ds.core && hacks.discordRPC)
	{
		discord::Activity activity{};
		activity.SetState("Browsing Menus");
		activity.GetAssets().SetLargeImage("cool");
		activity.GetAssets().SetLargeText(gd::GameManager::sharedState()->m_sPlayerName.c_str());
		activity.GetTimestamps().SetStart(Hacks::ds.timeStart);
		activity.SetType(discord::ActivityType::Playing);
		Hacks::ds.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}
}

bool __fastcall PlayLayer::editorInitHook(gd::LevelEditorLayer *self, void *, gd::GJGameLevel *lvl)
{
	playlayer = nullptr;
	startPosText = nullptr;
	if (drawer)
		drawer->clearQueue();
	bool res = PlayLayer::editorInit(self, lvl);
	self->getObjectLayer()->addChild(HitboxNode::getInstance(), 32);
	if (Hacks::ds.core && hacks.discordRPC)
	{
		discord::Activity activity{};
		activity.SetState(("Editing " + lvl->levelName).c_str());
		activity.GetAssets().SetLargeImage("cool");
		activity.GetAssets().SetLargeText(gd::GameManager::sharedState()->m_sPlayerName.c_str());
		activity.GetTimestamps().SetStart(Hacks::ds.timeStart);
		activity.SetType(discord::ActivityType::Playing);
		activity.GetAssets().SetSmallImage("editor");
		Hacks::ds.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}
	return res;
}

void __fastcall PlayLayer::lightningFlashHook(gd::PlayLayer *self, void *edx, CCPoint p, _ccColor3B c)
{
	if (!hacks.layoutMode)
		PlayLayer::lightningFlash(self, p, c);
}

void __fastcall PlayLayer::togglePlayerScaleHook(gd::PlayerObject *self, void *, bool toggle)
{
	if (simulating)
		return;
	if (drawer && playlayer)
	{
		self == playlayer->m_pPlayer1 ? drawer->m_isMini1 = toggle : drawer->m_isMini2 = toggle;
	}
	PlayLayer::togglePlayerScale(self, toggle);
}

void __fastcall PlayLayer::ringJumpHook(gd::PlayerObject *self, void *, gd::GameObject *ring)
{
	if (simulating)
		return;
	bool a = ring->m_bHasBeenActivated;
	bool b = ring->m_bHasBeenActivatedP2;
	PlayLayer::ringJump(self, ring);
	if (replayPlayer)
		replayPlayer->HandleActivatedObjects(a, b, ring);
}

void __fastcall PlayLayer::activateObjectHook(gd::GameObject *self, void *, gd::PlayerObject *player)
{
	if (simulating)
		return;
	bool a = self->m_bHasBeenActivated;
	bool b = self->m_bHasBeenActivatedP2;
	PlayLayer::activateObject(self, player);
	if (replayPlayer)
		replayPlayer->HandleActivatedObjects(a, b, self);
}

void __fastcall PlayLayer::bumpHook(gd::GJBaseGameLayer *self, void *, gd::PlayerObject *player, gd::GameObject *object)
{
	if (simulating)
		return;
	bool a = object->m_bHasBeenActivated;
	bool b = object->m_bHasBeenActivatedP2;
	PlayLayer::bump(self, player, object);
	if (replayPlayer)
		replayPlayer->HandleActivatedObjects(a, b, object);
}

void __fastcall PlayLayer::newBestHook(gd::PlayLayer *self, void *, bool b1, int i1, int i2, bool b2, bool b3, bool b4)
{
	PlayLayer::newBest(self, b1, i1, i2, b2, b3, b4);

	if (Hacks::ds.core && hacks.discordRPC)
	{
		discord::Activity activity{};
		activity.SetDetails(("Playing " + self->m_level->levelName).c_str());
		if (self->m_level->userName != "")
			activity.SetState(("by " + self->m_level->userName + " (" + std::to_string(self->m_level->normalPercent) + "%)").c_str());
		else
			activity.SetState((std::to_string(self->m_level->normalPercent) + "%").c_str());
		activity.GetAssets().SetLargeImage("cool");
		activity.GetAssets().SetLargeText(gd::GameManager::sharedState()->m_sPlayerName.c_str());
		activity.GetTimestamps().SetStart(Hacks::ds.timeStart);
		activity.SetType(discord::ActivityType::Playing);
		activity.GetAssets().SetSmallImage(Hacks::getDifficultyName(*self->m_level).c_str());
		Hacks::ds.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}
}

void __fastcall PlayLayer::checkCollisionsHook(gd::PlayLayer *self, void *, gd::PlayerObject *player)
{
	PlayLayer::checkCollisions(self, player);
}

void __fastcall PlayLayer::dispatchKeyboardMSGHook(void *self, void *, int key, bool down)
{
	dispatchKeyboardMSG(self, key, down);
	static int debugNum = 0;
	if (key == 'L' && down)
	{
		debugNum++;
		if (debugNum == 10)
		{
			debug.enabled = true;
		}
		else if (debugNum == 11)
		{
			debug.enabled = false;
			debugNum = 9;
		}
	}

	for (size_t i = 0; i < Shortcuts::shortcuts.size(); i++)
	{
		if (key == Shortcuts::shortcuts[i].key && down)
		{
			// this is an horrible way to do it but i cant think of any other solution
			bool prev = Hacks::show;

			if (!prev)
			{
				ImGui::GetStyle().Alpha = 0;
				Hacks::show = true;
				Hacks::fake = true;
			}
			Hacks::hackName = Shortcuts::shortcuts[i].name;
			if (!prev)
			{
				Hacks::RenderMain();
				Hacks::show = false;
				Hacks::fake = false;
				ImGui::GetStyle().Alpha = 1;
			}
		}
	}

	if (!playlayer)
		return;

	if (key == hacks.stepIndex)
	{
		if (down)
		{
			if (!hacks.holdAdvance)
				steps = hacks.stepCount;
			else
				holdingAdvance = true;
		}
		else
			holdingAdvance = false;
	}

	if (!hacks.startPosSwitcher || sp.size() <= 0)
		return;

	if (down && key == VK_RIGHT)
	{
		if (startPosIndex < (int)sp.size() - 1)
		{
			startPosIndex++;
			Change();
		}
		else
		{
			startPosIndex = -1;
			Change();
		}
	}
	else if (down && key == VK_LEFT)
	{
		if (startPosIndex >= 0)
		{
			startPosIndex--;
			Change();
		}
		else
		{
			startPosIndex = (int)sp.size() - 1;
			Change();
		}
	}
}