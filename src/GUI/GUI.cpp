#include "GUI.h"
#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "../JsonHacks/JsonHacks.h"
#include "../Settings.h"
#include "WindowAction.h"

#include <fstream>
#include <sstream>

class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr) {
		int menuKey = Settings::get<int>("menu/togglekey", VK_TAB);

        if (down && (key == menuKey)) {
            GUI::toggle();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
    }
};

class $modify(MenuLayer) {
    bool init() {
		static bool init = false;
		if (!init)
		{
			GUI::lateInit();
			GUI::loadStyle("GDMO\\Style.style");
			GUI::canToggle = true;
		}
		init = true;
        return MenuLayer::init();
    }
};

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

ImVec2 GUI::getJsonSize(std::string name, ImVec2 defaultSize)
{
	if (!windowPositions.contains(name) || !windowPositions[name].contains("w"))
	{
		windowPositions[name]["w"] = defaultSize.x;
		windowPositions[name]["h"] = defaultSize.y;
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
	windowPositions = json::object();
	load();
}

void GUI::setLateInit(const std::function<void()>& func)
{
	lateInit = func;
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
		window.size = getJsonSize(window.name, window.size);
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

void GUI::setStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	float r = Settings::get<float>("menu/window/color/r", 1.f);
	float g = Settings::get<float>("menu/window/color/g", 0.f);
	float b = Settings::get<float>("menu/window/color/b", 0.f);

	if (Settings::get<bool>("menu/window/rainbow/enabled"))
	{
		ImGui::ColorConvertHSVtoRGB(
			ImGui::GetTime() * Settings::get<float>("menu/window/rainbow/speed", .4f),
			Settings::get<float>("menu/window/rainbow/brightness", .8f),
			Settings::get<float>("menu/window/rainbow/brightness"),
			r, g, b
		);

		style.Colors[ImGuiCol_Border] = { r, g, b, 1 };
		style.Colors[ImGuiCol_TitleBg] = { r, g, b, 1 };
		style.Colors[ImGuiCol_TitleBgCollapsed] = { r, g, b, 1 };
		style.Colors[ImGuiCol_TitleBgActive] = { r, g, b, 1 };

		style.Colors[ImGuiCol_Tab] = { r * .85f, g * .85f, b * .85f, 1 };
		style.Colors[ImGuiCol_TabActive] = { r * .85f, g * .85f, b * .85f, 1 };
		style.Colors[ImGuiCol_TabHovered] = { r * .70f, g * .70f, b * .70f, 1 };
		style.Colors[ImGuiCol_TabUnfocused] = { r * .60f, g * .60f, b * .60f, 1 };
		style.Colors[ImGuiCol_TabUnfocusedActive] = { r * .60f, g * .60f, b * .60f, 1 };
	}
	else
	{
		style.Colors[ImGuiCol_TitleBg] = { r, g, b, 1.f };
		style.Colors[ImGuiCol_TitleBgActive] = { r, g, b, 1.f };
		style.Colors[ImGuiCol_TitleBgCollapsed] = { r, g, b, 1.f };
	}
}

bool GUI::shouldRender()
{
	return isVisible && !shortcutLoop;
}
