#include "JsonHacks.h"
#include "../GUI/GUI.h"
#include "../utils.hpp"
#include <fstream>
#include <imgui.h>

void JsonHacks::load()
{
	auto read_or_default = [](std::string modsName, nlohmann::json& mods) {
		std::ifstream file(modsName);

		if (file.peek() == std::ifstream::traits_type::eof())
		{
			file.close();
			std::ofstream file_write(modsName);
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

	read_or_default("GDMO\\mod\\bypass.json", bypass);
	read_or_default("GDMO\\mod\\creator.json", creator);
	read_or_default("GDMO\\mod\\global.json", global);
	read_or_default("GDMO\\mod\\level.json", level);
	read_or_default("GDMO\\mod\\player.json", player);
	read_or_default("GDMO\\mod\\variables.json", variables);
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

		if (mod["opcodes"][i].contains("lib") && mod["opcodes"][i]["lib"].get<std::string>() == "libcocos2d.dll")
			utils::writeBytes(utils::cc_base + address, utils::hexToBytes(opcode));
		else
			utils::writeBytes(utils::gd_base + address, utils::hexToBytes(opcode));
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
		std::ofstream file_write(modsName);
		file_write << mods.dump(4);
		file_write.close();
	};

	write_mods("GDMO\\mod\\bypass.json", bypass);
	write_mods("GDMO\\mod\\creator.json", creator);
	write_mods("GDMO\\mod\\global.json", global);
	write_mods("GDMO\\mod\\level.json", level);
	write_mods("GDMO\\mod\\player.json", player);
	write_mods("GDMO\\mod\\variables.json", variables);
}
