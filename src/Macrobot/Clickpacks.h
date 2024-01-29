#pragma once

#include <iostream>
#include <vector>

class Clickpack
{
public:
	std::string name = "None";
	std::vector<ghc::filesystem::path> clicks, softclicks, releases, platClicks, platReleases;

	static Clickpack fromPath(const ghc::filesystem::path& path);
};

namespace Clickpacks
{
	inline Clickpack currentClickpack;

	void init();
	void drawGUI();
}
