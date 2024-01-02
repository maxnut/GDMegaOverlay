#pragma once

#include <iostream>
#include <imgui/imgui.h>
#include <functional>

namespace GUI
{
    bool button(std::string);
    bool checkbox(std::string, bool*);
    bool inputInt(std::string, int*, int = -INT_MAX, int = INT_MAX);
    bool inputFloat(std::string, float*, float = -FLT_MAX, float = FLT_MAX);
    bool dragInt(std::string, int*, int = -INT_MAX, int = INT_MAX);
    bool dragFloat(std::string, float*, float = -FLT_MAX, float = FLT_MAX);

    bool combo(std::string name, int* value, const char* const items[], int itemsCount);

    bool colorEdit(std::string, float*, bool = false, bool = false);

    void arrowButton(std::string);

    bool hotkey(std::string, int*, const ImVec2& = ImVec2(0, 0));

    bool modalPopup(std::string, const std::function<void()>&, int = ImGuiWindowFlags_AlwaysAutoResize);
};