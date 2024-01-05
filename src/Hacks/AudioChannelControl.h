#pragma once

#include <fmod.hpp>
#include "MinHook.h"
#include "../utils.hpp"

namespace AudioChannelControl
{
	static FMOD::Channel* audioChannel;
	static float speed = 1.f;

	inline void* (__stdcall* setVolume)(FMOD::Channel*, float);
	void* __stdcall setVolumeHook(FMOD::Channel*, float);
	inline void* (__stdcall* setFrequency)(FMOD::Channel*, float);

	inline int(__thiscall* playLayerResetLevel)(void*);
	int __fastcall playLayerResetLevelHook(void* self, void*);

	void set(float);
	void setPitch(float);
	void initHooks();
}
