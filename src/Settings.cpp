#include <fstream>
#include <sstream>
#include "json.hpp"
#include "Settings.h"
#include "Common.h"
#include "utils.hpp"

void Settings::dumpCacheToJson()
{
	for (auto const& [key, _] : cache)
	{
		if (
			auto split_name = utils::split(key, "/");
			split_name.size() > 1
		) {
			nlohmann::json* token = &settingsJson;

			for (unsigned int i = 0; i < split_name.size(); i++)
				token = &token->at(split_name[i]);

			*token = cache.get(key);
		}
	}
}

void Settings::save()
{
	// if (!shouldSave) return;

	dumpCacheToJson();

	std::ofstream f("GDMO\\settings.json");
	f << settingsJson.dump(4);
	f.close();

	shouldSave = false;
}

void Settings::load()
{
	std::ifstream f("GDMO\\settings.json");
	if (f)
	{
		std::stringstream buffer;
		buffer << f.rdbuf();
		settingsJson = nlohmann::json::parse(buffer.str());
		buffer.clear();
	}
	f.close();
}
