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

bool show = false, loaded = false;
bool isDecember = false;
extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

DiscordManager Hacks::ds;

json Hacks::bypass, Hacks::creator, Hacks::global, Hacks::level, Hacks::player;

float Hacks::screenFps = 60.0f, Hacks::tps = 60.0f;

float screenSize = 0, pitch, oldScreenSize = 0;

int shortcutIndex, shortcutIndexKey, pitchName;
char fileName[30];
std::vector<std::string> Hacks::musicPaths;
std::filesystem::path Hacks::path;
std::vector<const char *> musicPathsVet;

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

void InitJSONHacks(json &js)
{
    for (size_t i = 0; i < js["mods"].size(); i++)
    {
        hackNamesString.push_back(js["mods"][i]["name"].get<std::string>());
        Hacks::ToggleJSONHack(js, i, false);
    }
}

void DrawFromJSON(json &js)
{
    for (size_t i = 0; i < js["mods"].size(); i++)
    {
        if (ImGui::Checkbox(js["mods"][i]["name"].get<std::string>().c_str(), js["mods"][i]["toggle"].get<bool *>()))
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
    if (ImGui::Combo(("Position##" + std::to_string(index)).c_str(), (int *)&labels.positions[index], positions, IM_ARRAYSIZE(positions)))
        for (size_t i = 0; i < 13; i++)
            PlayLayer::UpdatePositions(i);
    if (ImGui::InputFloat(("Scale##" + std::to_string(index)).c_str(), &labels.scale[index]))
        for (size_t i = 0; i < 13; i++)
            PlayLayer::UpdatePositions(i);
    if (ImGui::InputFloat(("Opacity##" + std::to_string(index)).c_str(), &labels.opacity[index]))
        for (size_t i = 0; i < 13; i++)
            PlayLayer::UpdatePositions(i);
    if (font && ImGui::Combo(("Font##" + std::to_string(index)).c_str(), &labels.fonts[index], fonts, IM_ARRAYSIZE(fonts)))
        for (size_t i = 0; i < 13; i++)
            PlayLayer::UpdatePositions(i);
}

bool resetWindows = false, repositionWindows = false;

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
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);

    if (!hacks.rainbowMenu)
        style->Colors[ImGuiCol_Border] = ImVec4(hacks.borderColor[0], hacks.borderColor[1], hacks.borderColor[2], hacks.borderColor[3]);
    else
        style->Colors[ImGuiCol_Border] = ImVec4(r, g, b, hacks.borderColor[3]);

    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);

    if (!hacks.rainbowMenu)
        style->Colors[ImGuiCol_TitleBg] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], hacks.titleColor[3]);
    else
        style->Colors[ImGuiCol_TitleBg] = ImVec4(r, g, b, hacks.titleColor[3]);

    if (!hacks.rainbowMenu)
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], hacks.titleColor[3]);
    else
        style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(r, g, b, hacks.titleColor[3]);

    if (!hacks.rainbowMenu)
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], hacks.titleColor[3]);
    else
        style->Colors[ImGuiCol_TitleBgActive] = ImVec4(r, g, b, hacks.titleColor[3]);

    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
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

    if (!std::filesystem::exists("imgui.ini"))
        resetWindows = true;

    ImGuiIO &io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.0f);
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    io.FontDefault = font;
    

    if (loaded)
        return;

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

    for (size_t i = 0; i < hackNamesString.size(); i++)
    {
        hackNames.push_back(hackNamesString[i].c_str());
    }

    for (size_t i = 0; i < sizeof(manualHackNames) / sizeof(const char *); i++)
    {
        hackNames.push_back(manualHackNames[i]);
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
        Hacks::FPSBypass(60.0f);
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

void RenderMain()
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

    if (show || debug.enabled)
        SetStyle();

    if (debug.enabled)
    {
        ImGui::PushStyleColor(0, {1, 1, 1, 1});
        //ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Debug");

        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        ImGui::InputFloat("N", &debug.debugNumber);
        ImGui::Text(debug.debugString.c_str());

        ImGui::End();

        ImGui::Begin("CocosExplorer by Mat", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({732 * screenSize, 12 * screenSize});
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

        CocosExplorer::draw();

        ImGui::End();
        ImGui::PopStyleColor();
    }

    if (show)
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
        cocos2d::CCEGLView::sharedOpenGLView()->showCursor(true);
        closed = false;

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Menu Settings", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({1210 * screenSize, 710 * screenSize});
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
        ImGui::PushItemWidth(70 * screenSize * hacks.menuSize);
        ImGui::InputFloat("Menu UI Size", &hacks.menuSize);
        if (hacks.menuSize > 3)
            hacks.menuSize = 1;
        else if (hacks.menuSize < 0.5f)
            hacks.menuSize = 0.5f;
        ImGui::InputFloat("Border Size", &hacks.borderSize);
        ImGui::InputFloat("Window Rounding", &hacks.windowRounding);
        ImGui::InputInt("Window Snap", &hacks.windowSnap, 0);
        ImGui::PopItemWidth();

        ImGui::ColorEdit4("Window Title BG Color", hacks.titleColor, ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4("Border Color", hacks.borderColor, ImGuiColorEditFlags_NoInputs);

        ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
        if (Hotkey("Toggle Menu", &hacks.menuKey))
        {
            ImGuiHook::setKeybind(hacks.menuKey);
        }
        ImGui::PopItemWidth();

        if (isDecember)
            ImGui::Checkbox("Snow", &hacks.snow);
        
        ImGui::CheckboxFlags("Docking", &ImGui::GetIO().ConfigFlags, ImGuiConfigFlags_DockingEnable);

        ImGui::Checkbox("Rainbow Menu", &hacks.rainbowMenu);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::BeginMenu("##rain"))
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImGui::InputFloat("Rainbow Speed", &hacks.menuRainbowSpeed);
            ImGui::InputFloat("Rainbow Brightness", &hacks.menuRainbowBrightness);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("General Mods", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({10 * screenSize, 10 * screenSize});
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

        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        ImGui::InputFloat("FPS Bypass", &hacks.fps);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Changes Max FPS. Disable VSync both in gd and your gpu drivers for it to work.");

        ImGui::InputFloat("TPS Bypass", &hacks.tpsBypass);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Changes how many times the physics gets updated every second.");
        ImGui::InputFloat("Draw Divide", &hacks.screenFPS);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Changes how many frames of the game will actually be rendered, otherwise they will be only processed.");

        ImGui::InputFloat("Speedhack", &hacks.speed);

        ImGui::Combo("Thread Priority", &hacks.priority, priorities, 5);
        ImGui::PopItemWidth();
        if (ImGui::Button("Apply##sv"))
        {
            if (hacks.fps <= 1)
                hacks.fps = 60;
            Hacks::FPSBypass(hacks.fps);
            if (hacks.speed <= 0)
                hacks.speed = 1;
            Hacks::Speedhack(hacks.speed);
            Hacks::Priority(hacks.priority);
            Hacks::tps = hacks.tpsBypass;
            Hacks::screenFps = hacks.screenFPS;
        }

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Global", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({250 * screenSize, 10 * screenSize});
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

        ImGui::Checkbox("Auto Deafen", &hacks.autoDeafen);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("autod", 1))
            ImGui::OpenPopup("Auto Deafen Settings");

        if (ImGui::BeginPopupModal("Auto Deafen Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImGui::InputFloat("Auto Deafen %", &hacks.percentage);

            ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
            Hotkey("Mute Key", &hacks.muteKey);
            ImGui::PopItemWidth();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Set a key combination in discord with leftalt + the key you set here");
            ImGui::PopItemWidth();
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Discord RPC", &hacks.discordRPC))
        {
            if (!hacks.discordRPC)
            {
                if (Hacks::ds.core)
                    Hacks::ds.core->ActivityManager().ClearActivity([](discord::Result result) {});
            }
        }

        ImGui::Checkbox("Hide Pause Menu", &hacks.hidePause);

        ImGui::Checkbox("Custom Menu Music", &hacks.replaceMenuMusic);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("custmm", 1))
            ImGui::OpenPopup("Custom Menu Music Settings");

        if (ImGui::BeginPopupModal("Custom Menu Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            if (ImGui::Button("Select Song"))
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
            ImGui::Checkbox("Random Menu Music", &hacks.randomMusic);
            if (Hacks::path.empty())
                Hacks::path = Hacks::musicPaths[hacks.randomMusic ? hacks.randomMusicIndex : hacks.musicIndex];

            std::string diobono = hacks.menuSongId;
            if(hacks.randomMusic) ImGui::Text(("Playing: " + Hacks::path.filename().string()).c_str());
            else ImGui::Text(("Playing: " + diobono).c_str());
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        DrawFromJSON(Hacks::global);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Level", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({730 * screenSize, 10 * screenSize});
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

        ImGui::Checkbox("StartPos Switcher", &hacks.startPosSwitcher);
        ImGui::Checkbox("Smart StartPos", &hacks.smartStartPos);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("smar", 1))
            ImGui::OpenPopup("Smart StartPos Settings");

        if (ImGui::BeginPopupModal("Smart StartPos Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Checkbox("Enable Gravity Detection", &hacks.gravityDetection);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Enable auto detection of gravity, might not work since it does not account for blue pads or orbs");
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Show Hitboxes", &hacks.showHitboxes);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("hit", 1))
            ImGui::OpenPopup("Hitbox Settings");

        if (ImGui::BeginPopupModal("Hitbox Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Checkbox("Only on Death", &hacks.onlyOnDeath);
            ImGui::Checkbox("Show Decorations", &hacks.showDecorations);
            ImGui::Checkbox("Hitboxes only", &hacks.hitboxOnly);
            ImGui::Checkbox("Hitbox trail", &hacks.hitboxTrail);
            ImGui::InputFloat("Trail Length", &hacks.hitboxTrailLength);
            ImGui::InputInt("Hitbox Opacity", &hacks.borderOpacity, 0);
            ImGui::InputInt("Fill Opacity", &hacks.hitboxOpacity, 0);
            ImGui::InputFloat("Hitbox Thickness", &hacks.hitboxThickness);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Layout Mode", &hacks.layoutMode);
        ImGui::Checkbox("Hide Attempts", &hacks.hideattempts);

        DrawFromJSON(Hacks::level);

        ImGui::Checkbox("Practice Fix", &hacks.fixPractice);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Activate this if you want the practice fixes to be active even if macrobot is not recording");

        ImGui::Checkbox("Auto Sync Music", &hacks.autoSyncMusic);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("ausm", 1))
            ImGui::OpenPopup("Auto Sync Music Settings");

        if (ImGui::BeginPopupModal("Auto Sync Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputInt("Max Desync Amount (ms)", &hacks.musicMaxDesync, 0);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Confirm Quit", &hacks.confirmQuit);
        ImGui::Checkbox("Show Endscreen Info", &hacks.showExtraInfo);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("sei", 1))
            ImGui::OpenPopup("Endscreen Settings");

        if (ImGui::BeginPopupModal("Endscreen Settings", NULL))
        {
            ImGui::Checkbox("Safe Mode", &hacks.safeModeEndscreen);
            ImGui::Checkbox("Practice Button", &hacks.practiceButtonEndscreen);
            ImGui::Checkbox("Cheat Indicator", &hacks.cheatIndicatorEndscreen);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PushItemWidth(50 * screenSize * hacks.menuSize);
        ImGui::Checkbox("Hitbox Multiplier", &hacks.enableHitboxMultiplier);
        if(ImGui::IsItemHovered()) ImGui::SetTooltip("Requires level reload to apply properly");
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("hbm", 1))
            ImGui::OpenPopup("Hitbox Multiplier Settings");

        if (ImGui::BeginPopupModal("Hitbox Multiplier Settings", NULL))
        {
            ImGui::InputFloat("Harards", &hacks.hitboxMultiplier);
            ImGui::InputFloat("Solids", &hacks.hitboxSolids);
            ImGui::InputFloat("Special", &hacks.hitboxSpecial);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Bypass", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({1210 * screenSize, 10 * screenSize});
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

        DrawFromJSON(Hacks::bypass);

        if (ImGui::Button("Anticheat Bypass"))
            Hacks::AnticheatBypass();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Player", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({490 * screenSize, 10 * screenSize});
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

        DrawFromJSON(Hacks::player);

        ImGui::Checkbox("Void Click Fix", &hacks.voidClick);
        ImGui::Checkbox("Rainbow Icons", &hacks.rainbowIcons);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("rain", 1))
            ImGui::OpenPopup("Rainbow Icons Settings");

        if (ImGui::BeginPopupModal("Rainbow Icons Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Checkbox("Rainbow Color 1", &hacks.rainbowPlayerC1);
            ImGui::Checkbox("Rainbow Color 2", &hacks.rainbowPlayerC2);
            ImGui::Checkbox("Rainbow Vehicle", &hacks.rainbowPlayerVehicle);
            ImGui::Checkbox("Rainbow Glow", &hacks.rainbowOutline);

            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImGui::InputFloat("Rainbow Speed Interval", &hacks.rainbowSpeed);
            ImGui::PopItemWidth();
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("No Wave Pulse", &hacks.solidWavePulse);
        ImGui::PushItemWidth(90 * screenSize * hacks.menuSize);
        if (ImGui::InputFloat("Wave Trail Size", &hacks.waveSize))
            Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
        if (ImGui::InputFloat("Respawn Time", &hacks.respawnTime))
            Hacks::WriteRef(gd::base + 0x20A677, hacks.respawnTime);
        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Creator", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({970 * screenSize, 10 * screenSize});
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

        DrawFromJSON(Hacks::creator);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Status", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({1690 * screenSize, 10 * screenSize});
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
        ImGui::Checkbox("Hide All", &labels.hideLabels);

        ImGui::Checkbox("Rainbow Labels", &labels.rainbowLabels);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::BeginMenu("##rainl"))
        {
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImGui::InputFloat("Rainbow Speed##lab", &labels.rainbowSpeed);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }

        if (ImGui::Checkbox("Cheat Indicator", &labels.statuses[0]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("ci", 1))
            ImGui::OpenPopup("Cheat Indicator Settings");
        if (ImGui::BeginPopupModal("Cheat Indicator Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(0, false);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("FPS Counter", &labels.statuses[1]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("fps", 1))
            ImGui::OpenPopup("FPS Counter Settings");
        if (ImGui::BeginPopupModal("FPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(1, true);
            ImGui::Combo("Style##fpsc", &labels.styles[0], style, IM_ARRAYSIZE(style));
            ImGui::InputFloat("Update Interval", &labels.fpsUpdate);
            ImGui::Checkbox("Show TPS", &labels.showReal);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("CPS Counter", &labels.statuses[2]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("cps", 1))
            ImGui::OpenPopup("CPS Counter Settings");
        if (ImGui::BeginPopupModal("CPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(2, true);
            ImGui::Combo("Style##cpsc", &labels.styles[1], style, IM_ARRAYSIZE(style));
            ImGui::ColorEdit3("Clicked Color", hacks.clickColor, ImGuiColorEditFlags_NoInputs);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Noclip accuracy", &labels.statuses[3]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("nca", 1))
            ImGui::OpenPopup("Noclip Accuracy Settings");

        if (ImGui::BeginPopupModal("Noclip Accuracy Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(3, true);
            ImGui::InputFloat("Noclip Accuracy limit", &hacks.noClipAccuracyLimit);
            ImGui::Checkbox("Enable Screen Effect", &hacks.noclipRed);
            ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
            ImGui::InputFloat("Opacity Limit", &hacks.noclipRedLimit);
            ImGui::InputFloat("Opacity Rate Up", &hacks.noclipRedRate);
            ImGui::InputFloat("Opacity Rate Down", &hacks.noclipRedRateDown);
            ImGui::ColorEdit3("Overlay Color", hacks.noclipColor, ImGuiColorEditFlags_NoInputs);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Noclip deaths", &labels.statuses[4]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("ncd", 1))
            ImGui::OpenPopup("Noclip Deaths Settings");

        if (ImGui::BeginPopupModal("Noclip Deaths Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(4, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Clock", &labels.statuses[5]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("clock", 1))
            ImGui::OpenPopup("Clock Settings");
        if (ImGui::BeginPopupModal("Clock Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(5, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Best Run", &labels.statuses[6]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("best run", 1))
            ImGui::OpenPopup("Best Run Settings");
        if (ImGui::BeginPopupModal("Best Run Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(6, true);
            ImGui::Checkbox("Accumulate Runs", &hacks.accumulateRuns);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Attempts", &labels.statuses[7]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("attempts", 1))
            ImGui::OpenPopup("Attempts Settings");
        if (ImGui::BeginPopupModal("Attempts Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(7, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("From %", &labels.statuses[8]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("from", 1))
            ImGui::OpenPopup("From % Settings");
        if (ImGui::BeginPopupModal("From % Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(8, true);
            ImGui::Checkbox("Only in Runs", &hacks.onlyInRuns);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Message Status", &labels.statuses[9]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("stat", 1))
            ImGui::OpenPopup("Message Status Settings");
        if (ImGui::BeginPopupModal("Message Status Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(9, true);
            ImGui::InputText("Message", hacks.message, 30);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Current Attempt", &labels.statuses[10]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("curr", 1))
            ImGui::OpenPopup("Current Attempt Settings");
        if (ImGui::BeginPopupModal("Current Attempt Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(10, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Level ID", &labels.statuses[11]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("levelid", 1))
            ImGui::OpenPopup("Level ID Settings");
        if (ImGui::BeginPopupModal("Level ID Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(11, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("Jumps", &labels.statuses[12]))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("jumps", 1))
            ImGui::OpenPopup("Jumps Settings");
        if (ImGui::BeginPopupModal("Jumps Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(12, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        if (ImGui::InputFloat("Label Spacing", &labels.labelSpacing))
            for (size_t i = 0; i < 13; i++)
                PlayLayer::UpdatePositions(i);
        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Shortcuts", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({1450 * screenSize, 10 * screenSize});
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

        ImGui::PushItemWidth(150 * screenSize * hacks.menuSize);
        ImGui::Combo("Hack", &shortcutIndex, hackNames.data(), hackNames.size());
        ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
        Hotkey("Shortcut Key", &shortcutIndexKey);
        ImGui::PopItemWidth();

        if (ImGui::Button("Add"))
        {

            bool addShortcut = true;

            for (size_t i = 0; i < Shortcuts::shortcuts.size(); i++)
            {
                if (Shortcuts::shortcuts[i].key == shortcutIndexKey && Shortcuts::shortcuts[i].shortcutIndex == shortcutIndex)
                    addShortcut = false;
            }

            if (addShortcut)
            {
                Shortcuts::Shortcut s;
                s.key = shortcutIndexKey;
                s.shortcutIndex = shortcutIndex;
                Shortcuts::shortcuts.push_back(s);
                Shortcuts::Save();
            }
        }

        ImGui::Separator();

        for (size_t i = 0; i < Shortcuts::shortcuts.size(); i++)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::Text(KeyNames[Shortcuts::shortcuts[i].key]);
            ImGui::SameLine();
            ImGui::Text(hackNames[Shortcuts::shortcuts[i].shortcutIndex]);
            ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
            if (ImGui::Button(("x##" + std::to_string(i)).c_str()))
            {
                Shortcuts::shortcuts.erase(Shortcuts::shortcuts.begin() + i);
                Shortcuts::Save();
            }

            ImGui::Separator();
        }
        ImGui::PopItemWidth();

        if (ImGui::Button("Open GD Settings"))
            gd::OptionsLayer::addToCurrentScene(false);
        if (ImGui::Button("Open Song Folder"))
        {
            ShellExecute(0, NULL, Hacks::GetSongFolder().c_str(), NULL, NULL, SW_SHOW);
        }

        if (ImGui::Button("Uncomplete Level"))
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

        if (ImGui::Button("Inject DLL"))
        {
            auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath2(),
                                            {"DLL File", "*.dll"},
                                            pfd::opt::multiselect)
                                 .result();
            for (auto const &filename : selection)
                LoadLibrary(filename.c_str());
        }

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Pitch Shift", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({10 * screenSize, 720 * screenSize});
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

        ImGui::PushItemWidth(120 * screenSize * hacks.menuSize);
        if (ImGui::Button("Select Song##pitch"))
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

        ImGui::InputFloat("Pitch", &pitch);
        ImGui::PopItemWidth();

        if (ImGui::Button("Render"))
            Hacks::ChangePitch(pitch);

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Internal Recorder", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({1690 * screenSize, 580 * screenSize});
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

        if (hacks.windowSnap > 1)
        {
            auto pos = ImGui::GetWindowPos();
            ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
        }

        if (ImGui::Checkbox("Record", &hacks.recording))
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
        ImGui::Checkbox("Include Clicks", &hacks.includeClicks);
        ImGui::PushItemWidth(110 * screenSize * hacks.menuSize);
        ImGui::InputInt2("Size##videosize", hacks.videoDimenstions);
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(75 * screenSize * hacks.menuSize);
        ImGui::InputInt("Framerate", &hacks.videoFps, 0);
        ImGui::InputFloat("Music Volume", &hacks.renderMusicVolume);
        ImGui::InputFloat("Click Volume", &hacks.renderClickVolume);
        ImGui::InputText("Bitrate", hacks.bitrate, 8);
        ImGui::InputText("Codec", hacks.codec, 20);
        ImGui::InputText("Extraargs Before -i", hacks.extraArgs, 60);
        ImGui::InputText("Extraargs After -i", hacks.extraArgsAfter, 60);
        ImGui::InputInt("Click Chunk Size", &hacks.clickSoundChunkSize, 0);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("How many actions does a click chunk file contains? A click chunk file is a part of the whole rendered clicks, i have to split them to bypass the command character limit.\nTry increasing this if the clicks do not render.");
        ImGui::InputFloat("Show End For", &hacks.afterEndDuration);
        ImGui::PopItemWidth();

        Marker("Usage", "Hit record in a level and let a macro play. The rendered video will be in GDmenu/renders/level - levelid. If you're unsure of what a setting does, leave it on default.\n If you're using an NVIDIA GPU i reccomend settings your extra args before -i to: -hwaccel cuda -hwaccel_output_format cuda and the encoder to: h264_nvenc.\n If you're using an AMD GPU i reccomend setting the encoder to either: h264_amf or hevc_amf.");
        Marker("Credits", "All the credits for the recording side goes to matcool's replaybot implementation, i integrated my clickbot into it");

        ImGui::End();

        ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
        ImGui::Begin("Macrobot", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
        ImGui::SetWindowFontScale(screenSize * hacks.menuSize);
        if (resetWindows)
            ImGui::SetWindowPos({10 * screenSize, 260 * screenSize});
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
        if (ReplayPlayer::getInstance().IsRecording())
            ImGui::PushStyleColor(0, ImVec4(0, 1, 0, 1));
        else
            ImGui::PushStyleColor(0, ImVec4(1, 0, 0, 1));
        if (ImGui::Button("Toggle Recording"))
            ReplayPlayer::getInstance().ToggleRecording();
        if (ReplayPlayer::getInstance().IsPlaying())
            ImGui::PushStyleColor(0, ImVec4(0, 1, 0, 1));
        else
            ImGui::PushStyleColor(0, ImVec4(1, 0, 0, 1));
        if (ImGui::Button("Toggle Playing"))
            ReplayPlayer::getInstance().TogglePlaying();
        else
            ImGui::PushStyleColor(0, ImVec4(1, 1, 1, 1));
        ImGui::Checkbox("Show Replay Label", &hacks.botTextEnabled);

        ImGui::Checkbox("Click sounds", &hacks.clickbot);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("clicks", 1))
            ImGui::OpenPopup("Click sounds settings");
        if (ImGui::BeginPopupModal("Click sounds settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputFloat("Click volume", &hacks.baseVolume);
            ImGui::InputFloat("Max pitch variation", &hacks.maxPitch);
            ImGui::InputFloat("Min pitch variation", &hacks.minPitch);
            ImGui::InputDouble("Play Medium Clicks at", &hacks.playMediumClicksAt);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Amount of time between click so that a medium click is played");
            ImGui::InputFloat("Minimum time difference", &hacks.minTimeDifference);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Minimum time difference for a click to play, to avoid tiny double clicks");
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            Marker("?", "Put clicks, releases and mediumclicks in the respective folders found in GDMenu/clicks");
            ImGui::EndPopup();
        }
        ImGui::Checkbox("Prevent inputs", &hacks.preventInput);

        ImGui::Checkbox("Autoclicker", &hacks.autoclicker);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("aut", 1))
            ImGui::OpenPopup("Autoclicker Settings");

        if (ImGui::BeginPopupModal("Autoclicker Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputFloat("Click time", &hacks.clickTime);
            ImGui::InputFloat("Release time", &hacks.releaseTime);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Frame Step", &hacks.frameStep);
        ImGui::SameLine(arrowButtonPosition * screenSize * hacks.menuSize);
        if (ImGui::ArrowButton("fra", 1))
            ImGui::OpenPopup("Frame Step Settings");

        if (ImGui::BeginPopupModal("Frame Step Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputInt("Step Count", &hacks.stepCount, 0);
            ImGui::PushItemWidth(180 * screenSize * hacks.menuSize);
            Hotkey("Step Key", &hacks.stepIndex);
            ImGui::PopItemWidth();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        ImGui::InputInt("Start At Action", &hacks.actionStart, 0);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        if (ImGui::Button("Clear actions"))
            ReplayPlayer::getInstance().ClearActions();

        ImGui::Spacing();
        ImGui::PushItemWidth(100 * screenSize * hacks.menuSize);
        ImGui::InputText("Replay Name", fileName, 30);
        ImGui::PopItemWidth();
        if (ImGui::Button("Save"))
            ReplayPlayer::getInstance().Save(fileName);
        if (ImGui::Button("Load"))
            ReplayPlayer::getInstance().Load(fileName);
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar();

        if (resetWindows || repositionWindows)
            ImGui::SaveIniSettingsToDisk("imgui.ini");
        resetWindows = false;
        repositionWindows = false;
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
    ImGuiHook::setRenderFunction(RenderMain);
    ImGuiHook::setInitFunction(Init);
    ImGuiHook::setToggleCallback([]()
                                 { show = !show; });
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