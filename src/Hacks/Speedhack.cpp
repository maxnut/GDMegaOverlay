#include "Speedhack.h"
#include "../Common.h"
#include "../Macrobot/Macrobot.h"

#include "../Macrobot/Record.h"

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

		CCScheduler::update(dt);
	}
};
