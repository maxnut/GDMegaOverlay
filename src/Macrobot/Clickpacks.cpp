#include "Clickpacks.h"
#include "../Common.h"


#include "../GUI/GUI.h"
#include <portable-file-dialogs.h>
#include <filesystem>

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace fs = std::filesystem;

Clickpack Clickpack::fromPath(const std::string& pathString)
{
	Clickpack pack;
	fs::path path = pathString;

	pack.name = path.filename().string();

	fs::path clickPath = pathString + "\\clicks";
	fs::path softclickPath = pathString + "\\softclicks";
	fs::path releasePath = pathString + "\\releases";
	fs::path platClickPath = pathString + "\\plat_clicks";
	fs::path platReleasePath = pathString + "\\plat_releases";

	auto addFromPath = [](std::vector<std::string>& vector, fs::path folder) {
		if (!fs::exists(folder))
			return;

		for (const auto& entry : fs::directory_iterator(folder))
			if (entry.is_regular_file() && (entry.path().extension() == ".wav"))
				vector.push_back(entry.path().string());
	};

	addFromPath(pack.clicks, clickPath);
	addFromPath(pack.softclicks, softclickPath);
	addFromPath(pack.releases, releasePath);
	addFromPath(pack.platClicks, platClickPath);
	addFromPath(pack.platReleases, platReleasePath);

	return pack;
}

void Clickpacks::init()
{
	std::string clickPath = Mod::get()->getSavedValue<std::string>("clickpacks/path", "");

	if (clickPath != "")
		currentClickpack = Clickpack::fromPath(clickPath);
}

void Clickpacks::drawGUI()
{
	GUI::modalPopup("Clickpacks", [] {
		if (GUI::shouldRender())
			ImGui::Text(("Current pack: " + currentClickpack.name).c_str());

		if (GUI::button("Select clickpack"))
		{
			std::string clickPath = Mod::get()->getSavedValue<std::string>("clickpacks/path", "");

			const auto result = pfd::select_folder("Choose a folder", Mod::get()->getSaveDir().string() + "\\clickpacks").result();

			if (!result.empty())
			{
				currentClickpack = Clickpack::fromPath(result);
				Mod::get()->setSavedValue<std::string>("clickpacks/path", result);
			}
		}

		float clickVolume = Mod::get()->getSavedValue<float>("clickpacks/click/volume", 2.f);
		GUI::inputFloat("Click Volume", &clickVolume);

		if (ImGui::IsItemDeactivatedAfterEdit())
			Mod::get()->setSavedValue<float>("clickpacks/click/volume", clickVolume);
		

		float softclickAt = Mod::get()->getSavedValue<float>("clickpacks/softclicks_at", 0.1f);
		GUI::inputFloat("Softclicks at", &softclickAt);

		if (ImGui::IsItemDeactivatedAfterEdit())
			Mod::get()->setSavedValue<float>("clickpacks/softclicks_at", softclickAt);
	});
}