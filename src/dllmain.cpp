#include "pch.h"
#include <imgui-hook.hpp>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "ImgUtil.h"
#include "PlayLayer.h"
#include "MenuLayer.h"
#include "LevelSearchLayer.h"
#include "LevelEditorLayer.h"
#include "EndLevelLayer.h"
#include "Hacks.h"
#include "CCSchedulerHook.h"
#include "ReplayPlayer.h"
#include <fstream>
#include <shellapi.h>
#include "Shortcuts.h"
#include "json.hpp"
#include "explorer.hpp"
#include "portable-file-dialogs.h"

using json = nlohmann::json;

extern bool Hacks::show = false, Hacks::fake = false;
bool loaded = false;
bool isDecember = false;
extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

std::string Hacks::hackName;
Windows windowPositions;

DiscordManager Hacks::ds;

json Hacks::bypass, Hacks::creator, Hacks::global, Hacks::level, Hacks::player;

float Hacks::screenFps = 60.0f, Hacks::tps = 60.0f;

float screenSize = 0, pitch, oldScreenSize = 0;

int shortcutIndex, shortcutIndexKey, pitchName;
char fileName[30], searchbar[30], url[50], id[30];
std::vector<std::string> Hacks::musicPaths;
std::filesystem::path Hacks::path;
std::vector<const char *> musicPathsVet;

std::string hoveredHack = "";

const char *const priorities[] = {"Low", "Below Normal", "Normal", "Above Normal", "High"};
const char *const style[] = {"Number and text", "Number Only"};
const char *const positions[] = {"Top Right", "Top Left", "Bottom Right", "Bottom Left"};
const char *const items[] = {"Normal", "No Spikes", "No Hitbox", "No Solid", "Force Block", "Everything Hurts"};
const char *const trail[] = {"Normal", "Always Off", "Always On", "Inversed"};
const char *const fonts[] = {"Big Font", "Chat Font", "Font 01", "Font 02", "Font 03", "Font 04", "Font 05", "Font 06", "Font 07", "Font 08", "Font 09", "Font 10", "Font 11", "Gold Font"};

const char *const KeyNames[] = {
    "Unknown", "Mouse 0", "Mouse 1", "Cancel", "Mouse 3", "Mouse 4", "Mouse 5", "Unknown", "Backspace", "Tab", "Unknown", "Unknown", "Clear", "Return", "Unknown", "Unknown", "Shift", "CTRL", "Alt", "Pause", "Caps Lock", "Kana", "Unknown", "Junja", "Final", "Kanji", "Unknown", "Esc", "Convert", "Nonconvert", "Accept", "Modechange", "Space", "Prior", "Pgdn", "End", "Home", "Left Arrow", "Up Arrow", "Right Arrow", "Down Arrow", "Select", "Print", "Execute", "Print Screen", "Insert", "Canc", "Help", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "LWin", "RWin", "Apps", "Unknown", "Sleep", "Num0", "Num1", "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", "Multiply", "Add", "Separator", "Subtract", "Decimal", "Divide", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Numlock", "ScrollLock", "VK_OEM_NEC_EQUAL", "VK_OEM_FJ_MASSHOU", "VK_OEM_FJ_TOUROKU", "VK_OEM_FJ_LOYA", "VK_OEM_FJ_ROYA", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "LShift", "RShift", "LCTRL", "RCTRL", "LAlt", "RAlt"};

bool Hotkey(const char *label, int *k, const ImVec2 &size_arg = ImVec2(0, 0))
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    ImGuiIO &io = g.IO;
    const ImGuiStyle &style = g.Style;

    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);
    const ImRect frame_bb(window->DC.CursorPos + ImVec2(label_size.x + style.ItemInnerSpacing.x, 0.0f), window->DC.CursorPos + size);
    const ImRect total_bb(window->DC.CursorPos, frame_bb.Max);

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    const bool focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id);
    const bool focus_requested_by_code = focus_requested;
    const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

    const bool hovered = ImGui::ItemHoverable(frame_bb, id);

    if (hovered)
    {
        ImGui::SetHoveredID(id);
        g.MouseCursor = ImGuiMouseCursor_TextInput;
    }

    const bool user_clicked = hovered && io.MouseClicked[0];

    if (focus_requested || user_clicked)
    {
        if (g.ActiveId != id)
        {
            *k = 0;
        }
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
    }
    else if (io.MouseClicked[0])
    {
        if (g.ActiveId == id)
            ImGui::ClearActiveID();
    }

    bool value_changed = false;
    int key = *k;

    if (g.ActiveId == id)
    {
        if (!value_changed)
        {
            for (auto i = VK_BACK; i <= VK_RMENU; i++)
            {
                if (ImGui::IsKeyDown(static_cast<ImGuiKey>(i)))
                {
                    key = i;
                    value_changed = true;
                    ImGui::ClearActiveID();
                }
            }
        }

        if (ImGui::IsKeyPressedMap(static_cast<ImGuiKey>(ImGuiKey_Escape)))
        {
            *k = 0;
            ImGui::ClearActiveID();
        }
        else
        {
            *k = key;
        }
    }

    char buf_display[64] = "None";

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]), true, style.FrameRounding);

    if (*k != 0 && g.ActiveId != id)
    {
        strcpy_s(buf_display, KeyNames[*k]);
    }
    else if (g.ActiveId == id)
    {
        strcpy_s(buf_display, "<Press a key>");
    }

    const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y);
    ImVec2 render_pos = frame_bb.Min + style.FramePadding;
    ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, style.ButtonTextAlign, &clip_rect);

    if (label_size.x > 0)
        ImGui::RenderText(ImVec2(total_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), label);

    return value_changed;
}

void Marker(const char *marker, const char *desc)
{
    ImGui::TextDisabled(marker);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool findStringIC(const std::string &strHaystack, const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2)
        { return std::toupper(ch1) == std::toupper(ch2); });
    return (it != strHaystack.end());
}

bool ImHotkey(const char *label, int *k, const ImVec2 &size_arg = ImVec2(0, 0))
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = Hotkey(label, k, size_arg);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = Hotkey(label, k, size_arg);
            ImGui::PopStyleColor();
        }
    }
    else
        res = Hotkey(label, k, size_arg);
    return res;
}

