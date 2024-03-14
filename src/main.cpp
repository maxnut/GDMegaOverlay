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
#include "Hacks/PhysicsBypass.h"
#include "JsonPatches/JsonPatches.h"
#include "Macrobot/Clickpacks.h"
#include "Macrobot/Macrobot.h"
#include "Macrobot/Record.h"
#include "Settings.hpp"
#include "GUI/Blur.h"

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
		PhysicsBypass::calculateTickrate();
		Common::setPriority();
		Common::onAudioSpeedChange();
		Common::loadIcons();
		SafeMode::updateAuto();
	});
}

void initGUI()
{
	ImGui::GetCurrentContext()->PlatformLocaleDecimalPoint = *localeconv()->decimal_point;
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

		if (GUI::checkbox("Framerate", "general/fps/enabled"))
			Common::calculateFramerate();

		if (Macrobot::playerMode == Macrobot::RECORDING || Macrobot::playerMode == Macrobot::PLAYBACK)
			ImGui::BeginDisabled();

		float tps = Settings::get<float>("general/tps/value", 240.f);
		if (GUI::inputFloat("##TPSValue", &tps, 1.f, 10000.f))
			Mod::get()->setSavedValue<float>("general/tps/value", tps);

		if (GUI::shouldRender())
		{
			if (ImGui::IsItemDeactivatedAfterEdit())
				PhysicsBypass::calculateTickrate();

			GUI::sameLine();
		}

		if (GUI::checkbox("Physics Bypass", "general/tps/enabled"))
			PhysicsBypass::calculateTickrate();

		if (Macrobot::playerMode == Macrobot::RECORDING || Macrobot::playerMode == Macrobot::PLAYBACK)
			ImGui::EndDisabled();

		float speedhack = Settings::get<float>("general/speedhack/value", 1.f);
		if (GUI::inputFloat("##SpeedhackValue", &speedhack), 0.0000001f, 1000.f)
		{
			if (speedhack > 0)
				Mod::get()->setSavedValue<float>("general/speedhack/value", speedhack);
		}

		GUI::sameLine();
		if (GUI::checkbox("Speedhack", "general/speedhack/enabled"))
			Common::onAudioSpeedChange();

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

		GUI::tooltip("Changes the pitch of the game audio without changing the length");

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

		GUI::tooltip("Makes game music adapt based on speedhack");

		int priority = Settings::get<int>("general/priority", 2);
		if (GUI::combo("Thread Priority", &priority, priorities, 5))
		{
			Mod::get()->setSavedValue<int>("general/priority", priority);
			Common::setPriority();
		}

		GUI::tooltip("Changes the priority of the game over other processes.");
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
		
		GUI::checkbox("Show Hitboxes", "level/show_hitbox/enabled");
		GUI::arrowButton("Show Hitboxes Settings");
		GUI::modalPopup(
			"Show Hitboxes Settings",
			[] {
				float borderSize = Settings::get<float>("level/show_hitbox/size", 0.25f);
				int borderAlpha = Settings::get<int>("level/show_hitbox/border_alpha", 255);
				int fillAlpha = Settings::get<int>("level/show_hitbox/fill_alpha", 50);

				int maxQueue = Settings::get<int>("level/show_hitbox/max_queue", 240);

				GUI::checkbox("Show Trail", "level/show_hitbox/queue_enabled");
				GUI::checkbox("Only On Death", "level/show_hitbox/on_death");

				if (GUI::inputFloat("Border Size", &borderSize, 0.f, 3.f))
					Mod::get()->setSavedValue<float>("level/show_hitbox/size", borderSize);

				if (GUI::inputInt("Border Opacity", &borderAlpha, 1, 255))
					Mod::get()->setSavedValue<int>("level/show_hitbox/border_alpha", borderAlpha);

				if (GUI::inputInt("Fill Opacity", &fillAlpha, 1, 255))
					Mod::get()->setSavedValue<int>("level/show_hitbox/fill_alpha", fillAlpha);

				if (GUI::inputInt("Max Queue Length", &maxQueue, 1, 99999999))
					Mod::get()->setSavedValue<int>("level/show_hitbox/max_queue", maxQueue);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		GUI::checkbox("Auto Deafen", "level/auto_deafen/enabled");

		GUI::tooltip("Deafens you on discord whenever you get to a certain percent.\nTo use auto deafen, set a mute keybind in discord with ALT + the key you want to use");

		GUI::arrowButton("Auto Deafen Settings");
		GUI::modalPopup(
			"Auto Deafen Settings",
			[] {
				int key = Settings::get<int>("level/auto_deafen/mute_key", ImGuiKey_None);
				if (GUI::hotkey("Mute Key", &key))
					Mod::get()->setSavedValue<int>("level/auto_deafen/mute_key", key);

				float percent = Settings::get<float>("level/auto_deafen/percent", 50.f);

				if (GUI::inputFloat("Mute %", &percent))
					Mod::get()->setSavedValue<float>("level/auto_deafen/percent", percent);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		GUI::checkbox("Replay Last Checkpoint", "level/replay_checkpoint");
		GUI::tooltip("Respawns you at the last checkpoint when you restart after completing the level.");

		GUI::checkbox("Auto Safe Mode", "level/safe_mode/auto");
		GUI::tooltip("Automatically toggles safe mode when you are cheating.");

		if (GUI::checkbox("Safe Mode", "level/safe_mode/enabled"))
			SafeMode::updateState();
		GUI::checkbox("Safe Mode Endscreen Label", "level/safe_mode/endscreen_enabled", true);
		GUI::tooltip("Shows a label on the endscreen when safe mode is active.");

		GUI::checkbox("Endscreen Info", "level/endlevellayerinfo/enabled", true);
		GUI::tooltip("Shows run info such as accuracy and deaths on the endscreen.");

		GUI::checkbox("Hide Pause Button", "general/hide_pause/button");
		GUI::checkbox("Hide Pause Menu", "general/hide_pause/menu");

		GUI::checkbox("No Shaders", "level/no_shaders");
		GUI::tooltip("Disables shaders from rendering.");

		GUI::checkbox("Instant Complete", "level/instant_complete");
		GUI::tooltip("Completes any level when you play it.\nUsing Instant Complete can and will ban you from the leaderboards! Use with caution.");

		GUI::checkbox("Layout Mode", "level/layout_mode");
		GUI::tooltip("Removes any decoration or non gameplay related trigger from the level.");

		GUI::checkbox("Hitbox Multiplier", "level/hitbox_multiplier");
		GUI::tooltip("Changes the hitbox size of slopes, hazards, and players.");

		GUI::arrowButton("Hitbox Multiplier Settings");
		GUI::modalPopup(
			"Hitbox Multiplier Settings",
			[] {
				float scaleSlopes[2] = {
					Settings::get<float>("level/hitbox_multiplier/scale_slopes/x", 1.f),
					Settings::get<float>("level/hitbox_multiplier/scale_slopes/y", 1.f)
				};
				float scaleHazards[2] = {
					Settings::get<float>("level/hitbox_multiplier/scale_hazards/x", 1.f),
					Settings::get<float>("level/hitbox_multiplier/scale_hazards/y", 1.f)
				};
				float scalePlayer[2] = {
					Settings::get<float>("level/hitbox_multiplier/scale_player/x", 1.f),
					Settings::get<float>("level/hitbox_multiplier/scale_player/y", 1.f)
				};

				if (GUI::inputFloat2("Slope Scale", scaleSlopes, 0.1f, 4.f, 0.1f, 4.f))
				{
					Mod::get()->setSavedValue<float>("level/hitbox_multiplier/scale_slopes/x", scaleSlopes[0]);
					Mod::get()->setSavedValue<float>("level/hitbox_multiplier/scale_slopes/y", scaleSlopes[1]);
				}

				if (GUI::inputFloat2("Hazard Scale", scaleHazards, 0.1f, 4.f, 0.1f, 4.f))
				{
					Mod::get()->setSavedValue<float>("level/hitbox_multiplier/scale_hazards/x", scaleHazards[0]);
					Mod::get()->setSavedValue<float>("level/hitbox_multiplier/scale_hazards/y", scaleHazards[1]);
				}

				if (GUI::inputFloat2("Player Scale", scalePlayer, 0.1f, 4.f, 0.1f, 4.f))
				{
					Mod::get()->setSavedValue<float>("level/hitbox_multiplier/scale_player/x", scalePlayer[0]);
					Mod::get()->setSavedValue<float>("level/hitbox_multiplier/scale_player/y", scalePlayer[1]);
				}
			});

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

		GUI::tooltip("Makes window position snap to a grid");

		if (GUI::dragInt("Size Snap", &snapSize, 0))
			Mod::get()->setSavedValue<int>("menu/window/size_snap", snapSize);

		GUI::tooltip("Makes window size snap to a grid");

		if (GUI::dragFloat("Transition Duration", &transitionDuration, 0))
			Mod::get()->setSavedValue<float>("menu/transition_duration", transitionDuration);

		GUI::tooltip("Duration of the window transition animation");

		if (GUI::inputFloat("Window Opacity", &windowOpacity, 0.f, 1.f))
			Mod::get()->setSavedValue<float>("menu/window/opacity", windowOpacity);

		GUI::tooltip("Opacity of the window background");

		GUI::inputFloat("UI Scale", "menu/ui_scale", 1.f, 0.1f, 2.f);
		GUI::tooltip("Changes how big the menu will appear");

		GUI::checkbox("Title Gradient", "menu/title_gradient/enabled", true);
		GUI::tooltip("Makes the titlebar color a gradient instead of a solid color");

		GUI::checkbox("Rainbow Menu", "menu/window/rainbow/enabled");
		GUI::tooltip("Makes the titlebar color cycle over time");

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

		auto blurSettings = [] {
				GUI::inputFloat("Blur Darkness", "menu/blur/darkness", 1.f, 0.1f, 1.f);
				GUI::inputFloat("Blur Size", "menu/blur/size", 1.f, 0.1f, 10.f);
				GUI::inputInt("Blur Steps", "menu/blur/steps", 10, 5, 20);
			};

		GUI::checkbox("Window Shadow", "menu/box_shadow/enabled", true);
		GUI::tooltip("Makes windows cast a box shadow underneath itself.");

		GUI::arrowButton("Window Shadow Settings");
		GUI::modalPopup(
			"Window Shadow Settings",
			[] {
				GUI::inputFloat("Shadow Size", "menu/box_shadow/size", 24.f, 12.f, 100.f);
			},
			ImGuiWindowFlags_AlwaysAutoResize);

		if(GUI::checkbox("Blur Background", "menu/blur/enabled"))
			Blur::compileBlurShader();
		
		GUI::tooltip("Applies a blur effect to the window background.\nWARNING: this option is very performance heavy!");
		
		GUI::arrowButton("Blur Settings##0");
		GUI::modalPopup(
			"Blur Settings##0",
			blurSettings,
			ImGuiWindowFlags_AlwaysAutoResize);

		if(GUI::checkbox("Blur GD", "menu/blur/gd"))
			Blur::compileBlurShader();

		GUI::tooltip("Applies a blur effect to the game while the menu is open.\nWARNING: this option is very performance heavy!");

		GUI::arrowButton("Blur Settings##1");
		GUI::modalPopup(
			"Blur Settings##1",
			blurSettings,
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

		GUI::tooltip("Changes the color of the window titlebar");

		int togglekey = Settings::get<int>("menu/togglekey", ImGuiKey_Tab);
		if (GUI::hotkey("Toggle Menu", &togglekey))
			Mod::get()->setSavedValue<int>("menu/togglekey", togglekey);

		if (GUI::button("Open Resources Folder"))
			ShellExecute(0, NULL, string::wideToUtf8(Mod::get()->getResourcesDir().wstring()).c_str(), NULL, NULL, SW_SHOW);
		if (GUI::button("Open Save Folder"))
			ShellExecute(0, NULL, string::wideToUtf8(Mod::get()->getSaveDir().wstring()).c_str(), NULL, NULL, SW_SHOW);
		if (GUI::button("Reset Windows"))
			GUI::resetDefault();
	});
	menuSettings.position = {1050, 250};
	menuSettings.size.y = 300;
	GUI::addWindow(menuSettings);

	GUI::Window playerWindow("Player", [] { 
		JsonPatches::drawFromPatches(JsonPatches::player);
		
		GUI::checkbox("Custom Wave Trail", "player/trail/enabled");

		GUI::arrowButton("Wave Customization");
		GUI::modalPopup(
			"Wave Customization",
			[]{
				GUI::inputFloat("Wave Trail Size", "player/trail/size", 1.f, 0.f, 100.f);

				float trailColor[3]{
					Settings::get<float>("player/trail/color/r", 1.f),
					Settings::get<float>("player/trail/color/g", 1.0f),
					Settings::get<float>("player/trail/color/b", 1.0f)
				};

				GUI::checkbox("Wave Trail Color", "player/trail/color/enabled");

				if (GUI::colorEdit("Trail Color", trailColor))
				{
					Mod::get()->setSavedValue<float>("player/trail/color/r", trailColor[0]);
					Mod::get()->setSavedValue<float>("player/trail/color/g", trailColor[1]);
					Mod::get()->setSavedValue<float>("player/trail/color/b", trailColor[2]);
				}
			},
			ImGuiWindowFlags_AlwaysAutoResize);
	});
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
