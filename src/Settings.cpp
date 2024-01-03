#include "Settings.h"
#include "Common.h"

#include <fstream>
#include <sstream>
#include <string_view>

void Settings::save()
{
	std::ofstream f("GDMO\\settings.json");
	f << settingsJson.dump(4);
	f.close();
}

void Settings::load()
{
	std::ifstream f("GDMO\\settings.json");
	if (f)
	{
		std::stringstream buffer;
		buffer << f.rdbuf();
		settingsJson = json::parse(buffer.str());
		buffer.clear();
	}
	f.close();
}