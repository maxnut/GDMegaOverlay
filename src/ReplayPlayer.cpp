#include "ReplayPlayer.h"
#include "Hacks.h"
#include "PlayLayer.h"
#include "bools.h"
#include "ctime"
#include "fmod.hpp"
#include <stdio.h>

extern struct HacksStr hacks;
FMOD::System* sys;
std::vector<FMOD::Sound*> clicks, releases, softclicks;
FMOD::Channel *channel = 0, *channel2 = 0;

bool oldClick1 = false, oldClick2 = false;

double oldTime = 0, oldTimeP2 = 0;

uint32_t ReplayPlayer::GetFrame()
{
	gd::PlayLayer* pl = gd::GameManager::sharedState()->getPlayLayer();
	if (pl)
		return static_cast<uint32_t>((float)pl->m_time * hacks.fps) + frameOffset;
	else
		return -1;
}

void ReplayPlayer::StartPlaying(gd::PlayLayer* playLayer)
{
	UpdateFrameOffset();
	actionIndex = 0;
	actionIndex2 = 0;
}

ReplayPlayer::ReplayPlayer()
{
	srand(time(NULL));
	FMOD::System_Create(&sys);
	sys->init(32, FMOD_INIT_NORMAL, nullptr);
}

void ReplayPlayer::ChangeClickpack()
{
	clicks.clear();
	releases.clear();
	softclicks.clear();

	std::string path = "GDMenu/clickpacks/" + ExternData::clickpacks[hacks.currentClickpack];

	bool exist = std::filesystem::exists(path + "/clicks/1.wav");
	uint32_t i = 1;
	while (exist)
	{
		auto filestr = path + "/clicks/" + std::to_string(i) + ".wav";
		FMOD::Sound* s;
		sys->createSound(filestr.c_str(), FMOD_LOOP_OFF, 0, &s);
		if (s)
			clicks.push_back(s);
		i++;
		filestr = path + "/clicks/" + std::to_string(i) + ".wav";
		exist = std::filesystem::exists(filestr);
	}
	ExternData::amountOfClicks = clicks.size();

	exist = std::filesystem::exists(path + "/releases/1.wav");
	i = 1;
	while (exist)
	{
		auto filestr = path + "/releases/" + std::to_string(i) + ".wav";
		FMOD::Sound* s;
		sys->createSound(filestr.c_str(), FMOD_LOOP_OFF, 0, &s);
		if (s)
			releases.push_back(s);
		i++;
		filestr = path + "/releases/" + std::to_string(i) + ".wav";
		exist = std::filesystem::exists(filestr);
	}
	ExternData::amountOfReleases = releases.size();

	exist = std::filesystem::exists(path + "/softclicks/1.wav");
	i = 1;
	while (exist)
	{
		auto filestr = path + "/softclicks/" + std::to_string(i) + ".wav";
		FMOD::Sound* s;
		sys->createSound(filestr.c_str(), FMOD_LOOP_OFF, 0, &s);
		if (s)
			softclicks.push_back(s);
		i++;
		filestr = path + "/softclicks/" + std::to_string(i) + ".wav";
		exist = std::filesystem::exists(filestr);
	}
	ExternData::amountOfSoftClicks = softclicks.size();
}

void ReplayPlayer::ToggleRecording()
{
	playing = false;
	recording = !recording;

	replay.fps = hacks.fps;
	Hacks::FPSBypass(replay.fps);
	hacks.tpsBypass = replay.fps;
	ExternData::tps = hacks.tpsBypass;
	UpdateFrameOffset();

	if (replay.GetActionsSize() > 0 && IsRecording())
	{
		replay.ClearActions();
	}
}

void ReplayPlayer::TogglePlaying()
{
	recording = false;
	playing = !playing;
	Hacks::FPSBypass(hacks.fps);
	hacks.tpsBypass = hacks.fps;
	ExternData::tps = hacks.tpsBypass;
	ExternData::level["mods"][24]["toggle"] = true;
	Hacks::ToggleJSONHack(ExternData::level, 24, false);
	UpdateFrameOffset();
}

