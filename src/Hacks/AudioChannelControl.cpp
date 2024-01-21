#include "AudioChannelControl.h"
#include "../Common.h"

#include <Geode/Geode.hpp>
#include "Geode/fmod/fmod_dsp_effects.h"
#include <Geode/fmod/fmod.hpp>

#include <Geode/modify/PlayLayer.hpp>

#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;

FMOD_RESULT AudioChannelControl::setVolumeHook(FMOD::Channel* channel, float volume)
{
	audioChannel = channel;

	if (speed != 1.f)
		setFrequency(channel, speed);

	return channel->setVolume(volume);
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
	auto system = FMODAudioEngine::sharedEngine()->m_system;

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

class $modify(PlayLayer)
{
	void resetLevel()
	{
		PlayLayer::resetLevel();
		Common::onAudioPitchChange();
	}
};

$execute
{
	AudioChannelControl::setFrequency = reinterpret_cast<decltype(AudioChannelControl::setFrequency)>(GetProcAddress(
		reinterpret_cast<HMODULE>(util::fmod_base), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));
	void* setVolumeAddress = reinterpret_cast<void*>(GetProcAddress(
		reinterpret_cast<HMODULE>(util::fmod_base), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));

	Mod::get()->hook(setVolumeAddress, &AudioChannelControl::setVolumeHook, "setVolumeHook", tulip::hook::TulipConvention::Stdcall);

	AudioChannelControl::speed = 1.f;
}
