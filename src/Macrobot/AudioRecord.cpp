#include "AudioRecord.h"
#include "../Common.h"
#include "../Hacks/AudioChannelControl.h"

#include <iostream>

void AudioRecord::start()
{
	auto system = MBO(FMOD::System*, Common::FMODAudioEngine, 400);
	system->setOutput(FMOD_OUTPUTTYPE_WAVWRITER);
	recording = true;
}

void AudioRecord::stop()
{
	auto system = MBO(FMOD::System*, Common::FMODAudioEngine, 400);
	system->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
    recording = false;
}