#pragma once

#include <sndfile.h>
#include <fmod.hpp>

namespace AudioRecord
{
	inline FMOD::DSP* dsp;
	inline bool recording = false;

	void start();
	void stop();
}