void ReplayPlayer::Reset(gd::PlayLayer* playLayer)
{
	oldTime = 0;

	if (recording)
		recorder.update_song_offset(playLayer);

	bool addedAction = false;
	oldClick1 = false;
	oldClick2 = false;

	bool hasCheckpoint = playLayer->m_checkpoints->count() > 0;

	if (IsPlaying())
	{
		UpdateFrameOffset();
		actionIndex = 0;
		actionIndex2 = 0;
		playLayer->releaseButton(0, false);
		playLayer->releaseButton(0, true);
		practice.activatedObjects.clear();
		practice.activatedObjectsP2.clear();
	}
	else
	{
		const auto checkpoint = practice.GetLast();
		if (!hasCheckpoint)
		{
			practice.activatedObjects.clear();
			practice.activatedObjectsP2.clear();
			frameOffset = 0;
		}
		else
		{
			frameOffset = checkpoint.frameOffset;
			constexpr auto delete_from = [&](auto& vec, size_t index) { vec.erase(vec.begin() + index, vec.end()); };
			delete_from(practice.activatedObjects, checkpoint.activatedObjectsSize);
			delete_from(practice.activatedObjectsP2, checkpoint.activatedObjectsP2Size);
			if (IsRecording())
			{
				for (const auto& object : practice.activatedObjects)
				{
					if (object)
						object->m_bHasBeenActivated = true;
				}
				for (const auto& object : practice.activatedObjectsP2)
				{
					if (object)
						object->m_bHasBeenActivatedP2 = true;
				}
			}
		}

		if (IsRecording())
		{
			replay.RemoveActionsAfter(GetFrame());
		}
	}

	if ((IsRecording() || hacks.fixPractice) && playLayer->m_isPracticeMode && hasCheckpoint)
		practice.ApplyCheckpoint();

	if (hasCheckpoint || playLayer->m_isTestMode)
	{
		float targetX = playLayer->m_pPlayer1->getPositionX();
		size_t ind = 0;
		auto ac = replay.getActions();
		while (ind < ac.size() && ac[ind].px < targetX)
			ind++;

		actionIndex = ind;
	}

	if (IsPlaying() || IsRecording() && GetActionsSize() <= 0)
	{
		PlayLayer::releaseButton(playLayer->m_pPlayer1, 0);
		if (playLayer->m_bIsDualMode)
		{
			PlayLayer::releaseButton(playLayer->m_pPlayer2, 0);
		}
	}
}

void ReplayPlayer::Load(std::string name)
{
	replay.Load("GDMenu/macros/" + name + ".macro");
}

void ReplayPlayer::Delete(std::string name)
{
	remove(std::string("GDMenu/macros/" + name + ".macro").c_str());
}

