#pragma once
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_opengl3.h>
#include <functional>

namespace ImGuiHook {
    void setupHooks(std::function<void(void*, void*, void**)> hookFunc);

    void setRenderFunction(std::function<void()> func);
    void setToggleCallback(std::function<void()> func);
    void setInitFunction(std::function<void()> func);
    void setKeybind(size_t key);
}
