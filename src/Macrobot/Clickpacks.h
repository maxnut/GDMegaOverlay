#pragma once

#include <iostream>
#include <vector>

#include <Geode/fmod/fmod.hpp>

class Clickpack
{
public:
	std::string name = "None";
	std::vector<FMOD::Sound*> clicks, softclicks, releases, platClicks, platReleases;

	FMOD::Sound* randomClick();
	FMOD::Sound* randomSoftClick();
	FMOD::Sound* randomRelease();
	FMOD::Sound* randomPlatClick();
	FMOD::Sound* randomPlatRelease();

	static Clickpack fromPath(const ghc::filesystem::path& path);
};

namespace Clickpacks
{
	inline Clickpack currentClickpack;

	void init();
	void drawGUI();
}
