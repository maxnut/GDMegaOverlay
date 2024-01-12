#include "Clickpacks.h"
#include "../Common.h"
#include "../Settings.h"

#include "../GUI/GUI.h"
#include "../portable-file-dialogs.h"
#include <filesystem>

namespace fs = std::filesystem;

Clickpack Clickpack::fromPath(std::string pathString)
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

        if(!fs::exists(folder))
            return;

		for (const auto& entry : fs::directory_iterator(folder))
		{
			if (entry.is_regular_file() && (entry.path().extension() == ".wav"))
			{
				vector.push_back(entry.path().string());
			}
		}
	};

	addFromPath(pack.clicks, clickPath);
	addFromPath(pack.softclicks, softclickPath);
	addFromPath(pack.releases, releasePath);
	addFromPath(pack.platClicks, platClickPath);
    addFromPath(pack.platReleases, platReleasePath);

    if(fs::exists(pathString + "\\noise.wav"))
        pack.noise = pathString + "\\noise.wav";

	return pack;
}

void Clickpacks::init()
{
	std::string clickPath = Settings::get<std::string>("clickpacks/path", "");

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
            std::string clickPath = Settings::get<std::string>("clickpacks/path", "");

			const auto result = pfd::select_folder("Choose a folder", "GDMO\\clickpacks").result();

			if (!result.empty())
            {
				currentClickpack = Clickpack::fromPath(result);
                Settings::set<std::string>("clickpacks/path", result);
            }
		}

        GUI::checkbox("Use Noise", Settings::get<bool*>("clickpacks/noise/enabled"));

        float clickVolume = Settings::get<float>("clickpacks/click/volume", 2.f);
        GUI::inputFloat("Click Volume", &clickVolume);

        if(ImGui::IsItemDeactivatedAfterEdit())
            Settings::set<float>("clickpacks/click/volume", clickVolume);


        float noiseVolume = Settings::get<float>("clickpacks/noise/volume", 1.f);
        GUI::inputFloat("Noise Volume", &noiseVolume);

        if(ImGui::IsItemDeactivatedAfterEdit())
            Settings::set<float>("clickpacks/noise/volume", noiseVolume);
        

        float softclickAt = Settings::get<float>("clickpacks/softclicks_at", 0.1f);
        GUI::inputFloat("Softclicks at", &softclickAt);

        if(ImGui::IsItemDeactivatedAfterEdit())
            Settings::set<float>("clickpacks/softclicks_at", softclickAt);
	});
}