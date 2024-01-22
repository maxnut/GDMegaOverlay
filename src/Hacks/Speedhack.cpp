#include "Speedhack.h"
#include "../Common.h"
#include "../Macrobot/Macrobot.h"

#include "../Macrobot/AudioRecord.h"

#include <Geode/modify/CCScheduler.hpp>
#include <Geode/binding/GameManager.hpp>

using namespace geode::prelude;

class $modify(CCScheduler)
{
	void update(float dt)
	{
		float speedhack =
		Mod::get()->getSavedValue<bool>("general/speedhack/enabled") ? Mod::get()->getSavedValue<float>("general/speedhack/value") : 1.f;

		dt *= speedhack;

		if (Macrobot::playerMode != -1 && !AudioRecord::recording)
		{
			float framerate;

			if (Mod::get()->getSavedValue<bool>("general/fps/enabled"))
				framerate = Mod::get()->getSavedValue<float>("general/fps/value", 60.f);
			else
				framerate = GameManager::get()->m_customFPSTarget;

			dt = 1.0f / (framerate * speedhack);
			dt *= speedhack;
		}

		CCScheduler::update(dt);
	}
};