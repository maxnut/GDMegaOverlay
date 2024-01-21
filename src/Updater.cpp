#include "Updater.h"
#include "Common.h"
#include "GUI/GUI.h"
#include "Settings.h"
#include "json.hpp"
#include "subprocess.hpp"

#include <Geode/cocos/platform/third_party/win32/curl/curl.h>

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
		Settings::save();

		char pBuf[256];
		size_t len = sizeof(pBuf);
		int bytes = GetModuleFileName(NULL, pBuf, len);
		std::filesystem::path p = pBuf;
		p = p.parent_path();

		auto gdpath = p.string();

		std::stringstream ss;
		ss << '"' << gdpath << "/GDMO/Updater/GDMOUpdater.exe" << '"';
		std::cout << ss.str() << std::endl;
		std::system(ss.str().c_str());
	});

	GUI::ButtonFunc no("No", [&] { hasUpdate = false; });

	GUI::alertPopup("Update Available", "Do you want to download this update?\n" + request["body"].get<std::string>(),
					yes, no);
}