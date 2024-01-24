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
	channel->setFrequency(44100 * speed);

	return channel->setVolume(volume);
}

void AudioChannelControl::set(float frequency)
{
	speed = frequency;

	FMOD::Channel* audioChannel;

	for(int i = 0; i < 2; i++)
	{
		FMODAudioEngine::sharedEngine()->m_system->getChannel(126 + i, &audioChannel);
		if(audioChannel)
			audioChannel->setFrequency(44100 * frequency);
	}
}

void AudioChannelControl::setPitch(float pitch)
{
	auto system = FMODAudioEngine::sharedEngine()->m_system;
	FMOD::ChannelGroup* group;
	system->getMasterChannelGroup(&group);

	static FMOD::DSP* pitchShifter = nullptr;

	if (pitchShifter)
	{
		group->removeDSP(pitchShifter);
		pitchShifter->release();
		pitchShifter = nullptr;
	}
	
	if (pitch == 1.f)
		return;
	
	system->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pitchShifter);
	pitchShifter->setParameterFloat(FMOD_DSP_PITCHSHIFT_FFTSIZE, 4096);
	pitchShifter->setParameterFloat(FMOD_DSP_PITCHSHIFT_PITCH, pitch);
	group->addDSP(0, pitchShifter);
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
	void* setVolumeAddress = reinterpret_cast<void*>(GetProcAddress(
		reinterpret_cast<HMODULE>(util::fmod_base), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));

	Mod::get()->hook(setVolumeAddress, &AudioChannelControl::setVolumeHook, "setVolumeHook", tulip::hook::TulipConvention::Stdcall);

	AudioChannelControl::speed = 1.f;
}
