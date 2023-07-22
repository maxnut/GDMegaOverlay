#include "pch.h"

namespace SpeedhackAudio {

	
	static void* channel;
	static float speed;
	static bool initialized = false;

	// setfrequency
	// setvolume

	static void* (__stdcall* setVolume)(void* t_channel, float volume);
	static void* (__stdcall* setFrequency)(void* t_channel, float frequency);

	static void* __stdcall setVolumeHook(void* t_channel, float volume) {
		channel = t_channel;

		if (speed != 1.f) {
			setFrequency(channel, speed);

		}
		return setVolume(channel, volume);
	}

	static void init() {
		if (initialized)
			return;

		setFrequency = (decltype(setFrequency))GetProcAddress(GetModuleHandle("fmod.dll"), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
		DWORD hkAddr = (DWORD)GetProcAddress(GetModuleHandle("fmod.dll"), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");

		MH_CreateHook(
			(PVOID)hkAddr,
			setVolumeHook,
			(PVOID*)&setVolume
		);

		speed = 1.f;
		initialized = true;
	}

	static void set(float frequency) {
		if (!initialized)
			init();

		if (channel == nullptr)
			return;

		speed = frequency;
		setFrequency(channel, frequency);
	}
}
