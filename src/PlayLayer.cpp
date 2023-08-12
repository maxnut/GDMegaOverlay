#include "PlayLayer.h"
#include "EndLevelLayer.h"
#include "ExitAlert.h"
#include "Hacks.h"
#include "HitboxNode.hpp"
#include "ReplayPlayer.h"
#include "Shortcuts.h"
#include "TrajectorySimulation.h"
#include "bools.h"
#include "fmod_dsp_effects.h"
#include <chrono>
#include <cmath>
#include <ctime>
#include <imgui.h>
#include <iomanip>
#include <regex>
#include <sstream>
#include <vector>

extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

PlayLayer::fCCApplication_toggleVerticalSync PlayLayer::CCApplication_toggleVerticalSync =
	(PlayLayer::fCCApplication_toggleVerticalSync)GetProcAddress(GetModuleHandleA("libcocos2d.dll"),
																 "?toggleVerticalSync@CCApplication@cocos2d@@QAEX_N@Z");

ExitAlert a;

bool PlayLayer::hadAction = false, PlayLayer::wasPaused = false;
int PlayLayer::respawnAction = 0, PlayLayer::respawnAction2 = 0;
float PlayLayer::player1RotRate = 0, PlayLayer::player2RotRate = 0;

HitboxNode* drawer;
gd::PlayLayer* playlayer;

float percent, startPercent = 0, endPercent = 0, maxRun = 0, delta = -1, prevP = 0, pressTimer = 0, releaseTimer = 0,
			   opacity = 0, p = 1;

std::string bestRun = "Best Run: none", text;

std::vector<float> clicksArr;
std::vector<gd::StartPosObject*> sp;
std::vector<gd::GameObject*> gravityPortals, dualPortals, gamemodePortals, miniPortals, speedChanges, mirrorPortals;
std::vector<bool> willFlip;

bool add = false, pressing, dead = false, hitboxDead = false, clickType = true, pressed = false, reset = true;

ccColor3B iconCol, secIconCol;

int totalClicks, startPosIndex, noClipDeaths, actualDeaths, frames, bestRunRepeat;
unsigned int songLength;
int hackAmts[5];

CCNode* statuses[STATUSSIZE];
CCLabelBMFont *startPosText, *macroText;
CCScheduler* scheduler;
CCSize size, pixelSize;
CCSprite* noclipRed;

std::vector<gd::GameObject*> coinPos;

ReplayPlayer* replayPlayer;

std::chrono::time_point<std::chrono::high_resolution_clock> previous_frame, last_update;
float avg = 0.f;
int frame_count = 0;

FMOD::Sound* accuracySound;
FMOD::DSP* pitchShifterDSP = nullptr;

std::deque<Checkpoint> backFrames;
bool steppingBack = false;

int cubeIcon, shipIcon, ballIcon, ufoIcon, waveIcon, robotIcon;

void UpdateLabels(gd::PlayLayer* self);
void Change();

void SetupLabel(gd::PlayLayer* self, int index)
{
	auto st = static_cast<CCLabelBMFont*>(statuses[index]);
	st = CCLabelBMFont::create("", "bigFont.fnt");
	st->setZOrder(1000);
	st->setScale(0.5f);
	st->setOpacity(150.0f);
	self->m_uiLayer->addChild(st);
	statuses[index] = st;
}

