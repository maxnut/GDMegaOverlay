#pragma once

#include <Geode/fmod/fmod.hpp>
#include "../util.hpp"

namespace AudioChannelControl
{
	inline float speed = 1.f;

	FMOD_RESULT setVolumeHook(FMOD::Channel*, float);

	void set(float);
	void setPitch(float);
}
