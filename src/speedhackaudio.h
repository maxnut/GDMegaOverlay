#include "pch.h"

namespace SpeedhackAudio
{

static void set(float speed)
{
	auto engine = gd::FMODAudioEngine::sharedEngine();

	float frequency;

	engine->m_pSound->getDefaults(&frequency, nullptr);

	engine->m_pGlobalChannel->setFrequency(frequency * speed);
}
} // namespace SpeedhackAudio
