#pragma once

#include <Geode/fmod/fmod.hpp>
#include "../util.hpp"

namespace AudioChannelControl
{
	static FMOD::Channel* audioChannel;
	static float speed = 1.f;

	FMOD_RESULT setVolumeHook(FMOD::Channel*, float);
	inline void* (__stdcall* setFrequency)(FMOD::Channel*, float);

	void set(float);
	void setPitch(float);
}