bool ImCheckbox(const char *label, bool *v, bool canMakeShortcut = true)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::Checkbox(label, v);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::Checkbox(label, v);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::Checkbox(label, v);

    if (canMakeShortcut)
    {
        std::string labelString = label;

        bool openpopuptemp = false;

        if (ImGui::BeginPopupContextItem(label, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem(("Add shortcut##" + labelString).c_str()))
            {
                openpopuptemp = true;
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (openpopuptemp == true)
        {
            ImGui::OpenPopup(("Create shortcut for " + labelString).c_str());
            openpopuptemp = false;
        }

        if (ImGui::BeginPopupModal(("Create shortcut for " + labelString).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            Hotkey("Shortcut Key", &shortcutIndexKey);

            if (ImGui::Button("Add"))
            {
                Shortcuts::Shortcut s;
                s.key = shortcutIndexKey;
                strcpy(s.name, labelString.c_str());
                Shortcuts::shortcuts.push_back(s);
                Shortcuts::Save();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (labelString == Hacks::hackName)
        {
            Hacks::hackName = "";
            *v = !*v;
            res = true;
            Hacks::SaveSettings();
        }
    }

    return res;
}

bool ImButton(const char *label, bool canMakeShortcut = true)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::Button(label);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::Button(label);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::Button(label);

    if (canMakeShortcut)
    {
        std::string labelString = label;

        bool openpopuptemp = false;

        if (ImGui::BeginPopupContextItem(label, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem(("Add shortcut##" + labelString).c_str()))
            {
                openpopuptemp = true;
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (openpopuptemp == true)
        {
            ImGui::OpenPopup(("Create shortcut for " + labelString).c_str());
            openpopuptemp = false;
        }

        if (ImGui::BeginPopupModal(("Create shortcut for " + labelString).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            Hotkey("Shortcut Key", &shortcutIndexKey);

            if (ImGui::Button("Add"))
            {
                Shortcuts::Shortcut s;
                s.key = shortcutIndexKey;
                strcpy(s.name, labelString.c_str());
                Shortcuts::shortcuts.push_back(s);
                Shortcuts::Save();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (labelString == Hacks::hackName)
        {
            Hacks::hackName = "";
            res = true;
        }
    }

    return res;
}

bool ImInputFloat(const char *label, float *v)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputFloat(label, v);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputFloat(label, v);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::InputFloat(label, v);
    return res;
}

bool ImInputInt(const char *label, int *v, int step = 1)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputInt(label, v, step);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputInt(label, v, step);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::InputInt(label, v, step);
    return res;
}

bool ImInputInt2(const char *label, int *v)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputInt2(label, v);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputInt2(label, v);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::InputInt2(label, v);
    return res;
}

bool ImInputText(const char *label, char *buf, size_t buf_size)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputText(label, buf, buf_size);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::InputText(label, buf, buf_size);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::InputText(label, buf, buf_size);
    return res;
}

bool ImCombo(const char *label, int *current_item, const char *const *items, int items_count)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::Combo(label, current_item, items, items_count);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::Combo(label, current_item, items, items_count);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::Combo(label, current_item, items, items_count);
    return res;
}

bool ImColorEdit3(const char *label, float *col, ImGuiColorEditFlags flags = 0)
{
    bool res = false;
    if (strlen(searchbar) > 0)
    {
        std::string s = label;
        if (findStringIC(s, searchbar))
        {
            ImGui::PushStyleColor(0, {1.0f, 0.4f, 0.4f, 1.0f});
            res = ImGui::ColorEdit3(label, col, flags);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(0, {0.4f, 0.4f, 0.4f, 1.0f});
            res = ImGui::ColorEdit3(label, col, flags);
            ImGui::PopStyleColor();
        }
    }
    else
        res = ImGui::ColorEdit3(label, col, flags);
    return res;
}

void InitJSONHacks(json &js)
{
    for (size_t i = 0; i < js["mods"].size(); i++)
    {
        Hacks::ToggleJSONHack(js, i, false);
    }
}

void DrawFromJSON(json &js)
{
    for (size_t i = 0; i < js["mods"].size(); i++)
    {
        if (ImCheckbox(js["mods"][i]["name"].get<std::string>().c_str(), js["mods"][i]["toggle"].get<bool *>()))
        {
            Hacks::ToggleJSONHack(js, i, false);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip(js["mods"][i]["description"].get<std::string>().c_str());
    }
}

char *convert(const std::string &s)
{
    char *pc = new char[s.size() + 1];
    std::strcpy(pc, s.c_str());
    return pc;
}

int roundInt(int n)
{
    int a = (n / hacks.windowSnap) * hacks.windowSnap;
    int b = a + hacks.windowSnap;
    return (n - a > b - n) ? b : a;
}

void TextSettings(int index, bool font)
{
    if (ImCombo(("Position##" + std::to_string(index)).c_str(), (int *)&labels.positions[index], positions, IM_ARRAYSIZE(positions)))
        for (size_t i = 0; i < STATUSSIZE; i++)
            PlayLayer::UpdatePositions(i);
    if (ImInputFloat(("Scale##" + std::to_string(index)).c_str(), &labels.scale[index]))
        for (size_t i = 0; i < STATUSSIZE; i++)
            PlayLayer::UpdatePositions(i);
    if (ImInputFloat(("Opacity##" + std::to_string(index)).c_str(), &labels.opacity[index]))
        for (size_t i = 0; i < STATUSSIZE; i++)
            PlayLayer::UpdatePositions(i);
    if (font && ImCombo(("Font##" + std::to_string(index)).c_str(), &labels.fonts[index], fonts, IM_ARRAYSIZE(fonts)))
        for (size_t i = 0; i < STATUSSIZE; i++)
            PlayLayer::UpdatePositions(i);
}

bool resetWindows = false, repositionWindows = false, saveWindows = false;

void SetStyle()
{
    ImGuiStyle *style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = hacks.windowRounding;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 2.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;
    style->WindowBorderSize = hacks.borderSize;

    style->ScaleAllSizes(screenSize * hacks.menuSize);

    float r, g, b;
    ImGui::ColorConvertHSVtoRGB(ImGui::GetTime() * hacks.menuRainbowSpeed, hacks.menuRainbowBrightness, hacks.menuRainbowBrightness, r, g, b);

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 0.03f, 0.03f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);

    if (!hacks.rainbowMenu)
    {
        style->Colors[ImGuiCol_Border] = ImVec4(hacks.borderColor[0], hacks.borderColor[1], hacks.borderColor[2], 1);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], 1);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], 1);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], 1);

        style->Colors[ImGuiCol_Tab] = ImVec4(hacks.titleColor[0] * 0.85f, hacks.titleColor[1] * 0.85f, hacks.titleColor[2] * 0.85f, 1);
        style->Colors[ImGuiCol_TabActive] = ImVec4(hacks.titleColor[0] * 0.85f, hacks.titleColor[1] * 0.85f, hacks.titleColor[2] * 0.85f, 1);
        style->Colors[ImGuiCol_TabHovered] = ImVec4(hacks.titleColor[0] * 0.70f, hacks.titleColor[1] * 0.70f, hacks.titleColor[2] * 0.70f, 1);
        style->Colors[ImGuiCol_TabUnfocused] = ImVec4(hacks.titleColor[0] * 0.60f, hacks.titleColor[1] * 0.60f, hacks.titleColor[2] * 0.60f, 1);
        style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(hacks.titleColor[0] * 0.60f, hacks.titleColor[1] * 0.60f, hacks.titleColor[2] * 0.60f, 1);
    }
    else
    {
        style->Colors[ImGuiCol_Border] = ImVec4(r, g, b, 1);
        style->Colors[ImGuiCol_TitleBg] = ImVec4(r, g, b, 1);
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(r, g, b, 1);
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(r, g, b, 1);

        style->Colors[ImGuiCol_Tab] = ImVec4(r * 0.85f, g * 0.85f, b * 0.85f, 1);
        style->Colors[ImGuiCol_TabActive] = ImVec4(r * 0.85f, g * 0.85f, b * 0.85f, 1);
        style->Colors[ImGuiCol_TabHovered] = ImVec4(r * 0.70f, g * 0.70f, b * 0.70f, 1);
        style->Colors[ImGuiCol_TabUnfocused] = ImVec4(r * 0.60f, g * 0.60f, b * 0.60f, 1);
        style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(r * 0.60f, g * 0.60f, b * 0.60f, 1);
    }

    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);

    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.80f, 0.0f, 0.61f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 0.30f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.73f);
}

bool closed = true;

