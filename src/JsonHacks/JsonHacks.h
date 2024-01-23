#pragma once
#include <nlohmann/json.hpp>

namespace JsonHacks
{
	inline nlohmann::json bypass;
	inline nlohmann::json creator;
	inline nlohmann::json global;
	inline nlohmann::json level;
	inline nlohmann::json player;
	inline nlohmann::json variables;

	void load();
	void toggleHack(nlohmann::json&, std::size_t, bool);
	void drawFromJson(nlohmann::json&);
	void save();
}
