#include "AudioRecord.h"
#include "../Common.h"
#include "../Hacks/AudioChannelControl.h"

#include <iostream>

#include <Geode/binding/FMODAudioEngine.hpp>

void AudioRecord::start()
{
	FMODAudioEngine::sharedEngine()->m_system->setOutput(FMOD_OUTPUTTYPE_WAVWRITER);
	recording = true;
}

void AudioRecord::stop()
{
	FMODAudioEngine::sharedEngine()->m_system->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
	recording = false;
}