void Init()
{
    closed = true;
    srand(time(NULL));

    ImGuiIO &io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.0f);
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    io.FontDefault = font;

    if (loaded)
    {
        if (hacks.dockSpace)
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        else
            io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
        return;
    }

    if (!std::filesystem::is_directory("GDMenu") || !std::filesystem::exists("GDMenu"))
    {
        std::filesystem::create_directory("GDMenu");
    }
    if (!std::filesystem::is_directory("GDMenu/renders") || !std::filesystem::exists("GDMenu/renders"))
    {
        std::filesystem::create_directory("GDMenu/renders");
    }
    if (!std::filesystem::is_directory("GDMenu/clicks") || !std::filesystem::exists("GDMenu/clicks"))
    {
        std::filesystem::create_directory("GDMenu/clicks");
    }
    if (!std::filesystem::is_directory("GDMenu/clicks/clicks") || !std::filesystem::exists("GDMenu/clicks/clicks"))
    {
        std::filesystem::create_directory("GDMenu/clicks/clicks");
    }
    if (!std::filesystem::is_directory("GDMenu/clicks/releases") || !std::filesystem::exists("GDMenu/clicks/releases"))
    {
        std::filesystem::create_directory("GDMenu/clicks/releases");
    }
    if (!std::filesystem::is_directory("GDMenu/clicks/mediumclicks") || !std::filesystem::exists("GDMenu/clicks/mediumclicks"))
    {
        std::filesystem::create_directory("GDMenu/clicks/mediumclicks");
    }
    if (!std::filesystem::is_directory("GDMenu/dll") || !std::filesystem::exists("GDMenu/dll"))
    {
        std::filesystem::create_directory("GDMenu/dll");
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    if (tm.tm_mon == 11)
    {
        isDecember = true;
    }

    for (std::filesystem::directory_entry loop : std::filesystem::directory_iterator{Hacks::GetSongFolder()})
    {
        if (loop.path().extension().string() == ".mp3")
        {
            Hacks::musicPaths.push_back(loop.path().string());
        }
    }

    for (size_t i = 0; i < Hacks::musicPaths.size(); i++)
    {
        musicPathsVet.push_back(Hacks::musicPaths[i].c_str());
    }

    Hacks::AnticheatBypass();

    std::ifstream mods;
    std::stringstream buffer;

    mods.open("GDMenu/mod/bypass.json");
    buffer << mods.rdbuf();
    Hacks::bypass = json::parse(buffer.str());
    hackAmts[0] = Hacks::bypass["mods"].size();
    mods.close();
    buffer.str("");
    buffer.clear();
    InitJSONHacks(Hacks::bypass);

    mods.open("GDMenu/mod/creator.json");
    buffer << mods.rdbuf();
    Hacks::creator = json::parse(buffer.str());
    hackAmts[1] = Hacks::creator["mods"].size();
    mods.close();
    buffer.str("");
    buffer.clear();
    InitJSONHacks(Hacks::creator);

    mods.open("GDMenu/mod/global.json");
    buffer << mods.rdbuf();
    Hacks::global = json::parse(buffer.str());
    hackAmts[2] = Hacks::global["mods"].size();
    mods.close();
    buffer.str("");
    buffer.clear();
    InitJSONHacks(Hacks::global);

    mods.open("GDMenu/mod/level.json");
    buffer << mods.rdbuf();
    Hacks::level = json::parse(buffer.str());
    hackAmts[3] = Hacks::level["mods"].size();
    mods.close();
    buffer.str("");
    buffer.clear();
    InitJSONHacks(Hacks::level);

    mods.open("GDMenu/mod/player.json");
    buffer << mods.rdbuf();
    Hacks::player = json::parse(buffer.str());
    hackAmts[4] = Hacks::player["mods"].size();
    mods.close();
    buffer.str("");
    buffer.clear();
    InitJSONHacks(Hacks::player);

    std::ofstream w;
    if (!std::filesystem::exists("imgui.ini") || !std::filesystem::exists("GDmenu/windows.bin"))
    {
        w.open("GDmenu/windows.bin", std::fstream::binary);
        windowPositions.positions[0] = {1210, 710};
        windowPositions.positions[1] = {10, 10};
        windowPositions.positions[2] = {250, 10};
        windowPositions.positions[3] = {730, 10};
        windowPositions.positions[4] = {1210, 10};
        windowPositions.positions[5] = {490, 10};
        windowPositions.positions[6] = {970, 10};
        windowPositions.positions[7] = {1690, 10};
        windowPositions.positions[8] = {1450, 10};
        windowPositions.positions[9] = {10, 720};
        windowPositions.positions[10] = {1690, 580};
        windowPositions.positions[11] = {10, 260};
        w.write((char *)&windowPositions, sizeof(windowPositions));
        w.close();

        if (!std::filesystem::exists("imgui.ini"))
            resetWindows = true;
    }

    std::ifstream f;
    f.open("GDMenu/settings.bin", std::fstream::binary);
    if (f)
    {
        f.read((char *)&hacks, sizeof(HacksStr));
        Hacks::level["mods"][24]["toggle"] = false;

        Hacks::FPSBypass(hacks.fps);
        Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
        Hacks::WriteRef(gd::base + 0x20A677, hacks.respawnTime);
        hacks.recording = false;

        Hacks::Priority(hacks.priority);
        Hacks::tps = hacks.tpsBypass;
        Hacks::screenFps = hacks.screenFPS;
    }
    else
    {
        Hacks::FPSBypass(60);
    }

    if (hacks.dockSpace)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    else
        io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;

    f.close();

    f.open("GDMenu/windows.bin", std::fstream::binary);
    if (f)
    {
        f.read((char *)&windowPositions, sizeof(windowPositions));
    }

    f.close();

    f.open("GDMenu/labels.bin", std::fstream::binary);
    if (f)
    {
        f.read((char *)&labels, sizeof(Labels));
    }

    f.close();

    Shortcuts::Load();

    Hacks::ds.InitDiscord();

    auto path = CCFileUtils::sharedFileUtils()->getWritablePath2() + "GDMenu/dll";

    for (const auto &file : std::filesystem::directory_iterator(path))
    {
        LoadLibrary(file.path().string().c_str());
    }

    loaded = true;
}

