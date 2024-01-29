#pragma once

#include "Window.h"
#include "Widgets.h"
#include "Shortcut.h"

#include <nlohmann/json.hpp>
#include <vector>


using json = nlohmann::json;

class WindowAction;

namespace GUI
{
	inline std::vector<Window> windows;
	inline std::vector<Shortcut> shortcuts;
	inline std::vector<WindowAction*> windowActions;
	inline std::string currentShortcut = "";
	inline json windowPositions;

	inline bool isVisible = false;
	inline bool shortcutLoop = false;

	inline bool canToggle = false;

	inline float hideTimer = 0.0f;

	inline std::function<void()> lateInit;

	inline bool(__thiscall* menuLayerInit)(int* self);
	bool __fastcall menuLayerInitHook(int* self, void*);

	ImVec2 getJsonPosition(const std::string&);
	void setJsonPosition(const std::string&, ImVec2);

	ImVec2 getJsonSize(const std::string&, ImVec2 defaultSize);
	void setJsonSize(const std::string&, ImVec2);

	void init();
	void setLateInit(const std::function<void()>&func);

	void draw();

	void toggle();

	void addWindow(Window window);

	void save();
	void load();

	void saveStyle(const ghc::filesystem::path& name);
	void loadStyle(const ghc::filesystem::path& name);
	void setStyle();

	bool shouldRender();
};