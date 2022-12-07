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

#define STATUSSIZE 13

using namespace porcodio;

extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

ExitAlert a;

HitboxNode *drawer;
gd::PlayLayer *playlayer;

float percent, startPercent = 0, endPercent = 0, maxRun = 0, delta = 0, prevP, pressTimer = 0, releaseTimer = 0, opacity = 0, p = 0;

std::string bestRun = "Best Run: none", text;

std::vector<float> clicksArr;
std::vector<gd::StartPosObject *> sp;
std::vector<gd::GameObject *> gravityPortals, dualPortals, gamemodePortals, miniPortals, speedChanges, mirrorPortals;
std::vector<bool> willFlip;

bool add = false, pressing, dead = false, hitboxDead = false, clickType = true, pressed = false, reset = true;

ccColor3B iconCol, secIconCol;

int totalClicks, startPosIndex, noClipDeaths, actualDeaths, frames, steps = 0;
unsigned int songLength;
int hackAmts[5];

CCNode *statuses[STATUSSIZE];
CCLabelBMFont *startPosText, *macroText;
CCScheduler *scheduler;
CCSize size;
CCSprite *noclipRed;

ReplayPlayer *replayPlayer;

void UpdateLabels(gd::PlayLayer *self);
void Change();

void SetupLabel(gd::PlayLayer *self, int index)
{
	auto st = static_cast<CCLabelBMFont *>(statuses[index]);
	st = CCLabelBMFont::create("", "bigFont.fnt");
	st->setZOrder(1000);
	st->setScale(0.5f);
	st->setOpacity(150.0f);
	self->addChild(st);
	statuses[index] = st;
}