void Hacks::RenderMain()
{
    if (Hacks::ds.core)
        Hacks::ds.core->RunCallbacks();

    const int windowWidth = 220;
    const int arrowButtonPosition = windowWidth - 39;

    if (hacks.hitboxMultiplier <= 0)
        hacks.hitboxMultiplier = 1;

    const float size = screenSize * hacks.menuSize;
    const float windowSize = windowWidth * size;

    const auto playLayer = gd::GameManager::sharedState()->getPlayLayer();

    if (!playLayer)
    {
        hacks.recording = false;
        if (ReplayPlayer::getInstance().recorder.m_renderer.m_texture && ReplayPlayer::getInstance().recorder.m_recording)
            ReplayPlayer::getInstance().recorder.stop();
    }

    if (Hacks::show || debug.enabled)
        SetStyle();

    if (debug.enabled)
    {
        ImGui::PushStyleColor(0, {1, 1, 1, 1});
        // ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Debug");

        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        ImInputFloat("N", &debug.debugNumber);
        ImGui::Text(debug.debugString.c_str());

        if (ImButton("Copy levelstring"))
        {
            if (playLayer)
                ImGui::SetClipboardText(playLayer->m_level->levelString.c_str());
        }

        ImGui::End();

        ImGui::Begin("CocosExplorer by Mat", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);

        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        CocosExplorer::draw();

        ImGui::End();
        ImGui::PopStyleColor();
    }

    if (Hacks::show)
    {

        oldScreenSize = screenSize;
        if (CCDirector::sharedDirector()->getOpenGLView()->getFrameSize().width != screenSize)
        {
            screenSize = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize().width / 1920.0f;
            repositionWindows = true;
        }

        if (oldScreenSize == 0)
            oldScreenSize = screenSize;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, hacks.windowRounding);
        if (!Hacks::fake)
            cocos2d::CCEGLView::sharedOpenGLView()->showCursor(true);
        closed = false;

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Menu Settings", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[0].x * screenSize, windowPositions.positions[0].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[0] = {(int)pos.x, (int)pos.y};
        }

        ImGui::PushItemWidth(70 * screenSize * hacks.menuSize);
        ImInputFloat("Menu UI Size", &hacks.menuSize);
        if (hacks.menuSize > 3)
            hacks.menuSize = 1;
        else if (hacks.menuSize < 0.5f)
            hacks.menuSize = 0.5f;
        ImInputFloat("Border Size", &hacks.borderSize);
        ImInputFloat("Window Rounding", &hacks.windowRounding);
        ImInputInt("Window Snap", &hacks.windowSnap, 0);
        ImGui::PopItemWidth();

        ImColorEdit3("Window Title BG Color", hacks.titleColor, ImGuiColorEditFlags_NoInputs);
        ImColorEdit3("Border Color", hacks.borderColor, ImGuiColorEditFlags_NoInputs);

        ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
        if (ImHotkey("Toggle Menu", &hacks.menuKey))
        {
            ImGuiHook::setKeybind(hacks.menuKey);
        }
        ImGui::PopItemWidth();

        ImCheckbox("Experimental Features", &hacks.experimentalFeatures);

        if (isDecember)
            ImCheckbox("Snow", &hacks.snow);

        ImCheckbox("Rainbow Menu", &hacks.rainbowMenu);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::BeginMenu("##rain"))
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImInputFloat("Rainbow Speed", &hacks.menuRainbowSpeed);
            ImInputFloat("Rainbow Brightness", &hacks.menuRainbowBrightness);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImCheckbox("Docking", &hacks.dockSpace))
        {
            ImGuiIO &io = ImGui::GetIO();
            if (hacks.dockSpace)
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            else
                io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
        }

        if (ImButton("Save Windows"))
            saveWindows = true;
        if (ImButton("Load Windows"))
            resetWindows = true;

        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        ImGui::InputText("Search", searchbar, 30);
        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("General Mods", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[1].x * screenSize, windowPositions.positions[1].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }

        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[1] = {(int)pos.x, (int)pos.y};
        }

        ImGui::PushItemWidth(70 * screenSize * hacks.menuSize);
        ImInputFloat("FPS Bypass", &hacks.fps);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Changes Max FPS. Disable VSync both in gd and your gpu drivers for it to work.");
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            if (hacks.fps <= 1)
                hacks.fps = 60;
            Hacks::FPSBypass(hacks.fps);
        }

        ImInputFloat("##TPSBypass", &hacks.tpsBypass);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Changes how many times the physics gets updated every second.");
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Hacks::tps = hacks.tpsBypass;
        }
        ImGui::SameLine();
        ImCheckbox("TPS Bypass", &hacks.tpsBypassBool);
        ImInputFloat("##Draw Divide", &hacks.screenFPS);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Changes how many frames of the game will actually be rendered, otherwise they will be only processed.");
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Hacks::screenFps = hacks.screenFPS;
        }
        ImGui::SameLine();
        ImCheckbox("Draw Divide", &hacks.drawDivideBool);

        ImInputFloat("##Speed hack", &hacks.speed);
        if (ImGui::IsItemDeactivatedAfterEdit() && hacks.speedhackBool)
        {
            if (hacks.speed <= 0)
                hacks.speed = 1;
            Hacks::Speedhack(hacks.speed);

            if (hacks.tieMusicToSpeed)
                SpeedhackAudio::set(hacks.speed);
        }
        ImGui::SameLine();
        if (ImCheckbox("Speedhack", &hacks.speedhackBool))
        {
            Hacks::Speedhack(hacks.speedhackBool ? hacks.speed : 1.0f);
            if (hacks.tieMusicToSpeed)
                SpeedhackAudio::set(hacks.speedhackBool ? hacks.speed : 1.0f);
        }
        if (hacks.tieMusicToSpeed)
        {
            ImGui::BeginDisabled();
            ImInputFloat("Music Speed", &hacks.musicSpeed);
            ImGui::EndDisabled();
        }
        else
            ImInputFloat("Music Speed", &hacks.musicSpeed);

        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            SpeedhackAudio::set(hacks.musicSpeed);
        }

        ImCombo("Thread Priority", &hacks.priority, priorities, 5);
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            Hacks::Priority(hacks.priority);
        }

        if (ImCheckbox("Tie Music to Gamespeed", &hacks.tieMusicToSpeed))
        {
            SpeedhackAudio::set(hacks.tieMusicToSpeed ? hacks.speed : hacks.musicSpeed);
        }

        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Global", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[2].x * screenSize, windowPositions.positions[2].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[2] = {(int)pos.x, (int)pos.y};
        }

        ImCheckbox("Auto Deafen", &hacks.autoDeafen);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("autod", 1))
            ImGui::OpenPopup("Auto Deafen Settings");

        if (ImGui::BeginPopupModal("Auto Deafen Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImInputFloat("Auto Deafen %", &hacks.percentage);

            ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
            ImHotkey("Mute Key", &hacks.muteKey);
            ImGui::PopItemWidth();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Set a key combination in discord with leftalt + the key you set here");
            ImGui::PopItemWidth();
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Discord RPC", &hacks.discordRPC))
        {
            if (!hacks.discordRPC)
            {
                if (Hacks::ds.core)
                    Hacks::ds.core->ActivityManager().ClearActivity([](discord::Result result) {});
            }
        }

        ImCheckbox("Hide Pause Menu", &hacks.hidePause);

        ImCheckbox("Custom Menu Music", &hacks.replaceMenuMusic);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("custmm", 1))
            ImGui::OpenPopup("Custom Menu Music Settings");

        if (ImGui::BeginPopupModal("Custom Menu Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            if (ImButton("Select Song"))
            {
                auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath(),
                                                {"Audio File", "*.mp3"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    std::filesystem::path p = filename;
                    memset(hacks.menuSongId, 0, sizeof(hacks.menuSongId));
                    p.stem().string().copy(hacks.menuSongId, 10);
                }
                Hacks::MenuMusic();
            }
            ImGui::PopItemWidth();
            ImCheckbox("Random Menu Music", &hacks.randomMusic);
            if (Hacks::path.empty())
                Hacks::path = Hacks::musicPaths[hacks.randomMusic ? hacks.randomMusicIndex : hacks.musicIndex];

            std::string diobono = hacks.menuSongId;
            if (hacks.randomMusic)
                ImGui::Text(("Playing: " + Hacks::path.filename().string()).c_str());
            else
                ImGui::Text(("Playing: " + diobono).c_str());
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        DrawFromJSON(Hacks::global);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Level", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[3].x * screenSize, windowPositions.positions[3].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[3] = {(int)pos.x, (int)pos.y};
        }

        ImCheckbox("StartPos Switcher", &hacks.startPosSwitcher);
        ImCheckbox("Smart StartPos", &hacks.smartStartPos);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("smar", 1))
            ImGui::OpenPopup("Smart StartPos Settings");

        if (ImGui::BeginPopupModal("Smart StartPos Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImCheckbox("Enable Gravity Detection", &hacks.gravityDetection);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Enable auto detection of gravity, might not work since it does not account for blue pads or orbs");
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImCheckbox("Show Hitboxes", &hacks.showHitboxes);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("hit", 1))
            ImGui::OpenPopup("Hitbox Settings");

        if (ImGui::BeginPopupModal("Hitbox Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImCheckbox("Only on Death", &hacks.onlyOnDeath);
            ImCheckbox("Show Decorations", &hacks.showDecorations);
            ImCheckbox("Coin Tracker", &hacks.coinTracker);
            ImCheckbox("Hitboxes only", &hacks.hitboxOnly);
            ImCheckbox("Hitbox trail", &hacks.hitboxTrail);
            ImInputFloat("Trail Length", &hacks.hitboxTrailLength);
            ImInputInt("Hitbox Opacity", &hacks.borderOpacity, 0);
            ImInputInt("Fill Opacity", &hacks.hitboxOpacity, 0);
            ImInputFloat("Hitbox Thickness", &hacks.hitboxThickness);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }
        ImCheckbox("Layout Mode", &hacks.layoutMode);
        ImCheckbox("Hide Attempts", &hacks.hideattempts);

        DrawFromJSON(Hacks::level);

        ImCheckbox("Auto Safe Mode", &hacks.autoSafeMode);
        ImCheckbox("Practice Fix", &hacks.fixPractice);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Activate this if you want the practice fixes to be active even if macrobot is not recording");

        ImCheckbox("Auto Sync Music", &hacks.autoSyncMusic);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("ausm", 1))
            ImGui::OpenPopup("Auto Sync Music Settings");

        if (ImGui::BeginPopupModal("Auto Sync Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImInputInt("Max Desync Amount (ms)", &hacks.musicMaxDesync, 0);
            if (ImButton("Sync Now"))
                PlayLayer::SyncMusic();
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImCheckbox("Confirm Quit", &hacks.confirmQuit);
        ImCheckbox("Show Endscreen Info", &hacks.showExtraInfo);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("sei", 1))
            ImGui::OpenPopup("Endscreen Settings");

        if (ImGui::BeginPopupModal("Endscreen Settings", NULL))
        {
            ImCheckbox("Safe Mode", &hacks.safeModeEndscreen);
            ImCheckbox("Practice Button", &hacks.practiceButtonEndscreen);
            ImCheckbox("Cheat Indicator##chea", &hacks.cheatIndicatorEndscreen);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImGui::PushItemWidth(50 * screenSize * hacks.menuSize);
        ImCheckbox("Hitbox Multiplier", &hacks.enableHitboxMultiplier);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Requires level reload to apply properly");
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("hbm", 1))
            ImGui::OpenPopup("Hitbox Multiplier Settings");

        if (ImGui::BeginPopupModal("Hitbox Multiplier Settings", NULL))
        {
            ImInputFloat("Harards", &hacks.hitboxMultiplier);
            ImInputFloat("Solids", &hacks.hitboxSolids);
            ImInputFloat("Special", &hacks.hitboxSpecial);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Bypass", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[4].x * screenSize, windowPositions.positions[4].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[4] = {(int)pos.x, (int)pos.y};
        }

        DrawFromJSON(Hacks::bypass);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Player", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[5].x * screenSize, windowPositions.positions[5].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[5] = {(int)pos.x, (int)pos.y};
        }

        DrawFromJSON(Hacks::player);

        ImCheckbox("Void Click Fix", &hacks.voidClick);
        ImCheckbox("Lock Cursor", &hacks.lockCursor);
        ImCheckbox("2P One Key", &hacks.twoPlayerOneKey);
        if (hacks.experimentalFeatures)
        {
            ImCheckbox("Show Trajectory", &hacks.trajectory);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Feature not complete, does not work with portals or rings.");
        }
        ImCheckbox("Rainbow Icons", &hacks.rainbowIcons);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("rain", 1))
            ImGui::OpenPopup("Rainbow Icons Settings");

        if (ImGui::BeginPopupModal("Rainbow Icons Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImCheckbox("Rainbow Color 1", &hacks.rainbowPlayerC1);
            ImCheckbox("Rainbow Color 2", &hacks.rainbowPlayerC2);
            ImCheckbox("Rainbow Vehicle", &hacks.rainbowPlayerVehicle);
            ImCheckbox("Rainbow Glow", &hacks.rainbowOutline);

            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImInputFloat("Rainbow Speed Interval", &hacks.rainbowSpeed);
            if (ImInputFloat("Rainbow Pastel Amount", &hacks.pastel))
            {
                hacks.pastel = hacks.pastel <= 0.1f ? 0.1f : hacks.pastel > 1.0f ? 1.0f
                                                                                 : hacks.pastel;
            }
            ImGui::PopItemWidth();
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImCheckbox("No Wave Pulse", &hacks.solidWavePulse);
        ImGui::PushItemWidth(90 * screenSize * hacks.menuSize);
        if (ImInputFloat("Wave Trail Size", &hacks.waveSize))
            Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
        if (ImInputFloat("Respawn Time", &hacks.respawnTime))
            Hacks::WriteRef(gd::base + 0x20A677, hacks.respawnTime);
        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Creator", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[6].x * screenSize, windowPositions.positions[6].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[6] = {(int)pos.x, (int)pos.y};
        }

        DrawFromJSON(Hacks::creator);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Status", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[7].x * screenSize, windowPositions.positions[7].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[7] = {(int)pos.x, (int)pos.y};
        }
        ImCheckbox("Hide All", &labels.hideLabels);

        ImCheckbox("Rainbow Labels", &labels.rainbowLabels);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::BeginMenu("##rainl"))
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImInputFloat("Rainbow Speed##lab", &labels.rainbowSpeed);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImCheckbox("Cheat Indicator", &labels.statuses[0]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("ci", 1))
            ImGui::OpenPopup("Cheat Indicator Settings");
        if (ImGui::BeginPopupModal("Cheat Indicator Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(0, false);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("FPS Counter", &labels.statuses[1]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("fps", 1))
            ImGui::OpenPopup("FPS Counter Settings");
        if (ImGui::BeginPopupModal("FPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(1, true);
            ImInputText("Style##fpsc", labels.styles[0], 15);
            ImInputFloat("Update Interval", &labels.fpsUpdate);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("CPS Counter", &labels.statuses[2]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("cps", 1))
            ImGui::OpenPopup("CPS Counter Settings");
        if (ImGui::BeginPopupModal("CPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(2, true);
            ImInputText("Style##fpsc", labels.styles[1], 15);
            ImColorEdit3("Clicked Color", hacks.clickColor, ImGuiColorEditFlags_NoInputs);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Noclip accuracy", &labels.statuses[3]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("nca", 1))
            ImGui::OpenPopup("Noclip Accuracy Settings");

        if (ImGui::BeginPopupModal("Noclip Accuracy Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(3, true);
            ImInputText("Style##noclipacc", labels.styles[2], 15);
            ImInputFloat("Noclip Accuracy limit", &hacks.noClipAccuracyLimit);
            ImCheckbox("Enable Screen Effect", &hacks.noclipRed);
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImInputFloat("Opacity Limit", &hacks.noclipRedLimit);
            ImInputFloat("Opacity Rate Up", &hacks.noclipRedRate);
            ImInputFloat("Opacity Rate Down", &hacks.noclipRedRateDown);
            ImColorEdit3("Overlay Color", hacks.noclipColor, ImGuiColorEditFlags_NoInputs);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Noclip deaths", &labels.statuses[4]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("ncd", 1))
            ImGui::OpenPopup("Noclip Deaths Settings");

        if (ImGui::BeginPopupModal("Noclip Deaths Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImInputText("Style##noclipdeaths", labels.styles[3], 15);
            TextSettings(4, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Clock", &labels.statuses[5]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("clock", 1))
            ImGui::OpenPopup("Clock Settings");
        if (ImGui::BeginPopupModal("Clock Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(5, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Best Run", &labels.statuses[6]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("best run", 1))
            ImGui::OpenPopup("Best Run Settings");
        if (ImGui::BeginPopupModal("Best Run Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(6, true);
            ImCheckbox("Accumulate Runs", &hacks.accumulateRuns);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Attempts", &labels.statuses[7]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("attempts", 1))
            ImGui::OpenPopup("Attempts Settings");
        if (ImGui::BeginPopupModal("Attempts Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(7, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("From %", &labels.statuses[8]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("from", 1))
            ImGui::OpenPopup("From % Settings");
        if (ImGui::BeginPopupModal("From % Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(8, true);
            ImCheckbox("Only in Runs", &hacks.onlyInRuns);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Message Status", &labels.statuses[9]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("stat", 1))
            ImGui::OpenPopup("Message Status Settings");
        if (ImGui::BeginPopupModal("Message Status Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(9, true);
            ImInputText("Message", hacks.message, 30);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Current Attempt", &labels.statuses[10]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("curr", 1))
            ImGui::OpenPopup("Current Attempt Settings");
        if (ImGui::BeginPopupModal("Current Attempt Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(10, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Level ID", &labels.statuses[11]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("levelid", 1))
            ImGui::OpenPopup("Level ID Settings");
        if (ImGui::BeginPopupModal("Level ID Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(11, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Jumps", &labels.statuses[12]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("jumps", 1))
            ImGui::OpenPopup("Jumps Settings");
        if (ImGui::BeginPopupModal("Jumps Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(12, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        if (ImCheckbox("Frame", &labels.statuses[13]))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("frame", 1))
            ImGui::OpenPopup("Frame Settings");
        if (ImGui::BeginPopupModal("Frame Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            TextSettings(13, true);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        if (ImInputFloat("Label Spacing", &labels.labelSpacing))
            for (size_t i = 0; i < STATUSSIZE; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Shortcuts", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[8].x * screenSize, windowPositions.positions[8].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[8] = {(int)pos.x, (int)pos.y};
        }

        if (Shortcuts::shortcuts.size() == 0)
        {
            ImGui::Text("Right click an option\nto make a shortcut.");
            ImGui::Spacing();
        }

        for (size_t i = 0; i < Shortcuts::shortcuts.size(); i++)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::Text(KeyNames[Shortcuts::shortcuts[i].key]);
            ImGui::SameLine();
            ImGui::Text(Shortcuts::shortcuts[i].name);
            ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
            if (ImButton(("x##" + std::to_string(i)).c_str()))
            {
                Shortcuts::shortcuts.erase(Shortcuts::shortcuts.begin() + i);
                Shortcuts::Save();
            }

            ImGui::Separator();
        }

        if (ImButton("Open GD Settings"))
            gd::OptionsLayer::addToCurrentScene(false);
        if (ImButton("Open Song Folder"))
        {
            ShellExecute(0, NULL, Hacks::GetSongFolder().c_str(), NULL, NULL, SW_SHOW);
        }

        if (ImButton("Uncomplete Level"))
        {
            if (playLayer)
            {
                gd::GameStatsManager::sharedState()->unCompleteLevel(playLayer->m_level);
                playLayer->m_level->set1(0);
                playLayer->m_level->set2(0);
                playLayer->m_level->set3(0);
                playLayer->m_level->practicePercent = 0;
                playLayer->m_level->orbCompletion = 0;
                if (playLayer->m_level->dailyID)
                    playLayer->m_level->set4(0);
                /* playLayer->m_level->set5(0);
                playLayer->m_level->set6(0); */
                playLayer->m_level->set7(0);
                playLayer->m_level->set8(0);
                playLayer->m_level->set9(0);
                for (size_t i = 0; i < playLayer->m_level->coins; i++)
                {
                    auto coinDict = gd::GameStatsManager::sharedState()->m_pVerifiedUserCoins;
                    coinDict->removeObjectForKey(playLayer->m_level->getCoinKey(i + 1));
                }

                // auto currDict = gd::GameStatsManager::sharedState()->m_pChallengeDiamonds;
                // currDict->removeObjectForKey(gd::GameStatsManager::sharedState()->getRewardKey(playLayer->m_level));
            }
        }

        if (ImButton("Inject DLL"))
        {
            auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath2(),
                                            {"DLL File", "*.dll"},
                                            pfd::opt::multiselect)
                                 .result();
            for (auto const &filename : selection)
                LoadLibrary(filename.c_str());
        }

        if (ImButton("Reset Level"))
        {
            if (playLayer)
                PlayLayer::resetLevelHook(playLayer, 0);
        }

        if (ImButton("Toggle Practice Mode"))
        {
            if (playLayer)
                PlayLayer::togglePracticeModeHook(playLayer, 0, !playLayer->m_isPracticeMode);
        }

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Pitch Shift", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[9].x * screenSize, windowPositions.positions[9].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[9] = {(int)pos.x, (int)pos.y};
        }

        ImGui::PushItemWidth(120 * screenSize * hacks.menuSize);
        if (ImButton("Select Song##pitch"))
        {
            auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath(),
                                            {"Audio File", "*.mp3"},
                                            pfd::opt::none)
                                 .result();
            for (auto const &filename : selection)
            {
                std::filesystem::path p = filename;
                memset(hacks.pitchId, 0, sizeof(hacks.pitchId));
                p.stem().string().copy(hacks.pitchId, 10);
            }
        }

        ImGui::SameLine();
        ImGui::Text(hacks.pitchId);

        ImInputFloat("Pitch", &pitch);
        ImGui::PopItemWidth();

        if (ImButton("Render"))
            Hacks::ChangePitch(pitch);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Nong Downloader", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[12].x * screenSize, windowPositions.positions[12].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[12] = {(int)pos.x, (int)pos.y};
        }

        ImGui::PushItemWidth(120 * screenSize * hacks.menuSize);

        ImInputText("Song Url", url, 50);
        ImInputText("Song Id", id, 30);
        ImGui::PopItemWidth();

        if (ImButton("Download"))
            Hacks::NongDownload(url, id);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Internal Recorder", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[10].x * screenSize, windowPositions.positions[10].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[10] = {(int)pos.x, (int)pos.y};
        }

        if (ImCheckbox("Record", &hacks.recording))
        {
            if (!playLayer)
                return;

            if (!hacks.recording)
            {
                if (ReplayPlayer::getInstance().recorder.m_renderer.m_texture)
                    ReplayPlayer::getInstance().recorder.stop();
            }
            else
            {
                ReplayPlayer::getInstance().recorder.start();
            }
        }
        ImCheckbox("Include Clicks", &hacks.includeClicks);
        ImGui::PushItemWidth(110 * screenSize * hacks.menuSize);
        ImInputInt2("Size##videosize", hacks.videoDimenstions);
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(75 * screenSize * hacks.menuSize);
        ImInputInt("Framerate", &hacks.videoFps, 0);
        ImInputFloat("Music Volume", &hacks.renderMusicVolume);
        ImInputFloat("Click Volume", &hacks.renderClickVolume);
        ImInputText("Bitrate", hacks.bitrate, 8);
        ImInputText("Codec", hacks.codec, 20);
        ImInputText("Extraargs Before -i", hacks.extraArgs, 60);
        ImInputText("Extraargs After -i", hacks.extraArgsAfter, 60);
        ImInputInt("Click Chunk Size", &hacks.clickSoundChunkSize, 0);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("How many actions does a click chunk file contains? A click chunk file is a part of the whole rendered clicks, i have to split them to bypass the command character limit.\nTry increasing this if the clicks do not render.");
        ImInputFloat("Show End For", &hacks.afterEndDuration);
        ImGui::PopItemWidth();

        Marker("Usage", "Hit record in a level and let a macro play. The rendered video will be in GDmenu/renders/level - levelid. If you're unsure of what a setting does, leave it on default.\n If you're using an NVIDIA GPU i reccomend settings your extra args before -i to: -hwaccel cuda -hwaccel_output_format cuda and the encoder to: h264_nvenc.\n If you're using an AMD GPU i reccomend setting the encoder to either: h264_amf or hevc_amf.");
        Marker("Credits", "All the credits for the recording side goes to matcool's replaybot implementation, i integrated my clickbot into it");

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Macrobot", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({windowPositions.positions[11].x * screenSize, windowPositions.positions[11].y * screenSize});
        else if (repositionWindows)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({pos.x * (screenSize / oldScreenSize), pos.y * (screenSize / oldScreenSize)});
        }
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }
        if (saveWindows)
        {
            auto pos = ImGui::GetWindowPos();
            windowPositions.positions[11] = {(int)pos.x, (int)pos.y};
        }
        if (ReplayPlayer::getInstance().IsRecording())
            ImGui::PushStyleColor(0, ImVec4(0, 1, 0, 1));
        else
            ImGui::PushStyleColor(0, ImVec4(1, 0, 0, 1));
        if (ImButton("Toggle Recording"))
            ReplayPlayer::getInstance().ToggleRecording();
        if (ReplayPlayer::getInstance().IsPlaying())
            ImGui::PushStyleColor(0, ImVec4(0, 1, 0, 1));
        else
            ImGui::PushStyleColor(0, ImVec4(1, 0, 0, 1));
        if (ImButton("Toggle Playing"))
            ReplayPlayer::getInstance().TogglePlaying();
        else
            ImGui::PushStyleColor(0, ImVec4(1, 1, 1, 1));
        ImCheckbox("Show Replay Label", &hacks.botTextEnabled);

        ImCheckbox("Click sounds", &hacks.clickbot);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("clicks", 1))
            ImGui::OpenPopup("Click sounds settings");
        if (ImGui::BeginPopupModal("Click sounds settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImInputFloat("Click volume", &hacks.baseVolume);
            ImInputFloat("Max pitch variation", &hacks.maxPitch);
            ImInputFloat("Min pitch variation", &hacks.minPitch);
            ImGui::InputDouble("Play Medium Clicks at", &hacks.playMediumClicksAt);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Amount of time between click so that a medium click is played");
            ImInputFloat("Minimum time difference", &hacks.minTimeDifference);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Minimum time difference for a click to play, to avoid tiny double clicks");
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            Marker("?", "Put clicks, releases and mediumclicks in the respective folders found in GDMenu/clicks");
            if (!Hacks::fake)
                ImGui::EndPopup();
        }
        ImCheckbox("Prevent inputs", &hacks.preventInput);

        ImCheckbox("Disable Corrections", &hacks.disableBotCorrection);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Disable physics correction with the bot, only uses the clicks.");

        ImCheckbox("Autoclicker", &hacks.autoclicker);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("aut", 1))
            ImGui::OpenPopup("Autoclicker Settings");

        if (ImGui::BeginPopupModal("Autoclicker Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImInputFloat("Click time", &hacks.clickTime);
            ImInputFloat("Release time", &hacks.releaseTime);
            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImCheckbox("Frame Step", &hacks.frameStep);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("fra", 1))
            ImGui::OpenPopup("Frame Step Settings");

        if (ImGui::BeginPopupModal("Frame Step Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || Hacks::fake)
        {
            ImInputInt("Step Count", &hacks.stepCount, 0);
            ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
            ImHotkey("Step Key", &hacks.stepIndex);
            ImGui::PopItemWidth();
            ImCheckbox("Hold to Advance", &hacks.holdAdvance);

            if (ImButton("Close", false))
            {
                ImGui::CloseCurrentPopup();
            }
            if (!Hacks::fake)
                ImGui::EndPopup();
        }

        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        ImInputInt("Start At Action", &hacks.actionStart, 0);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        if (ImButton("Clear actions"))
            ReplayPlayer::getInstance().ClearActions();

        ImGui::Spacing();
        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        ImInputText("Replay Name", fileName, 30);
        ImGui::PopItemWidth();
        if (ImButton("Save"))
            ReplayPlayer::getInstance().Save(fileName);
        if (ImButton("Load"))
            ReplayPlayer::getInstance().Load(fileName);

        ImGui::SameLine();

        if (ImButton("Select File"))
        {
            auto selection = pfd::open_file("Select a macro", "GDMenu/replays",
                                            {"REPLAY file", "*.replay"},
                                            pfd::opt::none)
                                 .result();
            std::filesystem::path p = selection[0];
            ReplayPlayer::getInstance().GetReplay()->Load(p.string());
        }

        ImGui::Spacing();

        if (ImButton("Open Converter"))
        {
            ImGui::OpenPopup("Converter");
        }

        if (ImGui::BeginPopupModal("Converter"))
        {
            if (ImButton("Export JSON"))
            {
                json tasmacro;
                tasmacro["fps"] = ReplayPlayer::getInstance().GetReplay()->fps;
                tasmacro["actions"] = json::array();
                for (int i = 0; i < ReplayPlayer::getInstance().GetActionsSize(); i++)
                {
                    json action;
                    auto ac = ReplayPlayer::getInstance().GetReplay()->getActions()[i];
                    action["x"] = ac.px;
                    action["y"] = ac.py;
                    action["yAccel"] = ac.yAccel;
                    action["press"] = ac.press;
                    action["player2"] = ac.player2;
                    action["frame"] = ac.frame;
                    tasmacro["actions"].push_back(action);
                }
                std::ofstream file("GDMenu/replays/" + std::string(fileName) + ".mcb.json");
                file << tasmacro;
            }
            if (ImButton("Import JSON"))
            {
                auto selection = pfd::open_file("Select a macro", "",
                                                {"JSON File", "*.mcb.json"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    ReplayPlayer::getInstance().ClearActions();
                    std::ifstream stream(filename);
                    json tasmacro = json::parse(stream);
                    ReplayPlayer::getInstance().GetReplay()->fps = tasmacro["fps"];
                    for (size_t i = 0; i < tasmacro["actions"].size(); i++)
                    {
                        Action ac;
                        auto action = tasmacro["actions"][i];
                        ac.frame = action["frame"];
                        ac.player2 = action["player2"];
                        ac.press = action["press"];
                        ac.px = action["x"];
                        ac.py = action["y"];
                        ac.yAccel = action["yAccel"];
                        ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
                    }
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(ReplayPlayer::getInstance().GetActionsSize()) + " actions."))->show();
                }
            }
            if (ImButton("From TASBOT"))
            {
                auto selection = pfd::open_file("Select a macro", "",
                                                {"JSON File", "*.json"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    ReplayPlayer::getInstance().ClearActions();
                    std::ifstream stream(filename);
                    json tasmacro = json::parse(stream);
                    ReplayPlayer::getInstance().GetReplay()->fps = tasmacro["fps"];
                    for (size_t i = 0; i < tasmacro["macro"].size(); i++)
                    {
                        Action ac;
                        auto action = tasmacro["macro"][i];
                        ac.frame = action["frame"] + 1;
                        if (action["player_1"]["click"] > 0)
                        {
                            ac.press = action["player_1"]["click"] == 1;
                            ac.px = action["player_1"]["x_position"];
                            ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
                        }
                        if (action["player_2"]["click"] > 0)
                        {
                            ac.press = action["player_2"]["click"] == 1;
                            ac.px = action["player_2"]["x_position"];
                            ac.player2 = true;
                            ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
                        }
                    }
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(ReplayPlayer::getInstance().GetActionsSize()) + " actions."))->show();
                }
            }
            if (ImButton("From zBot"))
            {
                auto selection = pfd::open_file("Select a macro", "",
                                                {"zBot file", "*.zbot, *.zbf"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    std::filesystem::path path = filename;
                    ReplayPlayer::getInstance().ClearActions();
                    std::ifstream stream(filename, std::fstream::binary);
                    stream.seekg(0, std::fstream::end);
                    size_t size = stream.tellg();
                    size -= sizeof(float) * 2;
                    stream.seekg(0);
                    float delta, speed;
                    stream.read((char *)&delta, sizeof(float));
                    stream.read((char *)&speed, sizeof(float));
                    ReplayPlayer::getInstance().GetReplay()->fps = 1 / delta / speed;
                    for (int i = 0; i < size / ((path.extension() == ".zbot" ? sizeof(float) : sizeof(uint32_t)) + sizeof(bool) + sizeof(bool)); i++)
                    {
                        Action action;
                        action.px = -1;
                        if (path.extension() == ".zbot")
                            stream.read((char *)&action.px, sizeof(float));
                        else
                            stream.read((char *)&action.frame, sizeof(uint32_t));
                        uint8_t cacca1;
                        stream.read((char *)&cacca1, sizeof(uint8_t));
                        action.press = cacca1 == 0x31;
                        uint8_t cacca;
                        stream.read((char *)&cacca, sizeof(uint8_t));
                        action.player2 = cacca != 0x31;
                        action.yAccel = -1;
                        ReplayPlayer::getInstance().GetReplay()->AddAction(action);
                    }
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(ReplayPlayer::getInstance().GetActionsSize()) + " actions."))->show();
                }
            }
            if (ImButton("From xBot"))
            {
                auto selection = pfd::open_file("Select a macro", "",
                                                {"xBot file", "*.xbot"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    ReplayPlayer::getInstance().ClearActions();
                    std::ifstream stream(filename);
                    bool isFrame = false;
                    while (!stream.eof())
                    {
                        std::string line;
                        getline(stream, line);
                        if (line.substr(0, 3) == "fps")
                            ReplayPlayer::getInstance().GetReplay()->fps = std::stof(line.substr(5, 3));
                        else if (line == "frames")
                        {
                            isFrame = true;
                            continue;
                        }
                        else if (line != "pro_plus")
                        {
                            Action ac;
                            ac.yAccel = -1;
                            size_t clickType = std::stoi(line.substr(0, 1));
                            if (isFrame)
                            {
                                ac.frame = std::stoi(line.substr(2, line.length() - 2));
                                ac.px = -1;
                            }
                            else
                            {
                                int a = std::stoi(line.substr(2, line.length() - 2));
                                *(int *)&ac.px = a;
                                ac.frame = -1;
                            }

                            ac.press = clickType & 1;
                            ac.player2 = clickType >> 1;
                            ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
                        }
                    }
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(ReplayPlayer::getInstance().GetActionsSize()) + " actions."))->show();
                }
            }
            if (ImButton("From ECHO"))
            {
                auto selection = pfd::open_file("Select a macro", "",
                                                {"ECHO File", "*.echo"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    ReplayPlayer::getInstance().ClearActions();
                    std::ifstream stream(filename);
                    json tasmacro = json::parse(stream);
                    ReplayPlayer::getInstance().GetReplay()->fps = tasmacro["FPS"];
                    for (size_t i = 0; i < tasmacro["Echo Replay"].size(); i++)
                    {
                        Action ac;
                        auto action = tasmacro["Echo Replay"][i];
                        ac.frame = action["Frame"];
                        if (ac.frame <= 0)
                            ac.frame = -1;
                        ac.px = action["X Position"];
                        if (ac.px <= 0)
                            ac.px = -1;
                        ac.press = action["Hold"];
                        ac.player2 = action["Player 2"];
                        ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
                    }
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(ReplayPlayer::getInstance().GetActionsSize()) + " actions."))->show();
                }
            }
            if (ImButton("From MHREPLAY"))
            {
                auto selection = pfd::open_file("Select a macro", "",
                                                {"MHR File", "*.mhr.json"},
                                                pfd::opt::none)
                                     .result();
                for (auto const &filename : selection)
                {
                    ReplayPlayer::getInstance().ClearActions();
                    std::ifstream stream(filename);
                    json tasmacro = json::parse(stream);
                    ReplayPlayer::getInstance().GetReplay()->fps = tasmacro["meta"]["fps"];
                    for (size_t i = 0; i < tasmacro["events"].size(); i++)
                    {
                        Action ac;
                        auto action = tasmacro["events"][i];
                        ac.frame = action["frame"];
                        if (ac.frame <= 0)
                            ac.frame = -1;
                        ac.px = action["x"];
                        if (ac.px <= 0)
                            ac.px = -1;
                        ac.py = action["y"];
                        ac.yAccel = action["a"];
                        if (ac.yAccel <= 0)
                            ac.yAccel = -1;
                        ac.press = action["down"];
                        if (action.contains("p2"))
                            ac.player2 = action["p2"];
                        ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
                    }
                    gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Replay loaded with " + std::to_string(ReplayPlayer::getInstance().GetActionsSize()) + " actions."))->show();
                }
            }
            if (ImButton("Close"))
                ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar();

        if (resetWindows || repositionWindows)
            ImGui::SaveIniSettingsToDisk("imgui.ini");

        if (saveWindows)
        {
            std::ofstream w;
            w.open("GDmenu/windows.bin", std::fstream::binary);
            w.write((char *)&windowPositions, sizeof(windowPositions));
            w.close();
        }

        resetWindows = false;
        repositionWindows = false;
        saveWindows = false;
    }
    else if (!closed)
    {
        closed = true;
        auto p = playLayer;
        if (p && !p->m_bIsPaused && !p->m_hasCompletedLevel)
            cocos2d::CCEGLView::sharedOpenGLView()->showCursor(false);

        Hacks::SaveSettings();
    }
}

DWORD WINAPI my_thread(void *hModule)
{
    ImGuiHook::setRenderFunction(Hacks::RenderMain);
    ImGuiHook::setInitFunction(Init);
    ImGuiHook::setToggleCallback([]()
                                 { Hacks::show = !Hacks::show; });
    if (MH_Initialize() == MH_OK)
    {
        ImGuiHook::setupHooks([](void *target, void *hook, void **trampoline)
                              { MH_CreateHook(target, hook, trampoline); });
        SpeedhackAudio::init();
        auto cocos = GetModuleHandleA("libcocos2d.dll");
        auto addr = GetProcAddress(cocos, "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z");

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x01FB780), PlayLayer::initHook, reinterpret_cast<void **>(&PlayLayer::init));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x2029C0), PlayLayer::updateHook, reinterpret_cast<void **>(&PlayLayer::update));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20D0D0), PlayLayer::togglePracticeModeHook, reinterpret_cast<void **>(&PlayLayer::togglePracticeMode));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20a1a0), PlayLayer::destroyPlayer_H, reinterpret_cast<void **>(&PlayLayer::destroyPlayer));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1F4E40), PlayLayer::pushButtonHook, reinterpret_cast<void **>(&PlayLayer::pushButton));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20D810), PlayLayer::onQuitHook, reinterpret_cast<void **>(&PlayLayer::onQuit));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x15EE00), PlayLayer::editorInitHook, reinterpret_cast<void **>(&PlayLayer::editorInit));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1F4F70), PlayLayer::releaseButtonHook, reinterpret_cast<void **>(&PlayLayer::releaseButton));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20BF00), PlayLayer::resetLevelHook, reinterpret_cast<void **>(&PlayLayer::resetLevel));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20A1A0), PlayLayer::hkDeath, reinterpret_cast<void **>(&PlayLayer::death));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1FD3D0), PlayLayer::levelCompleteHook, reinterpret_cast<void **>(&PlayLayer::levelComplete));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x253D60), PlayLayer::triggerObjectHook, reinterpret_cast<void **>(&PlayLayer::triggerObject));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1FFD80), PlayLayer::lightningFlashHook, reinterpret_cast<void **>(&PlayLayer::lightningFlash));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x25FAD0), PlayLayer::uiOnPauseHook, reinterpret_cast<void **>(&PlayLayer::uiOnPause));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x25FD20), PlayLayer::uiTouchBeganHook, reinterpret_cast<void **>(&PlayLayer::uiTouchBegan));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1F9640), PlayLayer::togglePlayerScaleHook, reinterpret_cast<void **>(&PlayLayer::togglePlayerScale));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1f4ff0), PlayLayer::ringJumpHook, reinterpret_cast<void **>(&PlayLayer::ringJump));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0xef0e0), PlayLayer::activateObjectHook, reinterpret_cast<void **>(&PlayLayer::activateObject));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x10ed50), PlayLayer::bumpHook, reinterpret_cast<void **>(&PlayLayer::bump));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1FE3A0), PlayLayer::newBestHook, reinterpret_cast<void **>(&PlayLayer::newBest));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0xE4A70), PlayLayer::getObjectRectHook, reinterpret_cast<void **>(&PlayLayer::getObjectRect));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0xE4B90), PlayLayer::getObjectRectHook2, reinterpret_cast<void **>(&PlayLayer::getObjectRect2));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x203CD0), PlayLayer::checkCollisionsHook, reinterpret_cast<void **>(&PlayLayer::checkCollisions));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0xEF110), PlayLayer::hasBeenActivatedByPlayerHook, reinterpret_cast<void **>(&PlayLayer::hasBeenActivatedByPlayer));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x14ebc0), PlayLayer::addPointHook, reinterpret_cast<void **>(&PlayLayer::addPoint));

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x16B7C0), LevelEditorLayer::drawHook, reinterpret_cast<void **>(&LevelEditorLayer::draw));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x75660), LevelEditorLayer::exitHook, reinterpret_cast<void **>(&LevelEditorLayer::exit));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0xC4BD0), LevelEditorLayer::fadeMusicHook, reinterpret_cast<void **>(&LevelEditorLayer::fadeMusic));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1695A0), LevelEditorLayer::onPlaytestHook, reinterpret_cast<void **>(&LevelEditorLayer::onPlaytest));

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x94CB0), EndLevelLayer::customSetupHook, reinterpret_cast<void **>(&EndLevelLayer::customSetup));

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x12ADF0), MenuLayer::onBackHook, reinterpret_cast<void **>(&MenuLayer::onBack));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x18CF40), MenuLayer::loadingStringHook, reinterpret_cast<void **>(&MenuLayer::loadingString));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1907B0), MenuLayer::hook, reinterpret_cast<void **>(&MenuLayer::init));

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x17DA60), LevelSearchLayer::hook, reinterpret_cast<void **>(&LevelSearchLayer::init));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x9f8e0), LevelSearchLayer::httpHook, reinterpret_cast<void **>(&LevelSearchLayer::http));

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20DDD0), CustomCheckpoint::createHook, reinterpret_cast<void **>(&CustomCheckpoint::create));

        MH_CreateHook(addr, PlayLayer::dispatchKeyboardMSGHook, reinterpret_cast<void **>(&PlayLayer::dispatchKeyboardMSG));
        Setup();
        // Speedhack::Setup();

        MH_EnableHook(MH_ALL_HOOKS);
    }
    else
    {
        std::cout << "MinHook failed to load! Unloading..." << std::endl;
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    }

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}