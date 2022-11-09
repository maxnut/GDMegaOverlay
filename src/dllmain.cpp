#include "pch.h"
#include <imgui-hook.hpp>
#include <imgui.h>
#include "ImgUtil.h"
#include "PlayLayer.h"
#include "MenuLayer.h"
#include "LevelSearchLayer.h"
#include "LevelEditorLayer.h"
#include "Hacks.h"
#include "bools.h"
#include "FpsMult.h"
#include "ReplayPlayer.h"
#include <fstream>
#include "speedhack.h"

bool show = false;
bool applied = false;
extern struct HacksStr hacks;
extern struct Labels labels;

int deafenInt = 0;
char fileName[30];

const char *style[] = {"Number and text", "Number Only"};
const char *items[] = {"Normal", "No Spikes", "No Hitbox", "No Solid", "Force Block", "Everything Hurts"};
const char *trail[] = {"Normal", "Always Off", "Always On", "Inversed"};
const char *fonts[] = {"Big Font", "Chat Font", "Font 01", "Font 02", "Font 03", "Font 04", "Font 05", "Font 06", "Font 07", "Font 08", "Font 09", "Font 10", "Font 11", "Gold Font"};
const char *alphabet[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "Caps", "CTRL", "Alt", "Shift", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};

void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void TextSettings(int index, bool font)
{
    if (ImGui::InputFloat3(("Position and Scale##" + std::to_string(index)).c_str(), labels.poscale[index]))
        PlayLayer::UpdatePositions(index);
    if (ImGui::InputFloat(("Opacity##" + std::to_string(index)).c_str(), &labels.opacity[index]))
        PlayLayer::UpdatePositions(index);
    if (font && ImGui::Combo(("Font##" + std::to_string(index)).c_str(), &labels.fonts[index], fonts, IM_ARRAYSIZE(fonts)))
        PlayLayer::UpdatePositions(index);
}

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

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(hacks.borderColor[0], hacks.borderColor[1], hacks.borderColor[2], hacks.borderColor[3]);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], hacks.titleColor[3]);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], hacks.titleColor[3]);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], hacks.titleColor[3]);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
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
char nameBuf[20];

void Init()
{
    if (!std::filesystem::is_directory("GDMenu") || !std::filesystem::exists("GDMenu"))
    {
        std::filesystem::create_directory("GDMenu");
    }

    ImGuiIO &io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.0f);
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    io.FontDefault = font;

    std::ifstream f;
    f.open("GDMenu/settings.bin", std::fstream::binary);
    if (f)
    {
        f.read((char *)&hacks, sizeof(HacksStr));
        Hacks::FPSBypass(hacks.fps);
        Hacks::noTransition(hacks.notransition);
        Hacks::unlockIcons(hacks.icons);
        Hacks::MainLevels(hacks.mainLevels);
        Hacks::AnticheatBypass();
        Hacks::accuratepercentage(hacks.accuratepercentage);
        Hacks::nopulse(hacks.nopulse);
        Hacks::practicemusic(hacks.practicemusic);
        Hacks::coinsonpractice(hacks.coinsPractice);
        Hacks::disabledeatheffect(hacks.deathEffect);
        Hacks::disablerespawnblink(hacks.respawnBlink);
        Hacks::hitboxType(hacks.hitboxType);
        Hacks::freeResize(hacks.freeResize);
        Hacks::sliderLimit(hacks.sliderLimit);
        Hacks::noclip(hacks.noclip);
        Hacks::jumphack(hacks.jumphack);
        Hacks::IgnoreESC(hacks.ignoreesc);
        Hacks::freezeplayer(hacks.freezeplayer);
        Hacks::noGlow(hacks.noglow);
        Hacks::fastalttab(hacks.fastalttab);
        Hacks::TextLength(hacks.textLength);
        Hacks::FilterBypass(hacks.filterBypass);
        Hacks::safemode(hacks.safemode);
        Hacks::practicepulse(hacks.practicepulse);
        Hacks::noprogressbar(hacks.noprogressbar);
        Hacks::copyhack(hacks.copyhack);
        Hacks::nocmark(hacks.nocmark);
        Hacks::zoomhack(hacks.zoomhack);
        Hacks::verifyhack(hacks.verifyhack);
        Hacks::defaultsong(hacks.defaultsong);
        Hacks::editorextension(hacks.extension);
        Hacks::placeover(hacks.placeover);
        Hacks::hideui(hacks.hideui);
        Hacks::trailType(hacks.trailType);
        Hacks::solidwavetrail(hacks.solidwavetrail);
        Hacks::noflash(hacks.noFlash);
        Hacks::noparticles(hacks.noParticles);
        Hacks::samedualcolor(hacks.sameDualColor);
        Hacks::objectHack(hacks.objectHack);
        Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
        Hacks::WriteRef(gd::base + 0x20A677, hacks.respawnTime);
        Hacks::transparentLists(hacks.transparentList);
        Hacks::transparentMenus(hacks.transparentMenus);
        deafenInt = hacks.curChar;

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
}

