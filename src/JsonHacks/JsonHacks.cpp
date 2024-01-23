#include "JsonHacks.h"
#include "../GUI/GUI.h"
#include "../util.hpp"
#include <fstream>
#include <imgui.h>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// TODO: add JsonHacks::find

void JsonHacks::load()
{
	auto read_or_default = [](std::string modsName, nlohmann::json& mods) {

		std::string path = Mod::get()->getResourcesDir().string() + "/" + modsName;

		std::ifstream file(path);

		if (file.peek() == std::ifstream::traits_type::eof())
		{
			file.close();
			std::ofstream file_write(path);
			file_write << mods;
			file_write.close();
		}
		else
			file >> mods;

		for (std::size_t i = 0; i < mods["mods"].size(); i++)
		{
			nlohmann::json& mod = mods["mods"][i];

			if (!mod.contains("toggle"))
				mod["toggle"] = false;

			toggleHack(mods, i, false);
		}
	};

	bypass = nlohmann::json::object();
	creator = nlohmann::json::object();
	global = nlohmann::json::object();
	level = nlohmann::json::object();
	player = nlohmann::json::object();
	variables = nlohmann::json::object();

	read_or_default("bypass.json", bypass);
	read_or_default("creator.json", creator);
	read_or_default("global.json", global);
	read_or_default("level.json", level);
	read_or_default("player.json", player);
	read_or_default("variables.json", variables);
}

void JsonHacks::toggleHack(nlohmann::json& mods, std::size_t index, bool toggle)
{
	nlohmann::json& mod = mods["mods"][index];

	if (toggle)
		mod["toggle"] = !mod["toggle"];

	for (std::size_t i = 0; i < mod["opcodes"].size(); i++)
	{
		unsigned long address = std::stoul(mod["opcodes"][i]["address"].get<std::string>(), nullptr, 16);
		std::string opcode = mod["opcodes"][i][mod["toggle"].get<bool>() ? "on" : "off"].get<std::string>();
		bool vp = mod["opcodes"][i].contains("vp") ? mod["opcodes"][i]["vp"].get<bool>() : false;

		if (mod["opcodes"][i].contains("lib") && mod["opcodes"][i]["lib"].get<std::string>() == "libcocos2d.dll")
			util::writeBytes(base::getCocos() + address, util::hexToBytes(opcode), vp);
		else
			util::writeBytes(base::get() + address, util::hexToBytes(opcode), vp);
	}
}

void JsonHacks::drawFromJson(nlohmann::json& mods)
{
	for (std::size_t i = 0; i < mods["mods"].size(); i++)
	{
		nlohmann::json& mod = mods["mods"][i];

		if (!mod.contains("toggle"))
			mod["toggle"] = false;

		if (GUI::checkbox(mod["name"].get<std::string>().c_str(), mod["toggle"].get<bool*>()))
			toggleHack(mods, i, false);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(mod["description"].get<std::string>().c_str());
	}
}

void JsonHacks::save()
{
	auto write_mods = [](std::string modsName, nlohmann::json& mods) {
		std::string path = Mod::get()->getResourcesDir().string() + "/" + modsName;
		std::ofstream file_write(path);
		file_write << mods.dump(4);
		file_write.close();
	};

	write_mods("bypass.json", bypass);
	write_mods("creator.json", creator);
	write_mods("global.json", global);
	write_mods("level.json", level);
	write_mods("player.json", player);
	write_mods("variables.json", variables);
}
