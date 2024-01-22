#include "imgui_internal.h"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include "ConstData.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui-cocos.hpp>
#include "json.hpp"
#include <filesystem>
#include <fstream>

#include "Common.h"
#include "DiscordRPCManager/DiscordRPCManager.h"
#include "GUI/GUI.h"
#include "Hacks/AudioChannelControl.h"
#include "Hacks/Labels.h"
#include "Hacks/ReplayLastCheckpoint.h"
#include "Hacks/Speedhack.h"
#include "Hacks/StartposSwitcher.h"
#include "JsonHacks/JsonHacks.h"
#include "Macrobot/Clickpacks.h"
#include "Macrobot/Macrobot.h"
#include "Macrobot/Record.h"
#include "Settings.h"
#include "Updater.h"

void init()
{
#ifdef DEV_CONSOLE
	if (AllocConsole())
	{
		freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
		SetConsoleTitleW(L"GDMegaOverlay");
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);
	}
#endif

	if (!std::filesystem::exists("GDMO"))
		std::filesystem::create_directory("GDMO");
	if (!std::filesystem::exists("GDMO\\mod"))
		std::filesystem::create_directory("GDMO\\mod");
	if (!std::filesystem::exists("GDMO\\renders"))
		std::filesystem::create_directory("GDMO\\renders");
	if (!std::filesystem::exists("GDMO\\clickpacks"))
		std::filesystem::create_directory("GDMO\\clickpacks");

	Settings::load();
	JsonHacks::load();
	GUI::init();
	DiscordRPCManager::init();

	GUI::setLateInit([] {
		Common::calculateFramerate();
		Common::setPriority();
		Common::onAudioSpeedChange();
		Common::onAudioPitchChange();
		Common::loadIcons();
		Clickpacks::init();

		if (Settings::get<bool>("menu/updates/check_on_start", true))
			Updater::checkForUpdate();
	});

	GUI::Window generalWindow("General", [] {
		float framerate = Settings::get<float>("general/fps/value", 60.f);

		if (GUI::inputFloat("##FPSValue", &framerate))
			Settings::set<float>("general/fps/value", framerate);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::calculateFramerate();

			ImGui::SameLine();
		}

		if (GUI::checkbox("FPS", Settings::get<bool*>("general/fps/enabled")))
			Common::calculateFramerate();

		float speedhack = Settings::get<float>("general/speedhack/value", 60.f);
		if (GUI::inputFloat("##SpeedhackValue", &speedhack))
			Settings::set<float>("general/speedhack/value", speedhack);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::calculateFramerate();

			ImGui::SameLine();
		}

		if (GUI::checkbox("Speedhack", Settings::get<bool*>("general/speedhack/enabled")))
			Common::calculateFramerate();

		float pitch = Settings::get<float>("general/music/pitch/value", 1.f);
		if (GUI::inputFloat("##PitchShiftValue", &pitch, 0.5f, 2.f))
			Settings::set<float>("general/music/pitch/value", pitch);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::onAudioPitchChange();

			ImGui::SameLine();
		}
		if (GUI::checkbox("Pitch Shift", Settings::get<bool*>("general/music/pitch/enabled")))
			Common::onAudioPitchChange();

		float music_speed = Settings::get<float>("general/music/speed/value", 1.f);
		if (GUI::inputFloat("##MusicSpeedValue", &music_speed))
			Settings::set<float>("general/music/speed/value", music_speed);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::onAudioSpeedChange();

			ImGui::SameLine();
		}

		if (Settings::get<bool>("general/tie_to_game_speed/music/enabled"))
		{
			Settings::set<bool>("general/music/speed/enabled", false);

			ImGui::BeginDisabled();
			GUI::checkbox("Music Speed", Settings::get<bool*>("general/music/speed/enabled"));
			ImGui::EndDisabled();
		}
		else
		{
			if (GUI::checkbox("Music Speed", Settings::get<bool*>("general/music/speed/enabled")))
				Common::onAudioSpeedChange();
		}

		if (GUI::checkbox("Tie Music To Game Speed", Settings::get<bool*>("general/tie_to_game_speed/music/enabled")))
			Common::onAudioSpeedChange();

		int priority = Settings::get<int>("general/priority", 2);

		if (GUI::combo("Thread Priority", &priority, priorities, 5))
		{
			Settings::set<int>("general/priority", priority);
			Common::setPriority();
		}
	});
	generalWindow.position = {50, 50};
	generalWindow.size.y = 250;
	GUI::addWindow(generalWindow);

	GUI::Window bypassWindow("Bypass", [] { JsonHacks::drawFromJson(JsonHacks::bypass); });
	bypassWindow.position = {1050, 50};
	bypassWindow.size.y = 180;
	GUI::addWindow(bypassWindow);

	GUI::Window creatorWindow("Creator", [] { JsonHacks::drawFromJson(JsonHacks::creator); });
	creatorWindow.position = {1300, 50};
	creatorWindow.size.y = 200;
	GUI::addWindow(creatorWindow);

	GUI::Window globalWindow("Global", [] {
		GUI::checkbox("Discord Rich Presence", Settings::get<bool*>("general/discordrpc/enabled"));

		JsonHacks::drawFromJson(JsonHacks::global);
	});
	globalWindow.position = {300, 50};
	globalWindow.size.y = 230;
	GUI::addWindow(globalWindow);

	GUI::Window levelWindow("Level", [] {
		GUI::checkbox("Startpos Switcher", Settings::get<bool*>("level/startpos_switcher"));

		GUI::arrowButton("Startpos Switcher Settings");
		GUI::modalPopup(
			"Startpos Switcher Settings",
			[] {
				if (GUI::button("Switch Left"))
					StartposSwitcher::change(false);

				if (GUI::button("Switch Right"))
					StartposSwitcher::change(true);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		GUI::checkbox("Replay Last Checkpoint", Settings::get<bool*>("level/replay_checkpoint"));

		JsonHacks::drawFromJson(JsonHacks::level);
	});
	levelWindow.position = {550, 50};
	levelWindow.size.y = 180;
	GUI::addWindow(levelWindow);

	GUI::Window menuSettings("Menu Settings", [] {
		int snap = Settings::get<int>("menu/window/snap", 10);
		int snapSize = Settings::get<int>("menu/window/size_snap", 10);
		float transitionDuration = Settings::get<float>("menu/transition_duration", 0.35f);
		float windowOpacity = Settings::get<float>("menu/window/opacity", 0.98f);

		if (GUI::dragInt("Window Snap", &snap, 0))
			Settings::set<int>("menu/window/snap", snap);

		if (GUI::dragInt("Size Snap", &snapSize, 0))
			Settings::set<int>("menu/window/size_snap", snapSize);

		if (GUI::dragFloat("Transition Duration", &transitionDuration, 0))
			Settings::set<float>("menu/transition_duration", transitionDuration);

		if (GUI::inputFloat("Window Opacity", &windowOpacity, 0.f, 1.f))
			Settings::set<float>("menu/window/opacity", windowOpacity);

		GUI::checkbox("Rainbow Menu", Settings::get<bool*>("menu/window/rainbow/enabled"));

		GUI::arrowButton("Rainbow Menu Settings");
		GUI::modalPopup(
			"Rainbow Menu Settings",
			[] {
				float speed = Settings::get<float>("menu/window/rainbow/speed", .4f);
				if (GUI::inputFloat("Rainbow Speed", &speed, .1f, 2.f))
					Settings::set<float>("menu/window/rainbow/speed", speed);

				float brightness = Settings::get<float>("menu/window/rainbow/brightness", .8f);
				if (GUI::inputFloat("Rainbow Brightness", &brightness, .1f, 2.f))
					Settings::set<float>("menu/window/rainbow/brightness", brightness);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		float windowColor[3]{
			Settings::get<float>("menu/window/color/r", 1.f),
			Settings::get<float>("menu/window/color/g", .0f),
			Settings::get<float>("menu/window/color/b", .0f)
		};

		if (GUI::colorEdit("Window Color", windowColor))
		{
			Settings::set<float>("menu/window/color/r", windowColor[0]);
			Settings::set<float>("menu/window/color/g", windowColor[1]);
			Settings::set<float>("menu/window/color/b", windowColor[2]);
		}

		int togglekey = Settings::get<int>("menu/togglekey");
		if (GUI::hotkey("Toggle Menu", &togglekey))
			Settings::set<int>("menu/togglekey", togglekey);

		GUI::checkbox("Check updates on start", Settings::get<bool*>("menu/updates/check_on_start", true));

		if (GUI::button("Check for updates"))
			Updater::checkForUpdate();
	});
	menuSettings.position = {1050, 250};
	menuSettings.size.y = 300;
	GUI::addWindow(menuSettings);

	GUI::Window playerWindow("Player", [] { JsonHacks::drawFromJson(JsonHacks::player); });
	playerWindow.position = {800, 50};
	playerWindow.size.y = 180;
	GUI::addWindow(playerWindow);

	GUI::Window variablesWindow("Variables", [] { JsonHacks::drawFromJson(JsonHacks::variables); });
	// GUI::addWindow(variablesWindow);

	GUI::Window macrobot("Macrobot", Macrobot::drawWindow);
	macrobot.position = {50, 350};
	macrobot.size.y = 260;
	GUI::addWindow(macrobot);

	GUI::Window shortcuts("Shortcuts", GUI::Shortcut::renderWindow);
	shortcuts.position = {1550, 50};
	shortcuts.size.y = 400;
	GUI::addWindow(shortcuts);

	GUI::Window labels("Labels", Labels::renderWindow);
	labels.position = {1300, 300};
	labels.size.y = 180;
	GUI::addWindow(labels);

	GUI::Window recorder("Recorder", Record::renderWindow);
	recorder.position = {800, 280};
	recorder.size.y = 450;
	GUI::addWindow(recorder);

	GUI::Window credits("Credits", [] {
		GUI::textURL("SpaghettDev", "https://github.com/SpaghettDev");
		GUI::textURL("TpdEA", "https://github.com/TpdeaX");
	});
	credits.position = {1550, 480};
	credits.size.y = 120;
	GUI::addWindow(credits);
}

void render()
{
	for (GUI::Shortcut& s : GUI::shortcuts)
	{
		if (ImGui::IsKeyPressed((ImGuiKey)s.key, false))
		{
			GUI::currentShortcut = s.name;
			GUI::shortcutLoop = true;
			GUI::draw();
			GUI::shortcutLoop = false;
			Settings::save();
			JsonHacks::save();
		}
	}

	GUI::draw();
	Updater::draw();
	GUI::setStyle();

	if (DiscordRPCManager::core)
		DiscordRPCManager::core->RunCallbacks();
}

$on_mod(Loaded)
{
	Mod::get()->setLoggingEnabled(true);

	ImGuiCocos::get().setup([]{
		init();
	}).draw([]{
		render();
	});
}