void RenderMain()
{

    if (show)
    {

        SetStyle();
        closed = false;

        ImGui::Begin("General Mods", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        ImGui::PushItemWidth(130);
        ImGui::InputFloat("FPS Bypass Value", &hacks.fps);
        ImGui::InputFloat("Speedhack Value", &hacks.speed);
        ImGui::PopItemWidth();
        if (ImGui::Button("Apply##sv"))
        {
            if (hacks.fps <= 1)
                hacks.fps = 60;
            Hacks::FPSBypass(hacks.fps);
            if (hacks.speed <= 0)
                hacks.speed = 1;
            Hacks::Speedhack(hacks.speed);
        }

        ImGui::End();

        ImGui::Begin("Global", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        ImGui::Checkbox("Auto Deafen", &hacks.autoDeafen);
        ImGui::SameLine(250, -10);
        if (ImGui::ArrowButton("autod", 1))
            ImGui::OpenPopup("Auto Deafen Settings");

        if (ImGui::BeginPopupModal("Auto Deafen Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth(130);
            ImGui::InputFloat("Auto Deafen %", &hacks.percentage);

            if (ImGui::Combo("Deafen Char", &deafenInt, alphabet, IM_ARRAYSIZE(alphabet)))
            {
                hacks.mute = *alphabet[deafenInt];
                hacks.curChar = deafenInt;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Set a key combination in discord with leftalt + the key you set here");
            ImGui::PopItemWidth();
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Checkbox("No Transitions", &hacks.notransition))
            Hacks::noTransition(hacks.notransition);
        if (ImGui::Checkbox("Free Resize", &hacks.freeResize))
            Hacks::freeResize(hacks.freeResize);
        if (ImGui::Checkbox("Fast AltTab", &hacks.fastalttab))
            Hacks::fastalttab(hacks.fastalttab);
        if (ImGui::Checkbox("Transparent Lists", &hacks.transparentList))
            Hacks::transparentLists(hacks.transparentList);
        if (ImGui::Checkbox("Transparent Menu", &hacks.transparentMenus))
            Hacks::transparentMenus(hacks.transparentMenus);

        ImGui::End();

        ImGui::Begin("Level", 0, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Checkbox("StartPos Switcher", &hacks.startPosSwitcher);

        ImGui::Checkbox("Show Hitboxes", &hacks.showHitboxes);
        ImGui::SameLine(250, -10);
        if (ImGui::ArrowButton("hit", 1))
            ImGui::OpenPopup("Hitbox Settings");

        if (ImGui::BeginPopupModal("Hitbox Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Checkbox("Only on Death", &hacks.onlyOnDeath);
            ImGui::Checkbox("Show Decorations", &hacks.showDecorations);
            ImGui::Checkbox("Hitboxes only", &hacks.hitboxOnly);
            ImGui::InputInt("Hitbox Opacity", &hacks.borderOpacity);
            ImGui::InputInt("Fill Opacity", &hacks.hitboxOpacity);
            ImGui::InputFloat("Hitbox Thickness", &hacks.hitboxThickness);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Layout Mode", &hacks.layoutMode);
        if (ImGui::Checkbox("No Glow", &hacks.noglow))
            Hacks::noGlow(hacks.noglow);
        if (ImGui::Checkbox("Practice Music", &hacks.practicemusic))
            Hacks::practicemusic(hacks.practicemusic);
        if (ImGui::Checkbox("Coins in Practice", &hacks.coinsPractice))
            Hacks::coinsonpractice(hacks.coinsPractice);
        if (ImGui::Checkbox("Safe Mode", &hacks.safemode))
            Hacks::safemode(hacks.safemode);
        if (ImGui::Checkbox("Practice Pulse", &hacks.practicepulse))
            Hacks::practicepulse(hacks.practicepulse);
        if (ImGui::Checkbox("No Progress Bar", &hacks.noprogressbar))
            Hacks::noprogressbar(hacks.noprogressbar);
        if (ImGui::Checkbox("Ignore ESC", &hacks.ignoreesc))
            Hacks::IgnoreESC(hacks.ignoreesc);
        ImGui::Checkbox("Hide Attempts", &hacks.hideattempts);
        ImGui::Checkbox("Hide Testmode", &hacks.hideTestmode);
        if (ImGui::Checkbox("No Pulse", &hacks.nopulse))
            Hacks::nopulse(hacks.nopulse);
        if (ImGui::Checkbox("No Particles", &hacks.noParticles))
            Hacks::noparticles(hacks.noParticles);
        if (ImGui::Checkbox("Accurate Percentage", &hacks.accuratepercentage))
            Hacks::accuratepercentage(hacks.accuratepercentage);
        if (ImGui::Checkbox("Instant Complete", &hacks.instantcomplete))
            Hacks::InstantComplete(hacks.instantcomplete);
        ImGui::Checkbox("Replay From Last Checkpoint", &hacks.lastCheckpoint);
        ImGui::Checkbox("Practice Fix", &hacks.fixPractice);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Activate this if you want the practice fixes to be active even if macrobot is not recording");

        ImGui::Checkbox("Auto Sync Music", &hacks.autoSyncMusic);
        ImGui::SameLine(250, -10);
        if (ImGui::ArrowButton("ausm", 1))
            ImGui::OpenPopup("Auto Sync Music Settings");

        if (ImGui::BeginPopupModal("Auto Sync Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputInt("Max Desync Amount (ms)", &hacks.musicMaxDesync);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Confirm Quit", &hacks.confirmQuit);

        ImGui::End();

        ImGui::Begin("Bypass", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        if (ImGui::Checkbox("Unlock Icons", &hacks.icons))
            Hacks::unlockIcons(hacks.icons);
        if (ImGui::Checkbox("Slider Limit", &hacks.sliderLimit))
            Hacks::sliderLimit(hacks.sliderLimit);
        if (ImGui::Checkbox("Main Levels", &hacks.mainLevels))
            Hacks::MainLevels(hacks.mainLevels);
        if (ImGui::Checkbox("Text Length", &hacks.textLength))
            Hacks::TextLength(hacks.textLength);
        if (ImGui::Checkbox("Filter Bypass", &hacks.filterBypass))
            Hacks::FilterBypass(hacks.filterBypass);

        ImGui::End();

        ImGui::Begin("Player", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        if (ImGui::Checkbox("Noclip", &hacks.noclip))
            Hacks::noclip(hacks.noclip);
        if (ImGui::Checkbox("Freeze player", &hacks.freezeplayer))
            Hacks::freezeplayer(hacks.freezeplayer);
        if (ImGui::Checkbox("Jump Hack", &hacks.jumphack))
            Hacks::jumphack(hacks.jumphack);
        if (ImGui::Checkbox("No Respawn Blink", &hacks.respawnBlink))
            Hacks::disablerespawnblink(hacks.respawnBlink);
        if (ImGui::Checkbox("No Death Effect", &hacks.deathEffect))
            Hacks::disabledeatheffect(hacks.deathEffect);
        if (ImGui::Checkbox("Solid Wave Trail", &hacks.solidwavetrail))
            Hacks::solidwavetrail(hacks.solidwavetrail);
        if (ImGui::Checkbox("Same Dual Color", &hacks.sameDualColor))
            Hacks::samedualcolor(hacks.sameDualColor);
        ImGui::Checkbox("Rainbow Icons", &hacks.rainbowIcons);
        ImGui::SameLine(250, -10);
        if (ImGui::ArrowButton("rain", 1))
            ImGui::OpenPopup("Rainbow Icons Settings");

        if (ImGui::BeginPopupModal("Rainbow Icons Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushItemWidth(130);
            ImGui::InputFloat("Rainbow Speed Interval", &hacks.rainbowSpeed);
            ImGui::PopItemWidth();
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Dash Orb Color Fix", &hacks.dashOrbFix);
        ImGui::PushItemWidth(130);
        if (ImGui::Combo("Hitbox Type", &hacks.hitboxType, items, IM_ARRAYSIZE(items)))
            Hacks::hitboxType(hacks.hitboxType);
        if (ImGui::Combo("Trail Type", &hacks.trailType, trail, IM_ARRAYSIZE(trail)))
            Hacks::trailType(hacks.trailType);
        if (ImGui::InputFloat("Wave Pulse Size", &hacks.waveSize))
            Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
        if (ImGui::InputFloat("Respawn Time", &hacks.respawnTime))
            Hacks::WriteRef(gd::base + 0x20A677, hacks.respawnTime);
        ImGui::PopItemWidth();

        ImGui::End();

        ImGui::Begin("Creator", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        if (ImGui::Checkbox("Copy Hack", &hacks.copyhack))
            Hacks::copyhack(hacks.copyhack);
        if (ImGui::Checkbox("Level Edit", &hacks.leveledit))
            Hacks::leveledit(hacks.leveledit);
        if (ImGui::Checkbox("No C Mark", &hacks.nocmark))
            Hacks::nocmark(hacks.nocmark);
        if (ImGui::Checkbox("Object Limit", &hacks.objectlimit))
            Hacks::objectlimit(hacks.objectlimit);
        if (ImGui::Checkbox("Zoom Hack", &hacks.zoomhack))
            Hacks::zoomhack(hacks.zoomhack);
        if (ImGui::Checkbox("Verify Hack", &hacks.verifyhack))
            Hacks::verifyhack(hacks.verifyhack);
        if (ImGui::Checkbox("Default Song Bypass", &hacks.defaultsong))
            Hacks::defaultsong(hacks.defaultsong);
        if (ImGui::Checkbox("Editor Extension", &hacks.extension))
            Hacks::editorextension(hacks.extension);
        if (ImGui::Checkbox("Place Over", &hacks.placeover))
            Hacks::placeover(hacks.placeover);
        if (ImGui::Checkbox("Hide UI", &hacks.hideui))
            Hacks::hideui(hacks.hideui);
        if (ImGui::Checkbox("Custom Object Limit", &hacks.objectHack))
            Hacks::objectHack(hacks.objectHack);

        ImGui::End();

        ImGui::Begin("Status", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        ImGui::Checkbox("Cheat Indicator", &labels.statuses[0]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("FPS Counter", &labels.statuses[1]);
        ImGui::SameLine(190, -10);
        if (ImGui::ArrowButton("fps", 1))
            ImGui::OpenPopup("FPS Counter Settings");
        if (ImGui::BeginPopupModal("FPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(1, true);
            ImGui::Combo("Style##fpsc", &labels.styles[0], style, IM_ARRAYSIZE(style));
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("CPS Counter", &labels.statuses[2]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("Noclip accuracy", &labels.statuses[3]);
        ImGui::SameLine(190, -10);
        if (ImGui::ArrowButton("nca", 1))
            ImGui::OpenPopup("Noclip Accuracy Settings");

        if (ImGui::BeginPopupModal("Noclip Accuracy Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(3, true);
            ImGui::InputFloat("Noclip Accuracy limit", &hacks.noClipAccuracyLimit);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Noclip deaths", &labels.statuses[4]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("Clock", &labels.statuses[5]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("Best Run", &labels.statuses[6]);
        ImGui::SameLine(190, -10);
        if (ImGui::ArrowButton("best run", 1))
            ImGui::OpenPopup("Best Run Settings");
        if (ImGui::BeginPopupModal("Best Run Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            TextSettings(6, true);
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Attempts", &labels.statuses[7]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("From %", &labels.statuses[8]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("Message Status", &labels.statuses[9]);
        ImGui::SameLine(190, -10);
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

        ImGui::Checkbox("Current Attempt", &labels.statuses[10]);
        ImGui::SameLine(190, -10);
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

        ImGui::End();

        ImGui::Begin("Menu Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        ImGui::PushItemWidth(130);
        ImGui::InputFloat("Font Size", &hacks.fontSize);
        if (hacks.fontSize > 3)
            hacks.fontSize = 3;
        else if (hacks.fontSize < 0.5f)
            hacks.fontSize = 0.5f;
        ImGui::InputFloat("Border Size", &hacks.borderSize);
        ImGui::InputFloat("Window Rounding", &hacks.windowRounding);
        ImGui::PopItemWidth();

        ImGui::ColorEdit4("Window Title BG Color", hacks.titleColor, ImGuiColorEditFlags_NoInputs);
        ImGui::ColorEdit4("Border Color", hacks.borderColor, ImGuiColorEditFlags_NoInputs);

        ImGui::End();

        ImGui::Begin("Shortcuts", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

        ImGui::Text("Respawn");
        ImGui::SameLine(150, -10);
        ImGui::SetNextItemWidth(50.0f);
        ImGui::Combo("Key##respawn", &hacks.respawnIndex, alphabet, IM_ARRAYSIZE(alphabet));

        ImGui::Text("Toggle Message");
        ImGui::SameLine(150, -10);
        ImGui::SetNextItemWidth(50.0f);
        ImGui::Combo("Key##message", &hacks.messageIndex, alphabet, IM_ARRAYSIZE(alphabet));

        ImGui::Text("Toggle Hitboxes");
        ImGui::SameLine(150, -10);
        ImGui::SetNextItemWidth(50.0f);
        ImGui::Combo("Key##hitboxes", &hacks.hitboxKeyIndex, alphabet, IM_ARRAYSIZE(alphabet));

        if (ImGui::Button("Open GD Settings"))
            gd::OptionsLayer::addToCurrentScene(false);

        ImGui::End();

        ImGui::Begin("Macrobot", 0, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowFontScale(hacks.fontSize);

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
        ImGui::SameLine(250, -10);
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
            HelpMarker("Put clicks, releases and mediumclicks in the respective folders found in GDMenu/clicks");
            ImGui::EndPopup();
        }

        ImGui::Checkbox("Autoclicker", &hacks.autoclicker);
        ImGui::SameLine(250, -10);
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
        ImGui::SameLine(250, -10);
        if (ImGui::ArrowButton("fra", 1))
            ImGui::OpenPopup("Frame Step Settings");

        if (ImGui::BeginPopupModal("Frame Step Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputInt("Step Count", &hacks.stepCount);
            ImGui::Combo("Key", &hacks.stepIndex, alphabet, IM_ARRAYSIZE(alphabet));
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::PushItemWidth(150);
        ImGui::InputInt("Start At Action", &hacks.actionStart);
        ImGui::PopItemWidth();

        ImGui::Spacing();
        if (ImGui::Button("Clear actions"))
            ReplayPlayer::getInstance().ClearActions();

        ImGui::Spacing();
        ImGui::PushItemWidth(150);
        ImGui::InputText("Replay Name", fileName, 30);
        ImGui::PopItemWidth();
        if (ImGui::Button("Save"))
            ReplayPlayer::getInstance().Save(fileName);
        if (ImGui::Button("Load"))
            ReplayPlayer::getInstance().Load(fileName);
        ImGui::PopStyleColor();
        ImGui::End();
    }
    else if (!closed)
    {
        std::ofstream f;
        f.open("GDMenu/settings.bin", std::fstream::binary);
        if (f)
            f.write((char *)&hacks, sizeof(HacksStr));
        f.close();
        f.open("GDMenu/labels.bin", std::fstream::binary);
        if (f)
            f.write((char *)&labels, sizeof(HacksStr));
        closed = true;
        f.close();
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
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1907B0), MenuLayer::hook, reinterpret_cast<void **>(&MenuLayer::init));
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
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x17DA60), LevelSearchLayer::hook, reinterpret_cast<void **>(&LevelSearchLayer::init));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x20DDD0), CustomCheckpoint::createHook, reinterpret_cast<void **>(&CustomCheckpoint::create));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x253D60), PlayLayer::triggerObjectHook, reinterpret_cast<void **>(&PlayLayer::triggerObject));
        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x1FFD80), PlayLayer::lightningFlashHook, reinterpret_cast<void **>(&PlayLayer::lightningFlash));

        MH_CreateHook(reinterpret_cast<void *>(gd::base + 0x16B7C0), LevelEditorLayer::drawHook, reinterpret_cast<void **>(&LevelEditorLayer::draw));
        Setup();
        // Speedhack::Setup();

        MH_CreateHook(addr, PlayLayer::dispatchKeyboardMSGHook, reinterpret_cast<void **>(&PlayLayer::dispatchKeyboardMSG));

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