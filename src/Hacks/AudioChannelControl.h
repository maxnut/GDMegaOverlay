#pragma once

#include "MinHook.h"
#include "../utils.hpp"

namespace AudioChannelControl
{
	static void* audioChannel;
	static float speed = 1.f;

	inline void* (__stdcall* setVolume)(void*, float);
	void* __stdcall setVolumeHook(void*, float);
	inline void* (__stdcall* setFrequency)(void*, float);

	inline int(__thiscall* playLayerResetLevel)(void*);
	int __fastcall playLayerResetLevelHook(void* self, void*);

	void set(float);
	void setPitch(float);
	void initHooks();
}
