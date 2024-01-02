#pragma once

#include "Window.h"
#include "Widgets.h"
#include "Shortcut.h"

#include "../json.hpp"
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

    inline float hideTimer = 0.0f;

    ImVec2 getJsonPosition(std::string);
    void setJsonPosition(std::string, ImVec2);

    ImVec2 getJsonSize(std::string);
    void setJsonSize(std::string, ImVec2);

    void init();

    void draw();

    void toggle();

    void addWindow(Window window);

    void save();
    void load();

    void saveStyle(std::string name);
    void loadStyle(std::string name);

    bool shouldRender();
};