void SmartStartPosSetup(gd::PlayLayer *self)
{
	willFlip.resize(gravityPortals.size());
	for (gd::StartPosObject *startPos : sp)
	{
		for (size_t i = 0; i < gravityPortals.size(); i++)
		{
			if (gravityPortals[i]->getPositionX() > startPos->getPositionX())
				break;
			if (gravityPortals[i]->getPositionX() < startPos->getPositionX())
				willFlip[i] = gravityPortals[i]->m_nObjectID == 11;
		}

		startPos->m_levelSettings->m_startDual = self->m_pLevelSettings->m_startDual;
		for (size_t i = 0; i < dualPortals.size(); i++)
		{
			if (dualPortals[i]->getPositionX() > startPos->getPositionX())
				break;
			if (dualPortals[i]->getPositionX() < startPos->getPositionX())
				startPos->m_levelSettings->m_startDual = dualPortals[i]->m_nObjectID == 286;
		}

		startPos->m_levelSettings->m_startGamemode = self->m_pLevelSettings->m_startGamemode;
		for (size_t i = 0; i < gamemodePortals.size(); i++)
		{
			if (gamemodePortals[i]->getPositionX() > startPos->getPositionX())
				break;
			if (gamemodePortals[i]->getPositionX() < startPos->getPositionX())
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
			if (miniPortals[i]->getPositionX() > startPos->getPositionX())
				break;
			if (miniPortals[i]->getPositionX() < startPos->getPositionX())
				startPos->m_levelSettings->m_startMini = miniPortals[i]->m_nObjectID == 101;
		}

		startPos->m_levelSettings->m_startSpeed = self->m_pLevelSettings->m_startSpeed;
		for (size_t i = 0; i < speedChanges.size(); i++)
		{
			if (speedChanges[i]->getPositionX() > startPos->getPositionX())
				break;
			if (speedChanges[i]->getPositionX() < startPos->getPositionX())
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

	CCARRAY_FOREACH(self->m_pObjects, obje)
	{
		auto g = reinterpret_cast<gd::GameObject *>(obje);

		if (g->m_nObjectID == 31)
			sp.push_back(reinterpret_cast<gd::StartPosObject *>(obje));

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
	text = "Accuracy: 100.00%";
	EndLevelLayer::accuracy = text;
	noClipDeaths = 0;
	totalClicks = 0;
	actualDeaths = 0;
	EndLevelLayer::deaths = 0;
	delta = 0;
	dead = false;
	clicksArr.clear();

	pressTimer = 0;
	releaseTimer = hacks.releaseTime;

	const ccColor3B noclipColor = {hacks.noclipColor[0] * 255, hacks.noclipColor[1] * 255, hacks.noclipColor[2] * 255};
	noclipRed = CCSprite::createWithSpriteFrameName("block005b_05_001.png");
	noclipRed->setPosition({size.width / 2, size.height / 2});
	noclipRed->setScale(1000.0f);
	noclipRed->setColor(noclipColor);
	noclipRed->setOpacity(0);
	noclipRed->setZOrder(1000);
	self->addChild(noclipRed);

	statuses[0] = CCSprite::createWithSpriteFrameName("edit_eToggleBtn2_001.png");
	statuses[0]->setZOrder(1000);
	statuses[0]->setScale(0.5f);
	self->addChild(statuses[0]);

	for (size_t i = 1; i < STATUSSIZE; i++)
		SetupLabel(self, i);

	macroText = CCLabelBMFont::create("ciao", "bigFont.fnt");
	macroText->setZOrder(1000);
	macroText->setScale(0.5f);
	macroText->setOpacity(150.0f);
	macroText->setAnchorPoint(CCPointMake(0, macroText->getAnchorPoint().y));
	macroText->setPosition(CCPointMake(5, 15));
	self->addChild(macroText);

	for (size_t i = 0; i < STATUSSIZE; i++)
		UpdatePositions(i);

	if (sp.size() > 0)
	{
		startPosText = CCLabelBMFont::create("", "bigFont.fnt");
		startPosText->setZOrder(1000);
		startPosText->setScale(0.5f);
		startPosText->setOpacity(150.0f);
		startPosText->setPosition(size.width / 2, 15);
		self->addChild(startPosText);
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

	unsigned int *pos = &songLength;
	gd::FMODAudioEngine::sharedEngine()->m_pSound->getLength(pos, FMOD_TIMEUNIT_MS);

	Change();

	return result;
}

bool __fastcall PlayLayer::pushButtonHook(gd::PlayerObject *self, void *, int PlayerButton)
{
	if (playlayer && replayPlayer && replayPlayer->IsRecording())
		replayPlayer->RecordAction(true, self, self == playlayer->m_pPlayer1, false);
	add = true;
	pressing = true;

	return PlayLayer::pushButton(self, PlayerButton);
}

bool __fastcall PlayLayer::releaseButtonHook(gd::PlayerObject *self, void *, int PlayerButton)
{
	pressing = false;
	if (playlayer && replayPlayer && replayPlayer->IsRecording())
	{
		if (self == playlayer->m_pPlayer2 && delta > 0 && playlayer->m_bIsDualMode || self == playlayer->m_pPlayer1)
			replayPlayer->RecordAction(false, self, self == playlayer->m_pPlayer1, false);
	}

	return PlayLayer::releaseButton(self, PlayerButton);
}

void __fastcall PlayLayer::destroyPlayer_H(gd::PlayLayer *self, void *, gd::PlayerObject *player, gd::GameObject *obj)
{
	if (delta > 0.15f && !Hacks::player["mods"][0]["toggle"] && !Hacks::player["mods"][2]["toggle"])
	{
		float run = ((player->getPositionX() / self->m_levelLength) * 100.0f) - startPercent;
		endPercent = (player->getPositionX() / self->m_levelLength) * 100.0f;
		if ((int)startPercent != (int)endPercent && run > maxRun)
		{
			maxRun = run;
			bestRun = (int)startPercent != 0 ? "Best Run: " + std::to_string((int)startPercent) + "-" + std::to_string((int)endPercent) : "Best Run: " + std::to_string((int)endPercent) + "%";
		}
	}
	PlayLayer::destroyPlayer(self, player, obj);
}

gd::GameSoundManager *__fastcall PlayLayer::levelCompleteHook(gd::PlayLayer *self)
{
	float run = 100.0f - startPercent;
	endPercent = 100.0f;
	if (run > maxRun)
	{
		maxRun = run;
		bestRun = (int)startPercent != 0 ? "Best Run: " + std::to_string((int)startPercent) + "-" + std::to_string((int)endPercent) : "Best Run: " + std::to_string((int)endPercent) + "%";
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
	if (delta > 0.2f && (percent - startPercent) * playlayer->m_levelLength > 0.04f * playlayer->m_levelLength)
	{
		dead = true;
		hitboxDead = true;
	}

	PlayLayer::death(self, go, powerrangers);
}

bool IsCheating()
{
	for (size_t i = 0; i < Hacks::bypass["mods"].size(); i++)
	{
		if (Hacks::bypass["mods"][i]["toggle"].get<bool>() && Hacks::bypass["mods"][i]["cheat"].get<bool>())
			return true;
	}

	for (size_t i = 0; i < Hacks::creator["mods"].size(); i++)
	{
		if (Hacks::creator["mods"][i]["toggle"].get<bool>() && Hacks::creator["mods"][i]["cheat"].get<bool>())
			return true;
	}

	for (size_t i = 0; i < Hacks::global["mods"].size(); i++)
	{
		if (Hacks::global["mods"][i]["toggle"].get<bool>() && Hacks::global["mods"][i]["cheat"].get<bool>())
			return true;
	}

	for (size_t i = 0; i < Hacks::level["mods"].size(); i++)
	{
		if (Hacks::level["mods"][i]["toggle"].get<bool>() && Hacks::level["mods"][i]["cheat"].get<bool>())
			return true;
	}

	for (size_t i = 0; i < Hacks::player["mods"].size(); i++)
	{
		if (Hacks::player["mods"][i]["toggle"].get<bool>() && Hacks::player["mods"][i]["cheat"].get<bool>())
			return true;
	}

	if (hacks.fps > 360.0f || hacks.autoclicker || hacks.frameStep || scheduler->getTimeScale() != 1 || replayPlayer->IsPlaying() || hacks.layoutMode || hacks.showHitboxes && !hacks.onlyOnDeath)
		return true;

	return false;
}

void UpdateLabels(gd::PlayLayer *self)
{
	auto spritePtr = static_cast<CCSprite *>(statuses[0]);
	const ccColor3B red = {255, 0, 0};
	const ccColor3B white = {255, 255, 255};
	const ccColor3B green = {0, 255, 0};
	const ccColor3B yellow = {255, 165, 0};

	if (labels.statuses[0] && scheduler)
	{
		spritePtr->setOpacity(labels.opacity[0]);
		if (IsCheating())
			spritePtr->setColor(red);
		else if (Hacks::level["mods"][24]["toggle"])
			spritePtr->setColor(yellow);
		else
			spritePtr->setColor(green);
	}
	else
		spritePtr->setOpacity(0);

	auto fontPtr = static_cast<CCLabelBMFont *>(statuses[1]);

	if (labels.statuses[1])
	{
		if (labels.styles[0] == 0)
			fontPtr->setString((std::to_string((int)(ImGui::GetIO().Framerate)) + "FPS").c_str());
		else
			fontPtr->setString((std::to_string((int)(ImGui::GetIO().Framerate))).c_str());
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
		if (labels.styles[1] == 0)
			fontPtr->setString((std::to_string(clicksArr.size()) + "/" + std::to_string(totalClicks) + "CPS").c_str());
		else
			fontPtr->setString((std::to_string(clicksArr.size()) + "/" + std::to_string(totalClicks)).c_str());
		const ccColor3B pressed = {hacks.clickColor[0] * 255, hacks.clickColor[1] * 255, hacks.clickColor[2] * 255};
		fontPtr->setColor(pressing ? pressed : white);
	}
	else
	{
		fontPtr->setString("");
		clicksArr.clear();
	}

	fontPtr = static_cast<CCLabelBMFont *>(statuses[3]);

	if (labels.statuses[3])
	{
		if (hacks.noClipAccuracyLimit > 0 && p * 100.0f < hacks.noClipAccuracyLimit)
		{
			bool t = Hacks::player["mods"][0]["toggle"];
			Hacks::player["mods"][0]["toggle"] = true;
			Hacks::ToggleJSONHack(Hacks::player, 0, false);
			Hacks::player["mods"][0]["toggle"] = t;
		}
		fontPtr->setColor(dead ? red : white);
		fontPtr->setString(text.c_str());
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[4]);

	if (labels.statuses[4] && replayPlayer)
	{
		fontPtr->setString(("Deaths: " + std::to_string(actualDeaths)).c_str());
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
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[6]);

	if (labels.statuses[6])
	{
		fontPtr->setString(bestRun.c_str());
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[7]);

	if (labels.statuses[7])
	{
		fontPtr->setString((std::to_string(self->m_level->m_nAttempts) + " Attempts").c_str());
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[8]);

	if (labels.statuses[8] && !hacks.onlyInRuns || labels.statuses[8] && (self->m_isPracticeMode || self->m_isTestMode) && hacks.onlyInRuns)
	{
		fontPtr->setString(("From " + std::to_string((int)startPercent) + "%").c_str());
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[9]);

	if (labels.statuses[9])
	{
		fontPtr->setString(hacks.message);
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[10]);

	if (labels.statuses[10])
	{
		fontPtr->setString(("Attempt " + std::to_string(self->m_currentAttempt)).c_str());
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[11]);

	if (labels.statuses[11])
	{
		fontPtr->setString(("Level ID: " + std::to_string(self->m_level->m_nLevelID)).c_str());
	}
	else
		fontPtr->setString("");

	fontPtr = static_cast<CCLabelBMFont *>(statuses[12]);

	if (labels.statuses[12])
	{
		if (!self->m_hasCompletedLevel)
			fontPtr->setString((std::to_string(self->m_attemptJumpCount) + " Jumps").c_str());
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

	switch (labels.fonts[index])
	{
	case 1:
		sc *= 1.65f;
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

bool lastFrameDead = false;

void Update(gd::PlayLayer *self, float dt)
{
	percent = (self->m_pPlayer1->getPositionX() / self->m_levelLength) * 100.0f;
	self->m_attemptLabel->setVisible(!hacks.hideattempts);

	debug.debugNumber = self->m_pPlayer1->getScale();

	if (dead && !lastFrameDead)
	{
		actualDeaths++;
		EndLevelLayer::deaths++;
	}

	lastFrameDead = dead;

	if (hacks.autoSyncMusic)
	{
		float f = self->timeForXPos(self->m_pPlayer1->getPositionX());
		unsigned int p;
		unsigned int *pos = &p;
		float offset = self->m_pLevelSettings->m_songStartOffset * 1000;
		gd::FMODAudioEngine::sharedEngine()->m_pGlobalChannel->getPosition(pos, FMOD_TIMEUNIT_MS);
		if (std::abs((int)(f * 1000) - (int)p + offset) > hacks.musicMaxDesync && !self->m_hasCompletedLevel)
		{
			gd::FMODAudioEngine::sharedEngine()->m_pGlobalChannel->setPosition((uint32_t)(f * 1000) + (uint32_t)offset, FMOD_TIMEUNIT_MS);
		}
	}

	if (noClipDeaths == 0)
	{
		text = "Accuracy: 100.00%";
		EndLevelLayer::accuracy = text;
	}
	else if (labels.statuses[3] || hacks.showExtraInfo)
	{
		p = (float)(frames - noClipDeaths) / (float)frames;
		std::stringstream stream;
		stream << "Accuracy: " << std::fixed << std::setprecision(2) << p * 100.f << "%";
		text = stream.str();
		EndLevelLayer::accuracy = text;
	}

	if (drawer)
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

	if (sp.size() > 0 && hacks.startPosSwitcher && startPosText)
	{
		startPosText->setString((std::to_string(startPosIndex + 1) + "/" + std::to_string(sp.size())).c_str());
	}
	else if (sp.size() > 0 && startPosText)
		startPosText->setString("");

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
	}
	prevP = self->m_pPlayer1->getPositionX();

	if (startPosText && delta > 0.5f && startPosText->getOpacity() > 0)
		startPosText->setOpacity(startPosText->getOpacity() - 2);

	if (hacks.rainbowIcons)
	{
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(self->m_totalTime * hacks.rainbowSpeed, 1, 1, r, g, b);
		const ccColor3B col = {(GLubyte)(r * 255.0f), (GLubyte)(g * 255.0f), (GLubyte)(b * 255.0f)};

		if (!hacks.onlyRainbowOutline)
		{
			self->m_pPlayer1->setColor(col);
			self->m_pPlayer1->m_iconSprite->setColor(col);
			self->m_pPlayer1->m_vehicleSprite->setColor(col);
			self->m_pPlayer1->m_spiderSprite->setColor(col);
			self->m_pPlayer1->m_robotSprite->setColor(col);
			self->m_pPlayer2->setColor(col);
			self->m_pPlayer2->m_iconSprite->setColor(col);
			self->m_pPlayer2->m_vehicleSprite->setColor(col);
			self->m_pPlayer2->m_spiderSprite->setColor(col);
			self->m_pPlayer2->m_robotSprite->setColor(col);
			self->m_pPlayer1->m_iconGlow->setColor(col);
			self->m_pPlayer1->m_vehicleGlow->setColor(col);
		}
		self->m_pPlayer1->m_vehicleGlow->setChildColor(col);
		self->m_pPlayer1->m_vehicleGlow->setChildOpacity(255);
		self->m_pPlayer2->m_vehicleGlow->setChildColor(col);
		self->m_pPlayer2->m_vehicleGlow->setChildOpacity(255);
	}

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

	UpdateLabels(self);

	dead = false;

	if (replayPlayer && replayPlayer->IsPlaying())
		replayPlayer->Update(self);

	if (replayPlayer->recorder.m_recording)
		replayPlayer->recorder.handle_recording(self, dt);

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

		float xp = self->m_pPlayer1->getPositionX();

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

		float xp = self->m_pPlayer1->getPositionX();

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

void __fastcall PlayLayer::pauseGameHook(gd::PlayLayer *self, void *, bool idk)
{
	if (replayPlayer && replayPlayer->IsRecording())
	{
		replayPlayer->RecordAction(false, self->m_pPlayer1, true, false);
		replayPlayer->RecordAction(false, self->m_pPlayer2, false, false);
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

void __fastcall PlayLayer::updateHook(gd::PlayLayer *self, void *, float dt)
{

	if (hacks.frameStep)
	{
		if (steps > 0)
		{
			Update(self, dt);
			steps--;
		}
	}
	else
		Update(self, dt);

	return;
}

void __fastcall PlayLayer::triggerObjectHook(gd::EffectGameObject *self, void *, gd::GJBaseGameLayer *idk)
{
	auto id = self->m_nObjectID;
	if (hacks.layoutMode && (id == 899 || id == 1006 || id == 1007 || id == 105 || id == 29 || id == 56 || id == 915 || id == 30 || id == 58))
		return;
	PlayLayer::triggerObject(self, idk);
}

void Change()
{
	if (startPosText)
		startPosText->setOpacity(150.0f);

	if (playlayer->unk330)
		playlayer->unk330->release();
	playlayer->unk330 = nullptr;
	if (startPosIndex >= 0)
	{
		playlayer->m_startPos = sp[startPosIndex];
		playlayer->m_playerStartPosition = CCPointMake(sp[startPosIndex]->getPositionX(), sp[startPosIndex]->getPositionY());
		PlayLayer::resetLevelHook(playlayer, nullptr);
	}
	else
	{
		playlayer->m_startPos = nullptr;
		playlayer->m_playerStartPosition = CCPointMake(0, 105);
		PlayLayer::resetLevelHook(playlayer, nullptr);
	}
}

void PlayLayer::SetHitboxSize(float size)
{
	static std::vector<cocos2d::CCSize> sizes;
	if (!playlayer || size < 0.1f)
		return;

	for (size_t i = 0; i < playlayer->getAllObjects()->count(); i++)
	{
		auto obj = static_cast<gd::GameObject *>(playlayer->getAllObjects()->objectAtIndex(i));
		if (sizes.size() <= i)
			sizes.push_back(obj->m_obObjectRect2.size);

		obj->m_obObjectRect2.size.setSize(sizes[i].width * size, sizes[i].height * size);
	}
}

void __fastcall PlayLayer::resetLevelHook(gd::PlayLayer *self, void *)
{
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
		drawer->clearQueue();

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

void PlayLayer::Quit()
{
	PlayLayer::onQuit(playlayer);
	playlayer = nullptr;
	startPosText = nullptr;
	Hacks::MenuMusic();
	drawer->clearQueue();
}

bool __fastcall PlayLayer::editorInitHook(gd::LevelEditorLayer *self, void *, gd::GJGameLevel *lvl)
{
	playlayer = nullptr;
	startPosText = nullptr;
	if (drawer)
		drawer->clearQueue();
	bool res = PlayLayer::editorInit(self, lvl);
	self->getObjectLayer()->addChild(HitboxNode::getInstance(), 32);
	return res;
}

void __fastcall PlayLayer::lightningFlashHook(gd::PlayLayer *self, void *edx, CCPoint p, _ccColor3B c)
{
	if (!hacks.layoutMode)
		PlayLayer::lightningFlash(self, p, c);
}

void __fastcall PlayLayer::dispatchKeyboardMSGHook(void *self, void *, int key, bool down)
{
	dispatchKeyboardMSG(self, key, down);
	static int debugNum = 0;
	if (key == 'L' && down)
	{
		debugNum++;
		if (debugNum >= 10)
			debug.enabled = true;
	}

	for (size_t i = 0; i < Shortcuts::shortcuts.size(); i++)
	{
		if (key == Shortcuts::shortcuts[i].key && down)
		{
			Shortcuts::OnPress(Shortcuts::shortcuts[i].shortcutIndex);
		}
	}

	if (!playlayer)
		return;

	if (key == hacks.stepIndex && down)
		steps = hacks.stepCount;

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