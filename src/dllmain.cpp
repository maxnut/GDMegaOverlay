#include <cocos2d.h>
#include <imgui-hook.hpp>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ConstData.h"
#include "GUI/GUI.h"
#include "imgui_internal.h"
#include "json.hpp"
#include <MinHook.h>
#include <filesystem>
#include <fstream>

#include "Common.h"
#include "Hacks/AudioChannelControl.h"
#include "Hacks/ReplayLastCheckpoint.h"
#include "Hacks/Speedhack.h"
#include "Hacks/StartposSwitcher.h"
#include "JsonHacks/JsonHacks.h"
#include "Macrobot/Macrobot.h"
#include "Settings.h"

void init()
{
#ifdef DEV_CONSOLE
	AllocConsole();
	static std::ofstream conout("CONOUT$", std::ios::out);
	std::cout.rdbuf(conout.rdbuf());
#endif

	if (!std::filesystem::exists("GDMO"))
		std::filesystem::create_directory("GDMO");
	if (!std::filesystem::exists("GDMO\\mod"))
		std::filesystem::create_directory("GDMO\\mod");

	Settings::load();
	JsonHacks::load();
	GUI::init();

	GUI::setLateInit([] {
		Common::calculateFramerate();
		Common::setPriority();
		Common::onAudioSpeedChange();
		Common::onAudioPitchChange();
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
	generalWindow.minSize = {200, 120};
	generalWindow.maxSize = {200, 2000};
	GUI::addWindow(generalWindow);

	GUI::Window bypassWindow("Bypass", [] { JsonHacks::drawFromJson(JsonHacks::bypass); });
	bypassWindow.minSize = {200, 120};
	bypassWindow.maxSize = {200, 2000};
	GUI::addWindow(bypassWindow);

	GUI::Window creatorWindow("Creator", [] { JsonHacks::drawFromJson(JsonHacks::creator); });
	creatorWindow.minSize = {200, 120};
	creatorWindow.maxSize = {200, 2000};
	GUI::addWindow(creatorWindow);

	GUI::Window globalWindow("Global", [] { JsonHacks::drawFromJson(JsonHacks::global); });
	globalWindow.minSize = {200, 120};
	globalWindow.maxSize = {200, 2000};
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
	levelWindow.minSize = {200, 120};
	levelWindow.maxSize = {200, 2000};
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

		float windowColor[3];
		windowColor[0] = Settings::get<float>("menu/window/color/r", 1.f);
		windowColor[1] = Settings::get<float>("menu/window/color/g", 0.f);
		windowColor[2] = Settings::get<float>("menu/window/color/b", 0.f);

		if (GUI::colorEdit("Window Color", windowColor))
		{
			Settings::set<float>("menu/window/color/r", windowColor[0]);
			Settings::set<float>("menu/window/color/g", windowColor[1]);
			Settings::set<float>("menu/window/color/b", windowColor[2]);
			ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = {windowColor[0], windowColor[1], windowColor[2], 1.f};
			ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = {windowColor[0], windowColor[1], windowColor[2], 1.f};
			ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed] = {windowColor[0], windowColor[1], windowColor[2], 1.f};
		}
	});
	menuSettings.minSize = {200, 120};
	menuSettings.maxSize = {200, 2000};
	GUI::addWindow(menuSettings);

	GUI::Window playerWindow("Player", [] { JsonHacks::drawFromJson(JsonHacks::player); });
	playerWindow.minSize = {200, 120};
	playerWindow.maxSize = {200, 2000};
	GUI::addWindow(playerWindow);

	GUI::Window variablesWindow("Variables", [] { JsonHacks::drawFromJson(JsonHacks::variables); });
	variablesWindow.minSize = {200, 120};
	variablesWindow.maxSize = {200, 2000};
	// GUI::addWindow(variablesWindow);

	GUI::Window macrobot("Macrobot", Macrobot::drawWindow);
	macrobot.minSize = {200, 120};
	macrobot.maxSize = {200, 2000};
	GUI::addWindow(macrobot);

	GUI::Window shortcuts("Shortcuts", GUI::Shortcut::renderWindow);
	shortcuts.minSize = {200, 120};
	shortcuts.maxSize = {200, 2000};
	GUI::addWindow(shortcuts);
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
		}
	}

	GUI::draw();
}

DWORD WINAPI my_thread(void* hModule)
{
	ImGuiHook::setRenderFunction(render);
	ImGuiHook::setInitFunction(init);
	ImGuiHook::setToggleCallback([]() { GUI::toggle(); });

	if (MH_Initialize() == MH_OK)
	{
		ImGuiHook::setupHooks(
			[](void* target, void* hook, void** trampoline) { MH_CreateHook(target, hook, trampoline); });
		Macrobot::initHooks();
		StartposSwitcher::initHooks();
		Common::initHooks();
		ReplayLastCheckpoint::initHooks();
		AudioChannelControl::initHooks();
		Speedhack::initHooks();
		GUI::initHooks();

		MH_EnableHook(MH_ALL_HOOKS);
	}
	else
	{
		std::cout << "MinHook failed to load! Unloading..." << std::endl;
		FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
	}
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
		[[fallthrough]];
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