float ReplayPlayer::Update(gd::PlayLayer* playLayer)
{
	sys->update();

	float dt = 0;

	if ((replay.fps != hacks.fps || hacks.tpsBypass != replay.fps) && (IsPlaying() || IsRecording()))
	{
		hacks.fps = replay.fps;
		Hacks::FPSBypass(hacks.fps);
		hacks.tpsBypass = hacks.fps;
		ExternData::tps = hacks.tpsBypass;
	}

	if (playLayer->m_hasCompletedLevel || playLayer->m_isDead)
		return dt;

	if (IsRecording() && hacks.replayMode == 2)
	{
		replay.AddFrame({false, GetFrame(), playLayer->m_pPlayer1->m_yAccel, playLayer->m_pPlayer1->m_position.x,
						 playLayer->m_pPlayer1->m_position.y, playLayer->m_pPlayer1->getRotation()});
		if (playLayer->m_bIsDualMode)
		{
			replay.AddFrame({true, GetFrame(), playLayer->m_pPlayer2->m_yAccel, playLayer->m_pPlayer2->m_position.x,
							 playLayer->m_pPlayer2->m_position.y, playLayer->m_pPlayer2->getRotation()});
		}
	}

	if (!IsPlaying() || actionIndex >= replay.getActions().size() || replay.getActions().size() <= 0 ||
		(hacks.replayMode == 2 && (actionIndex2 >= replay.getCaptures().size() || replay.getCaptures().size() <= 0)))
		return dt;

	size_t limit = 1;
	auto actions = replay.getActions();
	while (actionIndex + limit < actions.size() &&
		   (actions[actionIndex + limit].player2 ||
			(actions[actionIndex].frame == actions[actionIndex + limit].frame ||
			 actions[actionIndex].px >= 0 && actions[actionIndex].px == actions[actionIndex + limit].px)))
		limit++;

	for (size_t i = 0; i < limit; i++)
	{
		auto ac = actions[actionIndex];
		if (playLayer->m_pLevelSettings->m_twoPlayerMode && playLayer->m_bIsDualMode && actionIndex + 1 < actions.size() &&
			ac.px > actions[actionIndex + 1].px)
		{
			actionIndex++;
			continue;
		}
		if (!ac.player2 && playLayer->m_pPlayer1->m_position.x >= ac.px ||
			ac.player2 && playLayer->m_pPlayer2->m_position.x >= ac.px)
		{
			// if (debug.enabled && playLayer->m_pPlayer1->m_position.x != ac.px/*  || playLayer->m_pPlayer1->m_yAccel
			// != ac.yAccel || 	playLayer->m_pPlayer1->m_position.y != ac.py */)
			// {
			// 	hacks.frameStep = true;
			// }
			if (!ac.player2 && hacks.replayMode == 1)
			{
				playLayer->m_pPlayer1->m_position.x = ac.px;
				playLayer->m_pPlayer1->m_yAccel = ac.yAccel;
				playLayer->m_pPlayer1->m_position.y = ac.py;
			}
			else if (ac.player2 && hacks.replayMode == 1)
			{
				playLayer->m_pPlayer2->m_position.x = ac.px;
				playLayer->m_pPlayer2->m_yAccel = ac.yAccel;
				playLayer->m_pPlayer2->m_position.y = ac.py;
			}

			float v;
			float p;
			double t;
			uint16_t rc, rr, rmc;

			if (playLayer->m_pLevelSettings->m_twoPlayerMode && ac.player2)
				t = playLayer->m_time - oldTimeP2;
			else
				t = playLayer->m_time - oldTime;

			if (hacks.clickbot && clicks.size() > 0 && releases.size() > 0)
			{
				rc = rand() % clicks.size();
				rr = rand() % releases.size();
				if (ExternData::amountOfSoftClicks > 0)
					rmc = rand() % softclicks.size();
				p = hacks.minPitch +
					static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (hacks.maxPitch - hacks.minPitch)));
				v = t >= hacks.playSoftClicksAt ? 0.5f * ((float)t * 6) * hacks.baseVolume : 0.6f * hacks.baseVolume;
				if (v > 0.5f * hacks.baseVolume)
					v = 0.5f * hacks.baseVolume;
			}
			if (ac.press)
			{
				if (hacks.clickbot && t > hacks.minTimeDifference && ExternData::amountOfClicks > 0 &&
					ac.press != (oldClick1 && !ac.player2 || oldClick2 && ac.player2))
				{
					sys->playSound(t > 0.0 && t < hacks.playSoftClicksAt && ExternData::amountOfSoftClicks > 0
									   ? softclicks[rmc]
									   : clicks[rc],
								   nullptr, false, &channel);
					channel->setPitch(p);
					channel->setVolume(v);
				}
				if (playLayer->m_pLevelSettings->m_twoPlayerMode)
					oldTimeP2 = playLayer->m_time;
				else
					oldTime = playLayer->m_time;
				PlayLayer::isBot = true;
				if (ac.player2)
				{
					playLayer->pushButton(1, false);
				}
				else
				{
					playLayer->pushButton(1, true);
				}
				PlayLayer::isBot = false;
			}
			else
			{
				if (hacks.clickbot && ExternData::amountOfReleases > 0 &&
					ac.press != (oldClick1 && !ac.player2 || oldClick2 && ac.player2))
				{
					sys->playSound(releases[rr], nullptr, false, &channel);
					channel2->setPitch(p);
					channel2->setVolume(v + 0.5f);
				}
				if (playLayer->m_pLevelSettings->m_twoPlayerMode)
					oldTimeP2 = playLayer->m_time;
				else
					oldTime = playLayer->m_time;
				PlayLayer::isBot = true;
				if (ac.player2)
				{
					playLayer->releaseButton(1, false);
				}
				else
				{
					playLayer->releaseButton(1, true);
				}
				PlayLayer::isBot = false;
			}

			if (ac.player2)
				oldClick2 = ac.press;
			else
				oldClick1 = ac.press;

			actionIndex++;
		}
	}

	if (replay.GetFrameCapturesSize() <= 0 || hacks.replayMode < 2)
		return dt;

	auto captures = replay.getCaptures();

	size_t limit2 = 1;
	while (actionIndex2 + limit2 < captures.size() &&
		   (captures[actionIndex2 + limit2].player2 ||
			(captures[actionIndex2].frame == captures[actionIndex2 + limit2].frame ||
			 captures[actionIndex2].px >= 0 && captures[actionIndex2].px == captures[actionIndex2 + limit2].px)))
		limit2++;

	for (int i = 0; i < limit2; i++)
	{
		auto cap = captures[actionIndex2];
		if (playLayer->m_pPlayer1->m_position.x >= cap.px)
		{
			if (!cap.player2)
			{
				playLayer->m_pPlayer1->m_position.x = cap.px;
				playLayer->m_pPlayer1->m_position.y = cap.py;
				playLayer->m_pPlayer1->m_yAccel = cap.yAccel;
				playLayer->m_pPlayer1->setRotation(cap.rot);
			}
			else
			{
				playLayer->m_pPlayer2->m_position.x = cap.px;
				playLayer->m_pPlayer2->m_position.y = cap.py;
				playLayer->m_pPlayer2->m_yAccel = cap.yAccel;
				playLayer->m_pPlayer2->setRotation(cap.rot);
			}
			actionIndex2++;
		}
	}

	return dt;
}

void ReplayPlayer::UpdateFrameOffset()
{
	frameOffset = GetPractice().GetLast().frameOffset;
}

void ReplayPlayer::HandleActivatedObjects(bool a, bool b, gd::GameObject* object)
{
	auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
	if (play_layer && play_layer->m_isPracticeMode && IsRecording())
	{
		if (object->m_bHasBeenActivated && !a)
			GetPractice().activatedObjects.push_back(object);
		if (object->m_bHasBeenActivatedP2 && !b)
			GetPractice().activatedObjectsP2.push_back(object);
	}
}

void ReplayPlayer::RecordAction(bool press, gd::PlayerObject* pl, bool player1)
{
	if (!IsRecording())
		return;
	Action a;
	a.player2 = !player1;
	a.frame = GetFrame();
	a.press = press;
	a.yAccel = pl->m_yAccel;
	a.px = pl->m_position.x;
	a.py = pl->m_position.y;
	replay.AddAction(a);
}