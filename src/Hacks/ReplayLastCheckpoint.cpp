#include "ReplayLastCheckpoint.h"
#include "../Common.h"

#include "../util.hpp"

#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;
using namespace ReplayLastCheckpoint;

class $modify(PlayLayer)
{
	// ok so for whatever reason on windows instead of calling togglepracticemode rob calls resetlevelfromstart. on android
	// he calls togglepracticemode tho :)
	void resetLevelFromStart()
	{
		bool replay = Mod::get()->getSavedValue<bool>("level/replay_checkpoint");
	
		if (replay && levelCompleted)
		{
			GameManager::get()->getPlayLayer()->resetLevel();
			GameManager::get()->getPlayLayer()->m_isPracticeMode = true;
			levelCompleted = false;
			return;
		}
		levelCompleted = false;
		PlayLayer::resetLevelFromStart();
	}

	void levelComplete()
	{
		levelCompleted = true;
		PlayLayer::levelComplete();
	}

	void resetLevel()
	{
		levelCompleted = false;
		PlayLayer::resetLevel();
	}
};
