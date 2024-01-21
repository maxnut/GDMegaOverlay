#pragma once

#include <iostream>
#include <vector>

class Clickpack
{
public:
	std::string name = "None";
	std::vector<std::string> clicks, softclicks, releases, platClicks, platReleases;
	std::string noise = "";

	static Clickpack fromPath(std::string path);
};

namespace Clickpacks
{
	inline Clickpack currentClickpack;

	void init();
	void drawGUI();
}