void SmartStartPosSetup(gd::PlayLayer* self)
{
	willFlip.resize(sp.size());
	size_t index = 0;
	for (gd::StartPosObject* startPos : sp)
	{
		for (size_t i = 0; i < gravityPortals.size(); i++)
		{
			if (gravityPortals[i]->getPositionX() - 10 > startPos->getPositionX())
				break;
			if (gravityPortals[i]->getPositionX() - 10 < startPos->getPositionX())
				willFlip[index] = gravityPortals[i]->m_nObjectID == 11;
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
		index++;
	}
}

bool __fastcall PlayLayer::initHook(gd::PlayLayer* self, void*, gd::GJGameLevel* level)
{
	drawer = HitboxNode::getInstance();

	bool result = PlayLayer::init(self, level);

	playlayer = self;

	Hacks::AnticheatBypass();

	auto director = CCDirector::sharedDirector();
	size = director->getWinSize();
	pixelSize = director->getWinSizeInPixels();
	gd::FMODAudioEngine::sharedEngine()->m_pSystem->createSound("Resources/counter003.ogg", FMOD_LOOP_OFF, 0,
																&accuracySound);

	TrajectorySimulation::getInstance()->createSimulation();

	sp.clear();
	gamemodePortals.clear();
	mirrorPortals.clear();
	miniPortals.clear();
	dualPortals.clear();
	speedChanges.clear();
	gravityPortals.clear();
	willFlip.clear();
	hitboxDead = false;
	CCObject* obje;

	Hacks::cheatCheck.clear();

	for (size_t i = 0; i < ExternData::bypass["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(
			std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(),
					   ExternData::bypass["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < ExternData::creator["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(
			std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(),
					   ExternData::creator["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < ExternData::global["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(
			std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(),
					   ExternData::global["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < ExternData::level["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(
			std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(),
					   ExternData::level["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	for (size_t i = 0; i < ExternData::player["mods"].size(); i++)
	{
		Hacks::cheatCheck.push_back(
			std::count(Hacks::cheatVector.begin(), Hacks::cheatVector.end(),
					   ExternData::player["mods"][i]["opcodes"][0]["address"].get<std::string>()));
	}

	CCARRAY_FOREACH(self->m_pObjects, obje)
	{
		auto g = reinterpret_cast<gd::GameObject*>(obje);

		if (g->m_nObjectID == 31)
			sp.push_back(reinterpret_cast<gd::StartPosObject*>(obje));

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

	Hacks::UpdateRichPresence(0, 0, "none");

	if (replayPlayer)
		ExternData::isCheating = PlayLayer::IsCheating();

	return result;
}

bool PlayLayer::isBot = false;

bool __fastcall PlayLayer::pushButtonHook(gd::PlayerObject* self, void*, int PlayerButton)
{
	if (playlayer && !playlayer->m_hasCompletedLevel && replayPlayer && replayPlayer->IsRecording())
	{
		// if(hadAction) return true;
		if (self == playlayer->m_pPlayer2 &&
				(playlayer->m_pLevelSettings->m_twoPlayerMode || hacks.recordPosForPlayer2) ||
			self == playlayer->m_pPlayer1)
		{
			replayPlayer->RecordAction(true, self, self == playlayer->m_pPlayer1);
		}
	}
	else if (playlayer && replayPlayer && replayPlayer->IsPlaying() && hacks.preventInput && !PlayLayer::isBot)
		return true;

	add = true;
	pressing = true;
	hadAction = true;

	return PlayLayer::pushButton(self, PlayerButton);
}

bool __fastcall PlayLayer::releaseButtonHook(gd::PlayerObject* self, void*, int PlayerButton)
{
	if (playlayer && !playlayer->m_hasCompletedLevel && replayPlayer && replayPlayer->IsRecording())
	{
		if (self == playlayer->m_pPlayer2 && delta > 0 && playlayer->m_bIsDualMode &&
				(playlayer->m_pLevelSettings->m_twoPlayerMode || hacks.recordPosForPlayer2) ||
			self == playlayer->m_pPlayer1)
		{
			replayPlayer->RecordAction(false, self, self == playlayer->m_pPlayer1);
		}
	}
	else if (playlayer && replayPlayer && replayPlayer->IsPlaying() && hacks.preventInput && !PlayLayer::isBot)
		return true;

	pressing = false;
	hadAction = true;

	return PlayLayer::releaseButton(self, PlayerButton);
}

std::string oldRun = "";

void __fastcall PlayLayer::destroyPlayer_H(gd::PlayLayer* self, void*, gd::PlayerObject* player, gd::GameObject* obj)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooksWithPlayer(player))
	{
		TrajectorySimulation::getInstance()->m_pDieInSimulation = true;
		return;
	}
	if (delta > 0.2f && !ExternData::player["mods"][0]["toggle"] && !ExternData::player["mods"][2]["toggle"] &&
		!self->m_isDead)
	{
		float run = ((player->getPositionX() / self->m_levelLength) * 100.0f) - startPercent;
		endPercent = (player->getPositionX() / self->m_levelLength) * 100.0f;
		std::string runStr = (int)startPercent != 0 ? "Best Run: " + std::to_string((int)startPercent) + "-" +
														  std::to_string((int)endPercent)
													: "Best Run: " + std::to_string((int)endPercent) + "%";
		if ((int)startPercent != (int)endPercent && run > maxRun)
		{
			bestRunRepeat = 0;
			maxRun = run;
			bestRun.clear();
			bestRun = "";
			bestRun = runStr;
			oldRun = runStr;
			Hacks::UpdateRichPresence(0, 0,
									  (std::to_string((int)startPercent)) + "-" + std::to_string((int)endPercent));
		}
		else if (runStr == oldRun && hacks.accumulateRuns)
		{
			bestRunRepeat++;
			if (bestRunRepeat > 1)
			{
				bestRun = std::regex_replace(bestRun, std::regex("x" + std::to_string(bestRunRepeat)), "");
			}
			bestRun += "x" + std::to_string(bestRunRepeat + 1);
		}
	}

	if (player->m_isBall)
		player->stopActionByTag(1);

	PlayLayer::destroyPlayer(self, player, obj);
}

gd::GameSoundManager* __fastcall PlayLayer::levelCompleteHook(gd::PlayLayer* self)
{
	float run = 100.0f - startPercent;
	endPercent = 100.0f;
	std::string runStr = (int)startPercent != 0
							 ? "Best Run: " + std::to_string((int)startPercent) + "-" + std::to_string((int)endPercent)
							 : "Best Run: " + std::to_string((int)endPercent) + "%";
	if (run > maxRun)
	{
		bestRunRepeat = 0;
		maxRun = run;
		bestRun.clear();
		bestRun = "";
		bestRun = runStr;
		oldRun = runStr;
		Hacks::UpdateRichPresence(0, 0, (std::to_string((int)startPercent)) + "-" + std::to_string((int)endPercent));
	}
	else if (runStr == oldRun && hacks.accumulateRuns)
	{
		bestRunRepeat++;
		if (bestRunRepeat > 1)
		{
			bestRun.pop_back();
			bestRun.pop_back();
		}
		bestRun += "x" + std::to_string(bestRunRepeat + 1);
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

void __fastcall PlayLayer::hkDeath(void* self, void*, void* go, void* powerrangers)
{
	if (delta > 0.1f)
	{
		dead = true;
		hitboxDead = true;
	}

	PlayLayer::death(self, go, powerrangers);
}

bool PlayLayer::IsCheating()
{
	if (!playlayer)
		return false;
	size_t total = 0;
	for (size_t i = 0; i < ExternData::bypass["mods"].size(); i++)
	{
		if (ExternData::bypass["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < ExternData::creator["mods"].size(); i++)
	{
		if (ExternData::creator["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < ExternData::global["mods"].size(); i++)
	{
		if (ExternData::global["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < ExternData::level["mods"].size(); i++)
	{
		if (ExternData::level["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	for (size_t i = 0; i < ExternData::player["mods"].size(); i++)
	{
		if (ExternData::player["mods"][i]["toggle"].get<bool>() && Hacks::cheatCheck[total])
			return true;
		total++;
	}

	if (hacks.fps > 360.0f || hacks.screenFPS > 360.0f || hacks.tpsBypass > 360.0f || hacks.autoclicker ||
		hacks.frameStep || scheduler->getTimeScale() != 1 || replayPlayer->IsPlaying() || hacks.layoutMode ||
		hacks.enableHitboxMultiplier || hacks.showHitboxes && !hacks.onlyOnDeath || hacks.hidePause ||
		hacks.trajectory || hacks.waveSize > 2.1f)
		return true;

	return false;
}

void UpdateLabels(gd::PlayLayer* self)
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

	auto fontPtr = static_cast<CCLabelBMFont*>(statuses[0]);

	float r, g, b;
	if (labels.rainbowLabels)
		ImGui::ColorConvertHSVtoRGB(ImGui::GetTime() * labels.rainbowSpeed, 1, 1, r, g, b);
	const ccColor3B rainbow = {r * 255, g * 255, b * 255};

	if (labels.statuses[0] && scheduler)
	{
		fontPtr->setString(".");
		fontPtr->setOpacity(labels.opacity[0]);
		if (ExternData::isCheating)
			fontPtr->setColor(red);
		else if (ExternData::level["mods"][24]["toggle"])
			fontPtr->setColor(yellow);
		else
			fontPtr->setColor(green);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont*>(statuses[1]);

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

			fontPtr->setString(
				CCString::createWithFormat(labels.styles[0], ImGui::GetIO().Framerate, fps)->getCString());
		}

		previous_frame = now;

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont*>(statuses[2]);

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
		fontPtr->setColor(pressing ? pressed : labels.rainbowLabels ? rainbow : white);
	}
	else
	{
		fontPtr->setString("");
		clicksArr.clear();
	}

	fontPtr = static_cast<CCLabelBMFont*>(statuses[3]);

	if (labels.statuses[3])
	{
		fontPtr->setString(text.c_str());
		fontPtr->setColor(dead ? red : labels.rainbowLabels ? rainbow : white);

		if (hacks.noClipAccuracyLimit > 0 && p * 100.0f < hacks.noClipAccuracyLimit)
		{
			p = 1;
			bool t = ExternData::player["mods"][0]["toggle"];
			ExternData::player["mods"][0]["toggle"] = false;
			Hacks::ToggleJSONHack(ExternData::player, 0, false);
			ExternData::player["mods"][0]["toggle"] = t;
		}
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont*>(statuses[4]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[5]);

	if (labels.statuses[5])
	{
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		std::ostringstream s;
		if (!hacks.use12hFormat)
		{
			s << std::put_time(&tm, "%H:%M:%S");
		}
		else
		{
			s << std::put_time(&tm, "%I:%M:%S %p");
		}

		fontPtr->setString(s.str().c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont*>(statuses[6]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[7]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[8]);

	if (labels.statuses[8] && !hacks.onlyInRuns ||
		labels.statuses[8] && (self->m_isPracticeMode || self->m_isTestMode) && hacks.onlyInRuns)
	{
		fontPtr->setString(("From " + std::to_string((int)startPercent) + "%").c_str());

		if (labels.rainbowLabels)
			fontPtr->setColor(rainbow);
		else
			fontPtr->setColor(white);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont*>(statuses[9]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[10]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[11]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[12]);

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

	fontPtr = static_cast<CCLabelBMFont*>(statuses[13]);

	if (labels.statuses[13])
	{
		if (!self->m_hasCompletedLevel && replayPlayer)
			fontPtr->setString(("Frame " + std::to_string(replayPlayer->GetFrame())).c_str());

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
		{
			if (hacks.replayMode < 2)
				macroText->setString(("Recording: " + std::to_string(replayPlayer->GetActionsSize())).c_str());
			else
				macroText->setString(("Recording: " + std::to_string(replayPlayer->GetActionsSize()) + " | " +
									  std::to_string(replayPlayer->GetFrameCapturesSize()))
										 .c_str());
		}
		else if (replayPlayer->IsPlaying())
			if (hacks.replayMode < 2)
				macroText->setString(("Playing: " + std::to_string(replayPlayer->GetActionIndex()) + "/" +
									  std::to_string(replayPlayer->GetActionsSize()))
										 .c_str());
			else
				macroText->setString(("Playing: " + std::to_string(replayPlayer->GetActionIndex()) + "/" +
									  std::to_string(replayPlayer->GetActionsSize()) + " | " +
									  std::to_string(replayPlayer->GetCapturesIndex()) + "/" +
									  std::to_string(replayPlayer->GetFrameCapturesSize()))
										 .c_str());
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

	if (hacks.realPercentage)
	{
		float percent = (self->m_pPlayer1->getPositionX() / self->m_levelLength) * 100.0f;
		self->m_percentLabel->setString(
			CCString::createWithFormat("%.2f%%", (percent * 100.0f) / hacks.levelEndPercent)->getCString());
	}
}

const char* actualFonts[] = {"bigFont.fnt",	 "chatFont.fnt", "gjFont01.fnt", "gjFont02.fnt", "gjFont03.fnt",
							 "gjFont04.fnt", "gjFont05.fnt", "gjFont06.fnt", "gjFont07.fnt", "gjFont08.fnt",
							 "gjFont09.fnt", "gjFont10.fnt", "gjFont11.fnt", "goldFont.fnt"};

void PlayLayer::UpdatePositions(int index)
{
	if (playlayer == nullptr)
		return;

	auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();

	auto ptr = static_cast<CCLabelBMFont*>(statuses[index]);

	float tr = 0, tl = 0, br = 0, bl = 0, thisLabel;

	for (size_t i = 0; i < STATUSSIZE; i++)
	{
		if (!labels.statuses[i] ||
			i == 8 && hacks.onlyInRuns && !(playlayer->m_isTestMode || playlayer->m_isPracticeMode))
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
		statuses[index]->setPosition(x, height);
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
	statuses[index]->setAnchorPoint(statuses[index]->getPositionX() > 284.5f
										? CCPointMake(1.0f, statuses[index]->getAnchorPoint().y)
										: CCPointMake(0.0f, statuses[index]->getAnchorPoint().y));
}

void PlayLayer::SyncMusic()
{
	if (!playlayer || replayPlayer->recorder.m_recording ||
		reinterpret_cast<bool(__thiscall*)(gd::PlayerObject*)>(gd::base + 0x1f6820)(playlayer->m_pPlayer1))
		return;
	float f = playlayer->timeForXPos(playlayer->m_pPlayer1->getPositionX());
	unsigned int p;
	float offset = playlayer->m_pLevelSettings->m_songStartOffset * 1000;

	auto engine = gd::FMODAudioEngine::sharedEngine();

	engine->m_pGlobalChannel->getPosition(&p, FMOD_TIMEUNIT_MS);
	if (std::abs((int)(f * 1000) - (int)p + offset) > hacks.musicMaxDesync && !playlayer->m_hasCompletedLevel)
	{
		gd::FMODAudioEngine::sharedEngine()->m_pGlobalChannel->setPosition(
			static_cast<uint32_t>(f * 1000) + static_cast<uint32_t>(offset), FMOD_TIMEUNIT_MS);
	}
}

bool lastFrameDead = false;

void Update(gd::PlayLayer* self, float dt)
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
			if (hacks.accuracySound && labels.statuses[3])
				gd::FMODAudioEngine::sharedEngine()->m_pSystem->playSound(
					accuracySound, 0, false, &gd::FMODAudioEngine::sharedEngine()->m_pCurrentSoundChannel);
		}

		lastFrameDead = dead;

		if (dead && !self->m_hasCompletedLevel && ExternData::player["mods"][0]["toggle"] ||
			dead && !self->m_hasCompletedLevel && ExternData::player["mods"][2]["toggle"])
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

	if (ExternData::isCheating != (bool)ExternData::level["mods"][24]["toggle"] && hacks.autoSafeMode)
	{
		ExternData::level["mods"][24]["toggle"] = ExternData::isCheating;
		Hacks::ToggleJSONHack(ExternData::level, 24, false);
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

	if (hacks.enableWaveTrailColor)
	{
		const ccColor3B col = {hacks.waveTrailColor[0] * 255.f, hacks.waveTrailColor[1] * 255.f,
							   hacks.waveTrailColor[2] * 255.f};
		reinterpret_cast<CCNodeRGBA*>(self->m_pPlayer1->m_waveTrail)->setColor(col);
		reinterpret_cast<CCNodeRGBA*>(self->m_pPlayer2->m_waveTrail)->setColor(col);
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
		}
		if (hacks.rainbowPlayerC2)
		{
			self->m_pPlayer1->setSecondColor(col);
			self->m_pPlayer2->setSecondColor(col);
		}
		if (hacks.rainbowWaveTrail)
		{
			reinterpret_cast<CCNodeRGBA*>(self->m_pPlayer1->m_waveTrail)->setColor(col);
			reinterpret_cast<CCNodeRGBA*>(self->m_pPlayer2->m_waveTrail)->setColor(col);
		}

		if (hacks.rainbowOutline)
		{
			self->m_pPlayer1->m_iconGlow->setColor(col);
			self->m_pPlayer2->m_iconGlow->setColor(col);
			self->m_pPlayer1->m_vehicleGlow->setChildColor(col);
			self->m_pPlayer2->m_vehicleGlow->setChildColor(col);
		}
	}

	if (hacks.lockCursor && !ExternData::show && !self->m_hasCompletedLevel && !self->m_isDead)
		SetCursorPos(pixelSize.width / 2, pixelSize.height / 2);

	dead = false;

	if (replayPlayer)
	{
		replayPlayer->Update(self);
	}

	if (replayPlayer->recorder.m_recording)
		replayPlayer->recorder.handle_recording(self, dt);

	float xp = self->m_pPlayer1->getPositionX();

	static float prevRot1;
	static float prevRot2;

	if (prevRot1 != self->m_pPlayer1->getRotation())
	{
		PlayLayer::player1RotRate = self->m_pPlayer1->getRotation() - prevRot1;
		prevRot1 = self->m_pPlayer1->getRotation();
	}

	if (prevRot2 != self->m_pPlayer2->getRotation())
	{
		PlayLayer::player2RotRate = self->m_pPlayer2->getRotation() - prevRot2;
		prevRot2 = self->m_pPlayer2->getRotation();
	}

	if (hacks.frameStep && !steppingBack)
	{
		Checkpoint check;
		check.p1 = CheckpointData::fromPlayer(self->m_pPlayer1);
		if (self->m_bIsDualMode)
			check.p2 = CheckpointData::fromPlayer(self->m_pPlayer2);

		backFrames.push_back(check);
		if (backFrames.size() > hacks.backStepMax)
			backFrames.pop_back();
	}

	steppingBack = false;

	PlayLayer::update(self, dt);

	UpdateLabels(self);

	if (hacks.trajectory)
		TrajectorySimulation::getInstance()->processMainSimulation(dt);

	if (self->m_pPlayer1->m_waveTrail && hacks.solidWavePulse)
		self->m_pPlayer1->m_waveTrail->m_pulseSize = hacks.waveSize;
	if (self->m_pPlayer2->m_waveTrail && hacks.solidWavePulse)
		self->m_pPlayer2->m_waveTrail->m_pulseSize = hacks.waveSize;

	if (hacks.layoutMode)
	{
		bool changeBlockColor = false;
		auto p = self->getChildren()->objectAtIndex(0);
		if (frames > 10)
		{
			auto sprite = static_cast<CCSprite*>(p);
			ccColor3B color = {(GLubyte)(hacks.backgroundColor[0] * 255), (GLubyte)(hacks.backgroundColor[1] * 255),
							   (GLubyte)(hacks.backgroundColor[2] * 255)};
			sprite->setColor(color);
		}

		if (hacks.blocksColor[0] != hacks.backgroundColor[0] || hacks.blocksColor[1] != hacks.backgroundColor[1] ||
			hacks.blocksColor[2] != hacks.backgroundColor[2])
		{
			changeBlockColor = true;
		}

		for (int s = self->sectionForPos(xp) - 5; s < self->sectionForPos(xp) + 6; ++s)
		{
			if (s < 0)
				continue;
			if (s >= self->m_sectionObjects->count())
				break;
			auto section = static_cast<CCArray*>(self->m_sectionObjects->objectAtIndex(s));
			for (size_t i = 0; i < section->count(); ++i)
			{
				auto o = static_cast<gd::GameObject*>(section->objectAtIndex(i));

				if (changeBlockColor && o->getType() == gd::GameObjectType::kGameObjectTypeSolid)
				{
					auto block = static_cast<gd::GameObject*>(o);
					ccColor3B blockColor = {(GLubyte)(hacks.blocksColor[0] * 255),
											(GLubyte)(hacks.blocksColor[1] * 255),
											(GLubyte)(hacks.blocksColor[2] * 255)};
					block->setColor(blockColor);
				}

				if (o->getType() == gd::GameObjectType::kGameObjectTypeDecoration && o->isVisible() &&
					(o->m_nObjectID != 44 && o->m_nObjectID != 749 && o->m_nObjectID != 12 && o->m_nObjectID != 38 &&
					 o->m_nObjectID != 47 && o->m_nObjectID != 111 && o->m_nObjectID != 8 && o->m_nObjectID != 13 &&
					 o->m_nObjectID != 660 && o->m_nObjectID != 745 && o->m_nObjectID != 101 && o->m_nObjectID != 99 &&
					 o->m_nObjectID != 1331))
				{
					o->setVisible(false);
				}
			}
		}
	}

	if (drawer && hacks.showHitboxes && !hacks.onlyOnDeath ||
		drawer && hacks.showHitboxes && hacks.onlyOnDeath && hitboxDead && playlayer->m_isDead)
	{
		drawer->setVisible(true);

		if (hacks.coinTracker)
		{
			for (size_t i = 0; i < coinPos.size(); i++)
			{
				if (coinPos[i] && self->m_pPlayer1->getPositionX() <= coinPos[i]->getPositionX())
					drawer->drawSegment(self->m_pPlayer1->getPosition(), coinPos[i]->getPosition(), 0.5f,
										ccc4f(0, 1, 0, 1));
			}
		}

		if (replayPlayer && replayPlayer->GetActionsSize() > 0)
			drawer->drawCircleHitbox({replayPlayer->GetReplay()->getActions()[replayPlayer->GetActionIndex()].px,
									  replayPlayer->GetReplay()->getActions()[replayPlayer->GetActionIndex()].py},
									 5.0f, {255, 255, 0, 0});

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
			auto section = static_cast<CCArray*>(self->m_sectionObjects->objectAtIndex(s));
			for (size_t i = 0; i < section->count(); ++i)
			{
				auto obj = static_cast<gd::GameObject*>(section->objectAtIndex(i));

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

void __fastcall PlayLayer::pauseGameHook(gd::PlayLayer* self, void*, bool idk)
{
	if (replayPlayer && replayPlayer->IsRecording())
	{
		replayPlayer->RecordAction(false, self->m_pPlayer1, true);
		replayPlayer->RecordAction(false, self->m_pPlayer2, false);
	}
	PlayLayer::pauseGame(self, idk);
}

cocos2d::CCTouch* _touch = nullptr;
cocos2d::CCEvent* _event = nullptr;

void __fastcall PlayLayer::uiOnPauseHook(gd::UILayer* self, void*, CCObject* obj)
{
	wasPaused = true;
	if (hacks.voidClick && _touch)
		self->ccTouchEnded(_touch, _event);
	PlayLayer::uiOnPause(self, obj);
}

void __fastcall PlayLayer::uiTouchBeganHook(gd::UILayer* self, void*, cocos2d::CCTouch* touch, cocos2d::CCEvent* evnt)
{
	_touch = touch;
	_event = evnt;
	PlayLayer::uiTouchBegan(self, touch, evnt);
}

gd::GameObject* __fastcall PlayLayer::powerOffObjectHook(gd::GameObject* self)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return self;
	return PlayLayer::powerOffObject(self);
}

gd::GameObject* __fastcall PlayLayer::playShineEffectHook(gd::GameObject* self)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return self;
	return PlayLayer::playShineEffect(self);
}

gd::GameObject* __fastcall PlayLayer::hasBeenActivatedByPlayerHook(gd::GameObject* self, void*, gd::GameObject* other)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
	{
		if (self->getType() != gd::GameObjectType::kGameObjectTypeSlope &&
			self->getType() != gd::GameObjectType::kGameObjectTypeSolid &&
			self->getType() != gd::GameObjectType::kGameObjectTypeGravityPad &&
			self->getType() != gd::kGameObjectTypePinkJumpPad && self->getType() != gd::kGameObjectTypeRedJumpPad &&
			self->getType() != gd::kGameObjectTypeYellowJumpPad && self->getType() != gd::kGameObjectTypeDashRing &&
			self->getType() != gd::kGameObjectTypeDropRing && self->getType() != gd::kGameObjectTypeGravityDashRing &&
			self->getType() != gd::kGameObjectTypeGravityRing && self->getType() != gd::kGameObjectTypeGreenRing &&
			self->getType() != gd::kGameObjectTypePinkJumpRing && self->getType() != gd::kGameObjectTypeRedJumpRing &&
			self->getType() != gd::kGameObjectTypeYellowJumpRing && self->getType() != gd::kGameObjectTypeSpecial &&
			self->getType() != gd::kGameObjectTypeCollisionObject && self->getType() != gd::kGameObjectTypeHazard &&
			self->getType() != gd::kGameObjectTypeInverseGravityPortal &&
			self->getType() != gd::kGameObjectTypeNormalGravityPortal &&
			self->getType() != gd::kGameObjectTypeTeleportPortal &&
			self->getType() != gd::kGameObjectTypeMiniSizePortal &&
			self->getType() != gd::kGameObjectTypeRegularSizePortal)
		{
			return other;
		}
	}

	return PlayLayer::hasBeenActivatedByPlayer(self, other);
}

void __fastcall PlayLayer::addPointHook(gd::HardStreak* self, void*, CCPoint point)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return;
	PlayLayer::addPoint(self, point);
}

void __fastcall PlayLayer::updateHook(gd::PlayLayer* self, void*, float dt)
{
	Update(self, dt);
	return;
}

void __fastcall PlayLayer::triggerObjectHook(gd::EffectGameObject* self, void*, gd::GJBaseGameLayer* idk)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return;
	auto id = self->m_nObjectID;
	if (hacks.layoutMode && (id == 899 || id == 1006 || id == 1007 || id == 105 || id == 29 || id == 56 || id == 915 ||
							 id == 30 || id == 58))
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

	if (playlayer->m_startPosCheckpoint)
		playlayer->m_startPosCheckpoint->release();
	playlayer->m_startPosCheckpoint = nullptr;

	if (startPosIndex >= 0)
	{
		playlayer->m_isTestMode = true;
		playlayer->m_startPos = sp[startPosIndex];
		playlayer->m_playerStartPosition =
			CCPointMake(sp[startPosIndex]->getPositionX(), sp[startPosIndex]->getPositionY());
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

void __fastcall PlayLayer::playBackgroundMusicHook(gd::FMODAudioEngine* self, void*, char a2, int a3, void* a4, int a5,
												   int a6, int a7, int a8, unsigned int a9)
{
	PlayLayer::playBackgroundMusic(self, a2, a3, a4, a5, a6, a7, a8, a9);

	if (hacks.pitchShift)
		PlayLayer::SetPitch(hacks.pitchShiftAmt);
}

void __fastcall PlayLayer::resetLevelHook(gd::PlayLayer* self, void*)
{
	// reset stuff
	hitboxDead = false;
	totalClicks = 0;
	actualDeaths = 0;
	EndLevelLayer::deaths = 0;
	lastFrameDead = false;
	noClipDeaths = 0;
	delta = 0;
	ExternData::steps = 0;
	pressTimer = 0;
	opacity = 0;
	releaseTimer = hacks.releaseTime;
	clickType = true;
	dead = false;
	frames = 0;
	self->m_attemptJumpCount = 0;
	PlayLayer::player1RotRate = 0;
	PlayLayer::player2RotRate = 0;
	TrajectorySimulation::getInstance()->m_pDieInSimulation = false;
	TrajectorySimulation::getInstance()->m_pIsSimulation = false;
	Hacks::ToggleJSONHack(ExternData::player, 0, false);
	clicksArr.clear();

	if (hacks.autoUpdateRespawn)
		Hacks::WriteRef(gd::base + 0x20A677,
						hacks.respawnTime * CCDirector::sharedDirector()->getScheduler()->getTimeScale());

	if (drawer)
	{
		drawer->clearQueue();
		drawer->m_isMini1 = self->m_pLevelSettings->m_startMini;
		drawer->m_isMini2 = self->m_pLevelSettings->m_startMini;
	}

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
		gd::CheckpointObject* go = reinterpret_cast<gd::CheckpointObject*>(porcodio);
		startPercent = (go->m_player1->m_position.x / self->m_levelLength) * 100.0f;
	}
	else
	{
		startPercent = (self->m_playerStartPosition.x / self->m_levelLength) * 100.0f;
	}

	cubeIcon = Hacks::randomInt(0, 142);
	shipIcon = Hacks::randomInt(1, 51);
	ballIcon = Hacks::randomInt(0, 43);
	ufoIcon = Hacks::randomInt(1, 35);
	waveIcon = Hacks::randomInt(1, 35);
	robotIcon = Hacks::randomInt(1, 26);

	PlayLayer::resetLevel(self);

	auto gm = gd::GameManager::sharedState();

	self->m_pPlayer1->setColor(gm->colorForIdx(Hacks::randomInt(0, 41)));
	self->m_pPlayer1->setSecondColor(gm->colorForIdx(Hacks::randomInt(0, 41)));

	self->m_pPlayer2->setColor(gm->colorForIdx(Hacks::randomInt(0, 41)));
	self->m_pPlayer2->setSecondColor(gm->colorForIdx(Hacks::randomInt(0, 41)));

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

	if (wasPaused)
	{
		releaseButtonHook(self->m_pPlayer1, 0, 0);
		releaseButtonHook(self->m_pPlayer2, 0, 0);
		wasPaused = false;
	}

	if (replayPlayer)
		ExternData::isCheating = PlayLayer::IsCheating();

	if (!hacks.randomIcons)
	{
		bool t = ExternData::player["mods"][38]["toggle"];

		self->m_pPlayer1->setColor(gd::GameManager::sharedState()->colorForIdx(gm->getPlayerColor()));
		self->m_pPlayer1->setSecondColor(gd::GameManager::sharedState()->colorForIdx(gm->getPlayerColor2()));

		self->m_pPlayer2->setColor(gm->colorForIdx(t ? gm->getPlayerColor() : gm->getPlayerColor2()));
		self->m_pPlayer2->setSecondColor(gm->colorForIdx(t ? gm->getPlayerColor2() : gm->getPlayerColor()));
	}
}

void __fastcall PlayLayer::loadFromCheckpointHook(gd::PlayLayer* self, void* edx, gd::CheckpointObject* obj)
{
	// if(debug.debugNumber < self->m_checkpoints->count())
	// 	obj = static_cast<gd::CheckpointObject*>(self->m_checkpoints->objectAtIndex(debug.debugNumber));
	PlayLayer::loadFromCheckpoint(self, obj);
}

void __fastcall PlayLayer::togglePracticeModeHook(gd::PlayLayer* self, void* edx, bool on)
{
	// if (!on)
	// 	Practice::SaveCheckpoints();

	if (percent >= 100.0f && hacks.lastCheckpoint)
	{
		PlayLayer::togglePracticeMode(self, true);
		PlayLayer::resetLevelHook(self, nullptr);
	}
	else
		PlayLayer::togglePracticeMode(self, on);

	// if (on)
	// 	Practice::LoadCheckpoints();
}

void __fastcall PlayLayer::onQuitHook(gd::PlayLayer* self, void*)
{
	if (hacks.confirmQuit && !self->m_hasCompletedLevel)
		gd::FLAlertLayer::create(&a, "Confirm", "No", "Yes", "Do you really want to quit?")->show();
	else
	{
		PlayLayer::Quit();
	}
}

void* __fastcall PlayLayer::getObjectRectHook(cocos2d::CCNode* obj, void*, gd::GameObject* self, float w, float h)
{
	if (hacks.enableHitboxMultiplier && !EndLevelLayer::nameChildFind(obj, "PlayerObject"))
	{
		switch ((reinterpret_cast<gd::GameObject*>(obj))->getType())
		{
		case gd::GameObjectType::kGameObjectTypeSolid:
		case gd::GameObjectType::kGameObjectTypeSlope: {
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
		case gd::GameObjectType::kGameObjectTypeCollectible: {
			w *= hacks.hitboxSpecial;
			h *= hacks.hitboxSpecial;
			break;
		}
		case gd::GameObjectType::kGameObjectTypeHazard: {
			if ((reinterpret_cast<gd::GameObject*>(obj))->getObjectRadius() <= 0)
			{
				w *= hacks.hitboxMultiplier;
				h *= hacks.hitboxMultiplier;
			}
			else if (!(reinterpret_cast<gd::GameObject*>(obj))->m_unk368)
			{
				(reinterpret_cast<gd::GameObject*>(obj))->m_objectRadius *= hacks.hitboxMultiplier;
				(reinterpret_cast<gd::GameObject*>(obj))->m_unk368 = true;
			}
			break;
		}
		default:
			break;
		}
	}
	return PlayLayer::getObjectRect(obj, self, w, h);
}

void* __fastcall PlayLayer::getObjectRectHook2(cocos2d::CCNode* obj, void*, float w, float h)
{
	if (hacks.enableHitboxMultiplier && !EndLevelLayer::nameChildFind(obj, "PlayerObject"))
	{
		switch ((reinterpret_cast<gd::GameObject*>(obj))->getType())
		{
		case gd::GameObjectType::kGameObjectTypeSolid:
		case gd::GameObjectType::kGameObjectTypeSlope: {
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
		case gd::GameObjectType::kGameObjectTypeCollectible: {
			w *= hacks.hitboxSpecial;
			h *= hacks.hitboxSpecial;
			break;
		}
		case gd::GameObjectType::kGameObjectTypeHazard: {
			if ((reinterpret_cast<gd::GameObject*>(obj))->getObjectRadius() <= 0)
			{
				w *= hacks.hitboxMultiplier;
				h *= hacks.hitboxMultiplier;
			}
			else if (!(reinterpret_cast<gd::GameObject*>(obj))->m_unk368)
			{
				(reinterpret_cast<gd::GameObject*>(obj))->m_objectRadius *= hacks.hitboxMultiplier;
				(reinterpret_cast<gd::GameObject*>(obj))->m_unk368 = true;
			}
			break;
		}
		default:
			break;
		}
	}
	return PlayLayer::getObjectRect2(obj, w, h);
}

void __fastcall PlayLayer::incrementJumpsHook(gd::PlayerObject* self, void*)
{
	PlayLayer::incrementJumps(self);

	if (playlayer)
	{
		if (playlayer->m_pPlayer1 == self)
			ExternData::lastJumpRotP1 = self->getRotation();
		else if (playlayer->m_pPlayer2 == self)
			ExternData::lastJumpRotP2 = self->getRotation();

		if (hacks.trajectory && CheckpointData::GetGamemode(self) == gd::Gamemode::kGamemodeCube && !self->m_isDashing)
		{
			self->stopActionByTag(0);
			TrajectorySimulation::getInstance()->rotateAction = runNormalRotation(self, 1);
			self->runAction(TrajectorySimulation::getInstance()->rotateAction);
		}
	}
}

void PlayLayer::Quit()
{
	if (!std::filesystem::exists("GDMenu/dll/" + std::string(ExternData::replayName)))
		replayPlayer->Save(strlen(ExternData::replayName) == 0
							   ? playlayer->m_level->levelName + "_" + std::to_string(playlayer->m_level->levelID)
							   : ExternData::replayName);
	TrajectorySimulation::getInstance()->onQuitPlayLayer();
	PlayLayer::onQuit(playlayer);
	playlayer = nullptr;
	startPosText = nullptr;
	if (pitchShifterDSP)
		pitchShifterDSP->release();
	pitchShifterDSP = nullptr;
	Hacks::MenuMusic();
	drawer->clearQueue();
	Hacks::UpdateRichPresence(2);
}

bool __fastcall PlayLayer::editorInitHook(gd::LevelEditorLayer* self, void*, gd::GJGameLevel* lvl)
{
	playlayer = nullptr;
	startPosText = nullptr;
	// if (drawer)
	// 	drawer->clearQueue();
	bool res = PlayLayer::editorInit(self, lvl);
	self->getObjectLayer()->addChild(HitboxNode::getInstance(), 32);
	Hacks::UpdateRichPresence(1, lvl);
	return res;
}

void __fastcall PlayLayer::lightningFlashHook(gd::PlayLayer* self, void* edx, CCPoint p, _ccColor3B c)
{
	if (!hacks.layoutMode)
		PlayLayer::lightningFlash(self, p, c);
}

void __fastcall PlayLayer::togglePlayerScaleHook(gd::PlayerObject* self, void*, bool toggle)
{
	if (drawer && playlayer)
	{
		self == playlayer->m_pPlayer1 ? drawer->m_isMini1 = toggle : drawer->m_isMini2 = toggle;
	}
	PlayLayer::togglePlayerScale(self, toggle);
}

void __fastcall PlayLayer::toggleDartModeHook(gd::PlayerObject* self, void*, bool toggle)
{
	if (playlayer && hacks.trajectory && TrajectorySimulation::getInstance()->m_pPlayer1ForSimulation &&
		TrajectorySimulation::getInstance()->m_pPlayer2ForSimulation)
	{
		TrajectorySimulation::getInstance()->m_pIsSimulation = true;
		PlayLayer::toggleDartMode(self == playlayer->m_pPlayer1
									  ? TrajectorySimulation::getInstance()->m_pPlayer1ForSimulation
									  : TrajectorySimulation::getInstance()->m_pPlayer2ForSimulation,
								  toggle);
		TrajectorySimulation::getInstance()->m_pIsSimulation = false;
	}
	PlayLayer::toggleDartMode(self, toggle);

	if (hacks.randomIcons)
	{
		if (toggle)
			self->updatePlayerWaveFrame(waveIcon);
		else if (CheckpointData::GetGamemode(self) == gd::kGamemodeCube)
			self->updatePlayerFrame(cubeIcon);
	}
	else
	{
		if (!toggle && CheckpointData::GetGamemode(self) == gd::kGamemodeCube)
			self->updatePlayerFrame(gd::GameManager::sharedState()->getPlayerFrame());
	}
}

void __fastcall PlayLayer::toggleShipModeHook(gd::PlayerObject* self, void*, bool toggle)
{
	PlayLayer::toggleShipMode(self, toggle);
	if (hacks.randomIcons && toggle)
		self->updatePlayerShipFrame(shipIcon);
}

void __fastcall PlayLayer::toggleBallModeHook(gd::PlayerObject* self, void*, bool toggle)
{
	PlayLayer::toggleBallMode(self, toggle);
	if (hacks.randomIcons && toggle)
		self->updatePlayerBallFrame(ballIcon);
}

void __fastcall PlayLayer::toggleUFOModeHook(gd::PlayerObject* self, void*, bool toggle)
{
	PlayLayer::toggleUFOMode(self, toggle);
	if (hacks.randomIcons && toggle)
		self->updatePlayerUFOFrame(ufoIcon);
}

void __fastcall PlayLayer::toggleRobotModeHook(gd::PlayerObject* self, void*, bool toggle)
{
	PlayLayer::toggleRobotMode(self, toggle);
	if (hacks.randomIcons && toggle)
		self->updatePlayerRobotFrame(robotIcon);
}

void __fastcall PlayLayer::ringJumpHook(gd::PlayerObject* self, void*, gd::GameObject* ring)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooksWithPlayer(self))
		return;
	bool a = ring->m_bHasBeenActivated;
	bool b = ring->m_bHasBeenActivatedP2;
	PlayLayer::ringJump(self, ring);
	if (replayPlayer)
		replayPlayer->HandleActivatedObjects(a, b, ring);
}

void __fastcall PlayLayer::activateObjectHook(gd::GameObject* self, void*, gd::PlayerObject* player)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooksWithPlayer(player))
	{
		TrajectorySimulation::getInstance()->activateObjectsOnPlayerSimulations(self, player);
		return;
	}
	bool a = self->m_bHasBeenActivated;
	bool b = self->m_bHasBeenActivatedP2;
	PlayLayer::activateObject(self, player);
	if (replayPlayer)
		replayPlayer->HandleActivatedObjects(a, b, self);
}

void __fastcall PlayLayer::bumpHook(gd::GJBaseGameLayer* self, void*, gd::PlayerObject* player, gd::GameObject* object)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return;
	bool a = object->m_bHasBeenActivated;
	bool b = object->m_bHasBeenActivatedP2;
	PlayLayer::bump(self, player, object);
	if (replayPlayer)
		replayPlayer->HandleActivatedObjects(a, b, object);
}

void __fastcall PlayLayer::flipGravityHook(gd::PlayLayer* self, void*, gd::PlayerObject* player, bool idk, bool idk2)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return;

	PlayLayer::flipGravity(self, player, idk, idk2);
}

void __fastcall PlayLayer::newBestHook(gd::PlayLayer* self, void*, bool b1, int i1, int i2, bool b2, bool b3, bool b4)
{
	PlayLayer::newBest(self, b1, i1, i2, b2, b3, b4);

	Hacks::UpdateRichPresence(0, 0, (std::to_string((int)startPercent)) + "-" + std::to_string((int)endPercent));
}

void __fastcall PlayLayer::playGravityEffectHook(gd::PlayLayer* self, void*, bool idk)
{
	if (hacks.trajectory && TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		return;

	PlayLayer::playGravityEffect(self, idk);
}
enumKeyCodes mapKey(int keyCode)
{
	switch (keyCode)
	{
	case enumKeyCodes::KEY_LeftShift:
	case enumKeyCodes::KEY_RightShift:
		return enumKeyCodes::KEY_Shift;

	case enumKeyCodes::KEY_LeftControl:
	case enumKeyCodes::KEY_RightContol:
		return enumKeyCodes::KEY_Control;

	case enumKeyCodes::KEY_LeftMenu:
	case enumKeyCodes::KEY_RightMenu:
		return enumKeyCodes::KEY_Alt;

	case enumKeyCodes::KEY_Left:
	case enumKeyCodes::KEY_ArrowLeft:
		return enumKeyCodes::KEY_Left;

	case enumKeyCodes::KEY_Right:
	case enumKeyCodes::KEY_ArrowRight:
		return enumKeyCodes::KEY_Right;

	case enumKeyCodes::KEY_Up:
	case enumKeyCodes::KEY_ArrowUp:
		return enumKeyCodes::KEY_Up;

	case enumKeyCodes::KEY_Down:
	case enumKeyCodes::KEY_ArrowDown:
		return enumKeyCodes::KEY_Down;

	case enumKeyCodes::KEY_NumPad0:
		return enumKeyCodes::KEY_Zero;

	case enumKeyCodes::KEY_NumPad1:
		return enumKeyCodes::KEY_One;

	case enumKeyCodes::KEY_NumPad2:
		return enumKeyCodes::KEY_Two;

	case enumKeyCodes::KEY_NumPad3:
		return enumKeyCodes::KEY_Three;

	case enumKeyCodes::KEY_NumPad4:
		return enumKeyCodes::KEY_Four;

	case enumKeyCodes::KEY_NumPad5:
		return enumKeyCodes::KEY_Five;

	case enumKeyCodes::KEY_NumPad6:
		return enumKeyCodes::KEY_Six;

	case enumKeyCodes::KEY_NumPad7:
		return enumKeyCodes::KEY_Seven;

	case enumKeyCodes::KEY_NumPad8:
		return enumKeyCodes::KEY_Eight;

	case enumKeyCodes::KEY_NumPad9:
		return enumKeyCodes::KEY_Nine;

	default:
		return enumKeyCodes::KEY_None;
	}
}

void PlayLayer::SetPitch(float pitch)
{
	auto engine = gd::FMODAudioEngine::sharedEngine();

	if (!engine || !engine->m_pSystem || !engine->m_pGlobalChannel)
		return;

	if (pitchShifterDSP)
		engine->m_pGlobalChannel->removeDSP(pitchShifterDSP);

	pitchShifterDSP->release();
	pitchShifterDSP = nullptr;

	engine->m_pSystem->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pitchShifterDSP);

	pitchShifterDSP->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, 4096);

	pitchShifterDSP->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, pitch);

	engine->m_pGlobalChannel->addDSP(0, pitchShifterDSP);
}

void __fastcall PlayLayer::dispatchKeyboardMSGHook(void* self, void*, int key, bool down)
{
	dispatchKeyboardMSG(self, key, down);
	if (ImGui::GetIO().KeysDown[ImGuiKey_LeftCtrl] && ImGui::GetIO().KeysDown[ImGuiKey_LeftAlt] && key == 'D' && down)
	{
		debug.enabled = !debug.enabled;
	}

	for (size_t i = 0; i < Shortcuts::shortcuts.size(); i++)
	{
		if (key == Shortcuts::shortcuts[i].key && down)
		{
			// this is an horrible way to do it but i cant think of any other solution
			bool prev = ExternData::show;

			if (!prev)
			{
				ImGui::GetStyle().Alpha = 0;
				ExternData::show = true;
				ExternData::fake = true;
			}
			ExternData::hackName = Shortcuts::shortcuts[i].name;
			if (!prev)
			{
				Hacks::RenderMain();
				ExternData::show = false;
				ExternData::fake = false;
				ImGui::GetStyle().Alpha = 1;
			}
		}
	}

	if (!playlayer)
		return;

	if (key == mapKey(hacks.customJumpKey))
	{
		if (down)
			playlayer->pushButton(1, true);
		else
			playlayer->releaseButton(1, true);
	}
	if (key == mapKey(hacks.customJumpKey2))
	{
		if (down)
			playlayer->pushButton(1, false);
		else
			playlayer->releaseButton(1, false);
	}

	if (key == hacks.stepBackKey && down && backFrames.size() > 0)
	{
		backFrames[backFrames.size() - 1].p1.Apply(playlayer->m_pPlayer1, false);
		backFrames[backFrames.size() - 1].p2.Apply(playlayer->m_pPlayer2, false);
		ExternData::steps = 1;
		backFrames.pop_back();
		steppingBack = true;
	}

	if (key == hacks.stepIndex)
	{
		if (down)
		{
			if (!hacks.holdAdvance)
				ExternData::steps = hacks.stepCount;
			else
				ExternData::holdingAdvance = true;
		}
		else
			ExternData::holdingAdvance = false;
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