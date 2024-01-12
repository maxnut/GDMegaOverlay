#include "AudioChannelControl.h"
#include "../Common.h"
#include "fmod_dsp_effects.h"
#include <fmod.hpp>

void* __stdcall AudioChannelControl::setVolumeHook(FMOD::Channel* channel, float volume)
{
	audioChannel = channel;

	if (speed != 1.f)
		setFrequency(channel, speed);

	return setVolume(channel, volume);
}

void AudioChannelControl::set(float frequency)
{
	if (audioChannel == nullptr)
		return;

	speed = frequency;
	setFrequency(audioChannel, frequency);
}

void AudioChannelControl::setPitch(float pitch)
{
	auto system = MBO(FMOD::System*, Common::FMODAudioEngine, 400);

	if (!audioChannel || !system)
		return;

	static FMOD::DSP* pitchShifter;

	if (pitchShifter)
	{
		audioChannel->removeDSP(pitchShifter);
		pitchShifter->release();
		pitchShifter = nullptr;
	}

	if (pitch == 1.f)
		return;

	system->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pitchShifter);

	pitchShifter->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, 4096);
	pitchShifter->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, pitch);

	audioChannel->addDSP(0, pitchShifter);
}

int __fastcall AudioChannelControl::playLayerResetLevelHook(void* self, void*)
{
	int value = playLayerResetLevel(self);
	Common::onAudioPitchChange();

	return value;
}

void AudioChannelControl::initHooks()
{
	setFrequency = reinterpret_cast<decltype(setFrequency)>(GetProcAddress(
		reinterpret_cast<HMODULE>(utils::fmod_base), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));
	void* setVolumeAddress = reinterpret_cast<void*>(GetProcAddress(
		reinterpret_cast<HMODULE>(utils::fmod_base), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));

	MH_CreateHook(reinterpret_cast<void*>(setVolumeAddress), setVolumeHook, reinterpret_cast<void**>(&setVolume));

	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x2EA130), playLayerResetLevelHook,
				  reinterpret_cast<void**>(&playLayerResetLevel));

	speed = 1.f;
}
