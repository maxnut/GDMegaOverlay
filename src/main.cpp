#include "imgui_internal.h"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

#include "ConstData.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui-cocos.hpp>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

#include "Common.h"
#include "DiscordRPCManager/DiscordRPCManager.h"
#include "GUI/GUI.h"
#include "Hacks/Labels.h"
#include "Hacks/StartposSwitcher.h"
#include "Hacks/SafeMode.h"
#include "JsonPatches/JsonPatches.h"
#include "Macrobot/Clickpacks.h"
#include "Macrobot/Macrobot.h"
#include "Macrobot/Record.h"
#include "Settings.hpp"

void init()
{
	if (!ghc::filesystem::exists(Mod::get()->getSaveDir() / "macros"))
		ghc::filesystem::create_directory(Mod::get()->getSaveDir() / "macros");
	if (!ghc::filesystem::exists(Mod::get()->getSaveDir() / "renders"))
		ghc::filesystem::create_directory(Mod::get()->getSaveDir() / "renders");
	if (!ghc::filesystem::exists(Mod::get()->getSaveDir() / "clickpacks"))
		ghc::filesystem::create_directory(Mod::get()->getSaveDir() / "clickpacks");

	JsonPatches::init();
	DiscordRPCManager::init();
	Macrobot::getMacros();
	Clickpacks::init();
	Common::updateCheating();

	GUI::setLateInit([] {
		Common::calculateFramerate();
		Common::setPriority();
		Common::onAudioSpeedChange();
		Common::loadIcons();
		SafeMode::updateAuto();
	});
}

void initGUI()
{
	GUI::windows.clear();
	
	GUI::Window generalWindow("General", [] {
		float framerate = Settings::get<float>("general/fps/value", 60.f);
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

		float speedhack = Settings::get<float>("general/speedhack/value", 1.f);
		if (GUI::inputFloat("##SpeedhackValue", &speedhack))
		{
			if(speedhack > 0)
				Mod::get()->setSavedValue<float>("general/speedhack/value", speedhack);
		}

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::calculateFramerate();

			GUI::sameLine();
		}

		if (GUI::checkbox("Speedhack", "general/speedhack/enabled"))
			Common::calculateFramerate();

		float pitch = Settings::get<float>("general/music/pitch/value", 1.f);
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

		float music_speed = Settings::get<float>("general/music/speed/value", 1.f);
		if (GUI::inputFloat("##MusicSpeedValue", &music_speed))
			Mod::get()->setSavedValue<float>("general/music/speed/value", music_speed);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				Common::onAudioSpeedChange();

			GUI::sameLine();
		}

		if (Settings::get<bool>("general/tie_to_game_speed/music/enabled"))
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

		int priority = Settings::get<int>("general/priority", 2);
		if (GUI::combo("Thread Priority", &priority, priorities, 5))
		{
			Mod::get()->setSavedValue<int>("general/priority", priority);
			Common::setPriority();
		}
	});
	generalWindow.position = {50, 50};
	generalWindow.size.y = 250;
	GUI::addWindow(generalWindow);

	GUI::Window bypassWindow("Bypass", [] { JsonPatches::drawFromPatches(JsonPatches::bypass); });
	bypassWindow.position = {1050, 50};
	bypassWindow.size.y = 180;
	GUI::addWindow(bypassWindow);

	GUI::Window creatorWindow("Creator", [] { JsonPatches::drawFromPatches(JsonPatches::creator); });
	creatorWindow.position = {1300, 50};
	creatorWindow.size.y = 200;
	GUI::addWindow(creatorWindow);

	GUI::Window globalWindow("Global", [] {
		if (GUI::checkbox("Discord Rich Presence", "general/discordrpc/enabled"))
			DiscordRPCManager::updateState();

		GUI::arrowButton("Discord Rich Presence Settings");
		GUI::modalPopup(
			"Discord Rich Presence Settings",
			[] {
				GUI::checkbox("Incognito Mode", "general/discordrpc/incognito");
			},
			ImGuiWindowFlags_AlwaysAutoResize
		);


		JsonPatches::drawFromPatches(JsonPatches::global);
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
				int key = Settings::get<int>("level/startpos_switcher/left", ImGuiKey_LeftArrow);
				if (GUI::hotkey("Switch Left", &key))
					Mod::get()->setSavedValue<int>("level/startpos_switcher/left", key);

				key = Settings::get<int>("level/startpos_switcher/right", ImGuiKey_RightArrow);
				if (GUI::hotkey("Switch Right", &key))
					Mod::get()->setSavedValue<int>("level/startpos_switcher/right", key);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		GUI::checkbox("Replay Last Checkpoint", "level/replay_checkpoint");

		GUI::checkbox("Auto Safe Mode", "level/safe_mode/auto");

		if (GUI::checkbox("Safe Mode", "level/safe_mode/enabled"))
			SafeMode::updateState();
		GUI::checkbox("Safe Mode Endscreen Label", "level/safe_mode/endscreen_enabled", true);

		GUI::checkbox("Endscreen Info", "level/endlevellayerinfo/enabled", true);

		GUI::checkbox("Hide Pause Button", "general/hide_pause/button");
		GUI::checkbox("Hide Pause Menu", "general/hide_pause/menu");

		JsonPatches::drawFromPatches(JsonPatches::level);
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
				float speed = Settings::get<float>("menu/window/rainbow/speed", .4f);
				if (GUI::inputFloat("Rainbow Speed", &speed, .1f, 2.f))
					Mod::get()->setSavedValue<float>("menu/window/rainbow/speed", speed);

				float brightness = Settings::get<float>("menu/window/rainbow/brightness", .8f);
				if (GUI::inputFloat("Rainbow Brightness", &brightness, .1f, 2.f))
					Mod::get()->setSavedValue<float>("menu/window/rainbow/brightness", brightness);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		float windowColor[3]{
			Settings::get<float>("menu/window/color/r", 1.f),
			Settings::get<float>("menu/window/color/g", .0f),
			Settings::get<float>("menu/window/color/b", .0f)
		};

		if (GUI::colorEdit("Window Color", windowColor))
		{
			Mod::get()->setSavedValue<float>("menu/window/color/r", windowColor[0]);
			Mod::get()->setSavedValue<float>("menu/window/color/g", windowColor[1]);
			Mod::get()->setSavedValue<float>("menu/window/color/b", windowColor[2]);
		}

		int togglekey = Settings::get<int>("menu/togglekey", ImGuiKey_Tab);
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

	GUI::Window playerWindow("Player", [] { JsonPatches::drawFromPatches(JsonPatches::player); });
	playerWindow.position = {800, 50};
	playerWindow.size.y = 180;
	GUI::addWindow(playerWindow);

	GUI::Window variablesWindow("Variables", [] { JsonPatches::drawFromPatches(JsonPatches::variables); });
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

	init();

	ImGuiCocos::get().setup([]{
		GUI::init();
		initGUI();
	}).draw([]{
		render();
	});
}
