#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include <Geode/loader/Hook.hpp>

namespace JsonPatches
{
	// TODO: rename this to something else
	struct JPatch
	{
		std::string description;
		std::vector<geode::Patch*> patches;

		JPatch(std::string description, std::vector<geode::Patch*> patches)
			: description(description), patches(patches)
		{}

		JPatch() = default;
	};

	using patch_group_type = std::unordered_map<std::string, JPatch>;

	inline patch_group_type bypass;
	inline patch_group_type creator;
	inline patch_group_type global;
	inline patch_group_type level;
	inline patch_group_type player;
	inline patch_group_type variables;

	void init();
	void togglePatch(patch_group_type&, std::string);
	void togglePatch(patch_group_type&, std::string, bool);
	void drawFromPatches(patch_group_type&);
}
