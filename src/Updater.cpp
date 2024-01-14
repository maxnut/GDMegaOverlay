#include "Updater.h"
#include "Common.h"
#include "GUI/GUI.h"
#include "Settings.h"
#include "json.hpp"
#include "subprocess.hpp"

#include <curl/curl.h>

void Updater::checkForUpdate()
{
	std::string data;

	if (Common::getRequest("https://api.github.com/repos/maxnut/GDMegaOverlay/releases/latest", &data, "maxnut") !=
		CURLE_OK)
		return;

	int id = Settings::get<int>("updater/id", -1);

	request = json::parse(data);

	if (request["id"] == id)
		return;

	hasUpdate = true;
}

void Updater::draw()
{
	if (!hasUpdate)
		return;

	GUI::ButtonFunc yes("Yes", [&] {
        Settings::set<int>("updater/id", request["id"]);
		auto process = subprocess::Popen("GDMO/Updater/GDMOUpdater");
		try
		{
			process.close();
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << '\n';
		}
		hasUpdate = false;
	});

	GUI::ButtonFunc no("No", [&] { hasUpdate = false; });

	GUI::alertPopup("Update Available", "Do you want to download this update?\n" + request["body"].get<std::string>(),
					yes, no);
}