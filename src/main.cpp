#define isnan isnan

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
#include "Hacks/Labels.h"
#include "Hacks/StartposSwitcher.h"
#include "Hacks/SafeMode.h"
#include "JsonHacks/JsonHacks.h"
#include "Macrobot/Clickpacks.h"
#include "Macrobot/Macrobot.h"
#include "Macrobot/Record.h"

void init()
{
	if (!std::filesystem::exists(Mod::get()->getSaveDir().string() + "\\mod"))
		std::filesystem::create_directory(Mod::get()->getSaveDir().string() + "\\mod");
	if (!std::filesystem::exists(Mod::get()->getSaveDir().string() + "\\macros"))
		std::filesystem::create_directory(Mod::get()->getSaveDir().string() + "\\macros");
	if (!std::filesystem::exists(Mod::get()->getSaveDir().string() + "\\renders"))
		std::filesystem::create_directory(Mod::get()->getSaveDir().string() + "\\renders");
	if (!std::filesystem::exists(Mod::get()->getSaveDir().string() + "\\clickpacks"))
		std::filesystem::create_directory(Mod::get()->getSaveDir().string() + "\\clickpacks");

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
		SafeMode::updateState();
	});

	GUI::Window generalWindow("General", [] {
		float framerate = Mod::get()->getSavedValue<float>("general/fps/value", 60.f);
		if (GUI::inputFloat("##FPSValue", &framerate))
			Mod::get()->setSavedValue<float>("general/fps/value", framerate);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::calculateFramerate();

			GUI::sameLine();
		}

		if (GUI::checkbox("FPS", "general/fps/enabled"))
			Common::calculateFramerate();

		float speedhack = Mod::get()->getSavedValue<float>("general/speedhack/value", 60.f);
		if (GUI::inputFloat("##SpeedhackValue", &speedhack))
			Mod::get()->setSavedValue<float>("general/speedhack/value", speedhack);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::calculateFramerate();

			GUI::sameLine();
		}

		if (GUI::checkbox("Speedhack", "general/speedhack/enabled"))
			Common::calculateFramerate();

		float pitch = Mod::get()->getSavedValue<float>("general/music/pitch/value", 1.f);
		if (GUI::inputFloat("##PitchShiftValue", &pitch, 0.5f, 2.f))
			Mod::get()->setSavedValue<float>("general/music/pitch/value", pitch);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::onAudioPitchChange();

			GUI::sameLine();
		}
		if (GUI::checkbox("Pitch Shift", "general/music/pitch/enabled"))
			Common::onAudioPitchChange();

		float music_speed = Mod::get()->getSavedValue<float>("general/music/speed/value", 1.f);
		if (GUI::inputFloat("##MusicSpeedValue", &music_speed))
			Mod::get()->setSavedValue<float>("general/music/speed/value", music_speed);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::onAudioSpeedChange();

			GUI::sameLine();
		}

		if (Mod::get()->getSavedValue<bool>("general/tie_to_game_speed/music/enabled"))
		{
			Mod::get()->setSavedValue<bool>("general/music/speed/enabled", false);

			ImGui::BeginDisabled();
			GUI::checkbox("Music Speed", "general/music/speed/enabled");
			ImGui::EndDisabled();
		}
		else
		{
			if (GUI::checkbox("Music Speed", "general/music/speed/enabled"))
				Common::onAudioSpeedChange();
		}

		if (GUI::checkbox("Tie Music To Game Speed", "general/tie_to_game_speed/music/enabled"))
			Common::onAudioSpeedChange();

		int priority = Mod::get()->getSavedValue<int>("general/priority", 2);
		if (GUI::combo("Thread Priority", &priority, priorities, 5))
		{
			Mod::get()->setSavedValue<int>("general/priority", priority);
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
		GUI::checkbox("Discord Rich Presence", "general/discordrpc/enabled");

		JsonHacks::drawFromJson(JsonHacks::global);
	});
	globalWindow.position = {300, 50};
	globalWindow.size.y = 230;
	GUI::addWindow(globalWindow);

	GUI::Window levelWindow("Level", [] {
		GUI::checkbox("Startpos Switcher", "level/startpos_switcher");

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

		GUI::checkbox("Replay Last Checkpoint", "level/replay_checkpoint");

		if (GUI::checkbox("Safe Mode", "level/safe_mode/enabled"))
			SafeMode::updateState();
		GUI::checkbox("Safe Mode End Screen Label", "level/safe_mode/endscreen_enabled", true);

		GUI::checkbox("End Level Layer Info", "level/endlevellayerinfo/enabled", true);

		GUI::checkbox("Hide Pause Button", "general/hide_pause/button");
		GUI::checkbox("Hide Pause Menu", "general/hide_pause/menu");

		JsonHacks::drawFromJson(JsonHacks::level);
	});
	levelWindow.position = {550, 50};
	levelWindow.size.y = 180;
	GUI::addWindow(levelWindow);

	GUI::Window menuSettings("Menu Settings", [] {
		int snap = Mod::get()->getSavedValue<int>("menu/window/snap", 10);
		int snapSize = Mod::get()->getSavedValue<int>("menu/window/size_snap", 10);
		float transitionDuration = Mod::get()->getSavedValue<float>("menu/transition_duration", 0.35f);
		float windowOpacity = Mod::get()->getSavedValue<float>("menu/window/opacity", 0.98f);

		if (GUI::dragInt("Window Snap", &snap, 0))
			Mod::get()->setSavedValue<int>("menu/window/snap", snap);

		if (GUI::dragInt("Size Snap", &snapSize, 0))
			Mod::get()->setSavedValue<int>("menu/window/size_snap", snapSize);

		if (GUI::dragFloat("Transition Duration", &transitionDuration, 0))
			Mod::get()->setSavedValue<float>("menu/transition_duration", transitionDuration);

		if (GUI::inputFloat("Window Opacity", &windowOpacity, 0.f, 1.f))
			Mod::get()->setSavedValue<float>("menu/window/opacity", windowOpacity);

		GUI::checkbox("Rainbow Menu", "menu/window/rainbow/enabled");

		GUI::arrowButton("Rainbow Menu Settings");
		GUI::modalPopup(
			"Rainbow Menu Settings",
			[] {
				float speed = Mod::get()->getSavedValue<float>("menu/window/rainbow/speed", .4f);
				if (GUI::inputFloat("Rainbow Speed", &speed, .1f, 2.f))
					Mod::get()->setSavedValue<float>("menu/window/rainbow/speed", speed);

				float brightness = Mod::get()->getSavedValue<float>("menu/window/rainbow/brightness", .8f);
				if (GUI::inputFloat("Rainbow Brightness", &brightness, .1f, 2.f))
					Mod::get()->setSavedValue<float>("menu/window/rainbow/brightness", brightness);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		float windowColor[3]{
			Mod::get()->getSavedValue<float>("menu/window/color/r", 1.f),
			Mod::get()->getSavedValue<float>("menu/window/color/g", .0f),
			Mod::get()->getSavedValue<float>("menu/window/color/b", .0f)
		};

		if (GUI::colorEdit("Window Color", windowColor))
		{
			Mod::get()->setSavedValue<float>("menu/window/color/r", windowColor[0]);
			Mod::get()->setSavedValue<float>("menu/window/color/g", windowColor[1]);
			Mod::get()->setSavedValue<float>("menu/window/color/b", windowColor[2]);
		}

		int togglekey = Mod::get()->getSavedValue<int>("menu/togglekey", VK_TAB);
		if (GUI::hotkey("Toggle Menu", &togglekey))
			Mod::get()->setSavedValue<int>("menu/togglekey", togglekey);

		if (GUI::button("Open Resources Folder"))
			ShellExecute(0, NULL, Mod::get()->getResourcesDir().string().c_str(), NULL, NULL, SW_SHOW);
		if (GUI::button("Open Save Folder"))
			ShellExecute(0, NULL, Mod::get()->getSaveDir().string().c_str(), NULL, NULL, SW_SHOW);
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
	GUI::draw();
	GUI::setStyle();
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
