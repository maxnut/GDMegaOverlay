#include "GUI.h"
#include "../JsonHacks/JsonHacks.h"
#include "../Settings.h"
#include "WindowAction.h"

#include "CocoStudio/Armature/utils/CCTweenFunction.h"
#include <cocos2d.h>
#include <fstream>
#include <sstream>

ImVec2 GUI::getJsonPosition(std::string name)
{
	if (!windowPositions.contains(name))
	{
		windowPositions[name]["x"] = .0f;
		windowPositions[name]["y"] = .0f;
	}

	return {windowPositions[name]["x"].get<float>(), windowPositions[name]["y"].get<float>()};
}

void GUI::setJsonPosition(std::string name, ImVec2 pos)
{
	windowPositions[name]["x"] = pos.x;
	windowPositions[name]["y"] = pos.y;
}

ImVec2 GUI::getJsonSize(std::string name)
{
	if (!windowPositions.contains(name))
	{
		windowPositions[name]["w"] = 220.f;
		windowPositions[name]["h"] = 120.f;
	}

	return {windowPositions[name]["w"].get<float>(), windowPositions[name]["h"].get<float>()};
}

void GUI::setJsonSize(std::string name, ImVec2 size)
{
	windowPositions[name]["w"] = size.x;
	windowPositions[name]["h"] = size.y;
}

void GUI::init()
{
	auto fnt = ImGui::GetIO().Fonts->AddFontFromFileTTF("GDMO\\arial.ttf", 14);
	ImGui::GetIO().FontDefault = fnt;
	load();
}

void GUI::initHooks()
{
	MH_CreateHook(reinterpret_cast<void*>(utils::gd_base + 0x27B2E0), menuLayerInitHook,
				  reinterpret_cast<void**>(&menuLayerInit));
}

void GUI::setLateInit(const std::function<void()>& func)
{
	lateInit = func;
}

bool __fastcall GUI::menuLayerInitHook(int* self, void*)
{
	static bool init = false;
	if (!init)
	{
		lateInit();
		loadStyle("GDMO\\Style.style");
		canToggle = true;
	}
	init = true;

	return menuLayerInit(self);
}

void GUI::draw()
{
	if (!isVisible && !shortcutLoop)
		return;

	for (WindowAction* ac : windowActions)
		ac->step(ImGui::GetIO().DeltaTime);

	for (Window& w : windows)
		w.draw();

	windowPositions["res"]["x"] = ImGui::GetIO().DisplaySize.x;
	windowPositions["res"]["y"] = ImGui::GetIO().DisplaySize.y;

	float transitionDuration = Settings::get<float>("menu/transition_duration", 0.35f);

	hideTimer += ImGui::GetIO().DeltaTime;
	if (hideTimer > transitionDuration)
		isVisible = false;
}

void GUI::toggle()
{
	if (!canToggle)
		return;

	isVisible = true;
	static bool toggle = false;
	toggle = !toggle;

	// 🔥🔥🔥🔥
	hideTimer = toggle ? -FLT_MAX : 0;

	if (!toggle)
		save();

	for (WindowAction* ac : windowActions)
		delete ac;

	windowActions.clear();

	int direction = std::rand() % 4 + 1;

	for (Window& w : windows)
	{
		ImVec2 screenSize = ImGui::GetIO().DisplaySize;
		ImVec2 jsonScreenSize = {windowPositions["res"]["x"], windowPositions["res"]["y"]};

		if (screenSize.x != jsonScreenSize.x || screenSize.y != jsonScreenSize.y)
		{
			ImVec2 scaleFactor = {screenSize.x / jsonScreenSize.x, screenSize.y / jsonScreenSize.y};
			if (scaleFactor.x >= 0.5f)
				w.position = {w.position.x * scaleFactor.x, w.position.y * scaleFactor.y};
		}

		uint8_t animationType = (w.name.length() + direction) % 4;

		ImVec2 dir;

		switch (animationType)
		{
		case 0:
			dir = {1400, 1400};
			break;
		case 1:
			dir = {1400, -1400};
			break;
		case 2:
			dir = {-1400, 1400};
			break;
		case 3:
			dir = {-1400, -1400};
			break;
		}

		float transitionDuration = Settings::get<float>("menu/transition_duration", 0.35f);

		WindowAction* action = WindowAction::create(
			transitionDuration, &w, toggle ? w.position : ImVec2(w.position.x + dir.x, w.position.y + dir.y));
		windowActions.push_back(action);
	}
}

void GUI::addWindow(Window window)
{
	if (windowPositions.contains(window.name))
	{
		window.position = getJsonPosition(window.name);
		window.size = getJsonSize(window.name);
	}

	int direction = std::rand() % 4 + 1;

	uint8_t animationType = (window.name.length() + direction) % 4;

	switch (animationType)
	{
	case 0:
		window.renderPosition = {1400, 1400};
		break;
	case 1:
		window.renderPosition = {1400, -1400};
		break;
	case 2:
		window.renderPosition = {-1400, 1400};
		break;
	case 3:
		window.renderPosition = {-1400, -1400};
		break;
	}

	windows.push_back(window);
}

void GUI::save()
{
	for (Window& w : windows)
	{
		windowPositions[w.name]["x"] = w.position.x;
		windowPositions[w.name]["y"] = w.position.y;
	}

	windowPositions["res"]["x"] = ImGui::GetIO().DisplaySize.x;
	windowPositions["res"]["y"] = ImGui::GetIO().DisplaySize.y;

	std::ofstream f("GDMO\\windows.json");
	f << windowPositions.dump(4);
	f.close();

	f.open("GDMO\\shortcuts.json");
	json shortcutArray = json::array();

	for (Shortcut& s : shortcuts)
	{
		json shortcutObject = json::object();
		shortcutObject["name"] = s.name;
		shortcutObject["key"] = s.key;
		shortcutArray.push_back(shortcutObject);
	}

	f << shortcutArray.dump(4);
	f.close();

	saveStyle("GDMO\\Style.style");

	Settings::save();
	JsonHacks::save();
}

void GUI::load()
{
	std::ifstream f("GDMO\\windows.json");
	if (f)
	{
		std::stringstream buffer;
		buffer << f.rdbuf();
		windowPositions = json::parse(buffer.str());
		buffer.clear();
	}
	f.close();

	if (!windowPositions.contains("res"))
	{
		windowPositions["res"]["x"] = ImGui::GetIO().DisplaySize.x;
		windowPositions["res"]["y"] = ImGui::GetIO().DisplaySize.x;
	}

	f.open("GDMO\\shortcuts.json");
	if (f)
	{
		std::stringstream buffer;
		buffer << f.rdbuf();
		json shortcutArray = json::parse(buffer.str());

		for (json shortcutObject : shortcutArray)
		{
			Shortcut s(shortcutObject["key"], shortcutObject["name"]);
			GUI::shortcuts.push_back(s);
		}

		buffer.clear();
	}
	f.close();
}

void GUI::saveStyle(std::string name)
{
	ImGuiStyle style = ImGui::GetStyle();
	std::ofstream styleFile(name, std::ios::binary);
	styleFile.write((const char*)&style, sizeof(ImGuiStyle));
	styleFile.close();
}
void GUI::loadStyle(std::string name)
{
	ImGuiStyle& style = ImGui::GetStyle();
	std::ifstream styleFile(name, std::ios::binary);
	styleFile.read((char*)&style, sizeof(ImGuiStyle));
	styleFile.close();
}

bool GUI::shouldRender()
{
	return isVisible && !shortcutLoop;
}