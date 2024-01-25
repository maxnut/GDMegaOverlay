#include "JsonPatches.h"
#include "../GUI/GUI.h"
#include "../util.hpp"
#include <fstream>
#include <imgui.h>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

void JsonPatches::init()
{
	auto append_patches = [](JsonPatches::patch_group_type& patchGroup, nlohmann::json& mods)
	{
		for (const auto& jsonPatch : mods)	
		{
			JPatch jpatch{};

			jpatch.description = jsonPatch["description"];

			for (const auto& patch : jsonPatch["opcodes"])
			{
				bool isCocos = patch.contains("lib") && patch["lib"].get<std::string>() == "libcocos2d.dll";
				unsigned long address = std::stoul(patch["address"].get<std::string>(), nullptr, 16);

				const std::uintptr_t base = isCocos ? base::getCocos() : base::get();

				auto gpatch = Mod::get()->patch(reinterpret_cast<void*>(base + address), util::hexToBytes(patch["on"])).unwrap();
				gpatch->disable();

				jpatch.patches.push_back(gpatch);
			}

			patchGroup[jsonPatch["name"]] = jpatch;

			togglePatch(patchGroup, jsonPatch["name"]);
		}
	};

	auto read_or_default = [](std::string modsName, nlohmann::json& mods)
	{
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
	};

	nlohmann::json patchesJson = nlohmann::json::object();

	read_or_default("bypass.json", patchesJson);
	append_patches(JsonPatches::bypass, patchesJson["mods"]);
	patchesJson = nlohmann::json::object();

	read_or_default("creator.json", patchesJson);
	append_patches(JsonPatches::creator, patchesJson["mods"]);
	patchesJson = nlohmann::json::object();

	read_or_default("global.json", patchesJson);
	append_patches(JsonPatches::global, patchesJson["mods"]);
	patchesJson = nlohmann::json::object();

	read_or_default("level.json", patchesJson);
	append_patches(JsonPatches::level, patchesJson["mods"]);
	patchesJson = nlohmann::json::object();

	read_or_default("player.json", patchesJson);
	append_patches(JsonPatches::player, patchesJson["mods"]);
	patchesJson = nlohmann::json::object();

	// read_or_default("variables.json", patchesJson);
	// append_patches(JsonPatches::variables, patchesJson["mods"]);
}

void JsonPatches::togglePatch(JsonPatches::patch_group_type& patchGroup, std::string name)
{
	auto patches = patchGroup.at(name).patches;

	if (Mod::get()->getSavedValue<bool>("patch/" + name, false))
	{
		for (const auto& patch : patches)
			if (!patch->isEnabled())
				patch->enable();
	}
	else
	{
		for (const auto& patch : patches)
			if (patch->isEnabled())
				patch->disable();
	}
}

void JsonPatches::togglePatch(JsonPatches::patch_group_type& patchGroup, std::string name, bool value)
{
	Mod::get()->setSavedValue<bool>("patch/" + name, value);

	togglePatch(patchGroup, name);
}

void JsonPatches::drawFromPatches(JsonPatches::patch_group_type& patchGroup)
{
	for (const auto& [name, jpatch] : patchGroup)
	{
		if (GUI::checkbox(name, "patch/" + name, false))
			togglePatch(patchGroup, name);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(jpatch.description.c_str());
	}
}
