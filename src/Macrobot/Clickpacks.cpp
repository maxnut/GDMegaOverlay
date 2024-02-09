#include "Clickpacks.h"
#include "../Common.h"


#include "../GUI/GUI.h"
#include "../Settings.hpp"
#include <portable-file-dialogs.h>
#include <filesystem>

#include <Geode/Geode.hpp>
#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;

namespace fs = ghc::filesystem;

std::optional<Clickpack> Clickpack::fromPath(const fs::path& pathString)
{
	Clickpack pack;
	fs::path path = pathString;

	pack.name = path.filename().string();

	fs::path clickPath = pathString / "clicks";
	fs::path softclickPath = pathString / "softclicks";
	fs::path releasePath = pathString / "releases";
	fs::path platClickPath = pathString / "plat_clicks";
	fs::path platReleasePath = pathString / "plat_releases";

	auto addFromPath = [](std::vector<FMOD::Sound*>& vector, fs::path folder) {
		if (!fs::exists(folder))
			return;

		for (const auto& entry : fs::directory_iterator(folder))
			if (entry.is_regular_file() && (entry.path().extension() == ".wav"))
			{
				FMOD::Sound* s = nullptr;
				FMODAudioEngine::sharedEngine()->m_system->createSound(string::wideToUtf8(entry.path().wstring()).c_str(), FMOD_LOOP_OFF, 0, &s);
				vector.push_back(s);
			}
	};

	addFromPath(pack.clicks, clickPath);
	addFromPath(pack.softclicks, softclickPath);
	addFromPath(pack.releases, releasePath);
	addFromPath(pack.platClicks, platClickPath);
	addFromPath(pack.platReleases, platReleasePath);

	if(pack.clicks.size() <= 0 && pack.softclicks.size() <= 0 && pack.releases.size() <= 0 && pack.platClicks.size() <= 0 && pack.platReleases.size() <= 0)
		return std::nullopt;

	return pack;
}

void Clickpacks::init()
{
	std::string clickPath = Settings::get<std::string>("clickpacks/path");

	if (clickPath != "")
	{
		auto clickpackOpt = Clickpack::fromPath(clickPath);
		if(clickpackOpt.has_value())
			currentClickpack = clickpackOpt.value();
	}
}

void Clickpacks::drawGUI()
{
	GUI::modalPopup("Clickpacks", [] {
		if (GUI::shouldRender())
			ImGui::Text(("Current pack: " + currentClickpack.name).c_str());

		if (GUI::button("Select clickpack"))
		{
			std::string clickPath = Settings::get<std::string>("clickpacks/path");

			const auto result = pfd::select_folder("Choose a folder", (Mod::get()->getSaveDir() / "clickpacks").string()).result();

			if (!result.empty())
			{
				auto res = Clickpack::fromPath(result);
				if(res.has_value())
				{
					currentClickpack = res.value();
					Mod::get()->setSavedValue<std::string>("clickpacks/path", result);
				}
				else
					FLAlertLayer::create("Error", "The folder is not a valid clickpack!", "Ok")->show();
			}
		}

		float clickVolume = Settings::get<float>("clickpacks/click/volume", 1.f);
		GUI::inputFloat("Click Volume", &clickVolume);

		if (ImGui::IsItemDeactivatedAfterEdit())
			Mod::get()->setSavedValue<float>("clickpacks/click/volume", clickVolume);

		float minPitch = Settings::get<float>("clickpacks/click/min_pitch", 0.98f);
		GUI::inputFloat("Min Pitch", &minPitch);

		if (ImGui::IsItemDeactivatedAfterEdit())
			Mod::get()->setSavedValue<float>("clickpacks/click/min_pitch", minPitch);

		float maxPitch = Settings::get<float>("clickpacks/click/max_pitch", 1.02f);
		GUI::inputFloat("Max Pitch", &maxPitch);

		if (ImGui::IsItemDeactivatedAfterEdit())
			Mod::get()->setSavedValue<float>("clickpacks/click/max_pitch", maxPitch);

		float softclickAt = Settings::get<float>("clickpacks/softclicks_at", 0.1f);
		GUI::inputFloat("Softclicks at", &softclickAt);

		if (ImGui::IsItemDeactivatedAfterEdit())
			Mod::get()->setSavedValue<float>("clickpacks/softclicks_at", softclickAt);
	});
}

FMOD::Sound* Clickpack::randomClick()
{
	if(clicks.size() <= 0)
		return nullptr;

	return clicks[util::randomInt(0, clicks.size() - 1)];
}
FMOD::Sound* Clickpack::randomSoftClick()
{
	if(softclicks.size() <= 0)
		return nullptr;

	return softclicks[util::randomInt(0, softclicks.size() - 1)];
}
FMOD::Sound* Clickpack::randomRelease()
{
	if(releases.size() <= 0)
		return nullptr;

	return releases[util::randomInt(0, releases.size() - 1)];
}
FMOD::Sound* Clickpack::randomPlatClick()
{
	if(platClicks.size() <= 0)
		return nullptr;

	return platClicks[util::randomInt(0, platClicks.size() - 1)];
}
FMOD::Sound* Clickpack::randomPlatRelease()
{
	if(platReleases.size() <= 0)
		return nullptr;

	return platReleases[util::randomInt(0, platReleases.size() - 1)];
}