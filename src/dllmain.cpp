#include "pch.h"
#include <imgui-hook.hpp>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "API.h"
#include "CCSchedulerHook.h"
#include "ConstData.h"
#include "CustomAction.h"
#include "EndLevelLayer.h"
#include "ExternData.h"
#include "Hacks.h"
#include "ImgUtil.h"
#include "LevelEditorLayer.h"
#include "LevelSearchLayer.h"
#include "MenuLayer.h"
#include "PlayLayer.h"
#include "ReplayPlayer.h"
#include "Shortcuts.h"
#include "explorer.hpp"
#include "imgui_internal.h"
#include "json.hpp"
#include "portable-file-dialogs.h"
#include <fstream>
#include <shellapi.h>

using json = nlohmann::json;

bool loaded = false;
bool isDecember = false;
extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

float pitch;

int shortcutIndex, pitchName, editing, tabIndex = 0, variableIndex = 0;
char url[100], id[30], macroName[100];
std::vector<ReplayInfo> replays;
std::vector<const char*> musicPathsVet;

std::string hoveredHack = "";

bool opening = false;

char* convert(const std::string& s)
{
	char* pc = new char[s.size() + 1];
	std::strcpy(pc, s.c_str());
	return pc;
}

void GetMacros()
{
	auto dirIter = std::filesystem::directory_iterator("GDMenu/macros");
	static int fileCount = 0, fileCount2 = 0;
	int fileCountNow = std::count_if(begin(dirIter), end(dirIter), [](auto& entry) { return entry.is_regular_file(); });

	if (fileCount != fileCountNow)
	{
		fileCount = fileCountNow;
		replays.clear();
		for (std::filesystem::directory_entry loop : std::filesystem::directory_iterator{"GDMenu/macros"})
		{
			if (loop.path().extension().string() == ".replay" || loop.path().extension().string() == ".macro")
			{
				replays.push_back(Replay::GetInfo(loop.path()));
			}
		}
	}
}

void InitJSONHacks(json& js)
{
	for (size_t i = 0; i < js["mods"].size(); i++)
	{
		Hacks::ToggleJSONHack(js, i, false);
	}
}

void DrawFromJSON(json& js)
{
	for (size_t i = 0; i < js["mods"].size(); i++)
	{
		if (GDMO::ImCheckbox(js["mods"][i]["name"].get<std::string>().c_str(), js["mods"][i]["toggle"].get<bool*>()))
		{
			Hacks::ToggleJSONHack(js, i, false);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(js["mods"][i]["description"].get<std::string>().c_str());
	}
}

void TextSettings(int index, bool font)
{
	if (GDMO::ImCombo(("Position##" + std::to_string(index)).c_str(), (int*)&labels.positions[index], positions,
					  IM_ARRAYSIZE(positions)))
		for (size_t i = 0; i < STATUSSIZE; i++)
			PlayLayer::UpdatePositions(i);
	if (GDMO::ImInputFloat(("Scale##" + std::to_string(index)).c_str(), &labels.scale[index]))
		for (size_t i = 0; i < STATUSSIZE; i++)
			PlayLayer::UpdatePositions(i);
	if (GDMO::ImInputFloat(("Opacity##" + std::to_string(index)).c_str(), &labels.opacity[index]))
		for (size_t i = 0; i < STATUSSIZE; i++)
			PlayLayer::UpdatePositions(i);
	if (font &&
		GDMO::ImCombo(("Font##" + std::to_string(index)).c_str(), &labels.fonts[index], fonts, IM_ARRAYSIZE(fonts)))
		for (size_t i = 0; i < STATUSSIZE; i++)
			PlayLayer::UpdatePositions(i);
}

uint32_t GetPointerAddress(std::vector<uint32_t> offsets)
{
	if (offsets.size() > 1)
	{
		uint32_t buf = Hacks::Read<uint32_t>(offsets[0] + gd::base);

		for (size_t i = 1; i < offsets.size() - 1 /*ignore last offset*/; ++i)
			buf = Hacks::Read<uint32_t>(buf + offsets[i]);
		return buf + offsets.back();
	}
	return offsets.size() ? offsets[0] + gd::base : 0;
}

void SetStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();

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

	style->ScaleAllSizes(ExternData::screenSizeX * hacks.menuSize);

	float r, g, b;
	ImGui::ColorConvertHSVtoRGB(ImGui::GetTime() * hacks.menuRainbowSpeed, hacks.menuRainbowBrightness,
								hacks.menuRainbowBrightness, r, g, b);

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 0.03f, 0.03f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] =
		ImVec4(hacks.windowBgColor[0], hacks.windowBgColor[1], hacks.windowBgColor[2], 1);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);

	if (!hacks.rainbowMenu)
	{
		style->Colors[ImGuiCol_Border] = ImVec4(hacks.borderColor[0], hacks.borderColor[1], hacks.borderColor[2], 1);
		style->Colors[ImGuiCol_TitleBg] = ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], 1);
		style->Colors[ImGuiCol_TitleBgCollapsed] =
			ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], 1);
		style->Colors[ImGuiCol_TitleBgActive] =
			ImVec4(hacks.titleColor[0], hacks.titleColor[1], hacks.titleColor[2], 1);

		style->Colors[ImGuiCol_Tab] =
			ImVec4(hacks.titleColor[0] * 0.85f, hacks.titleColor[1] * 0.85f, hacks.titleColor[2] * 0.85f, 1);
		style->Colors[ImGuiCol_TabActive] =
			ImVec4(hacks.titleColor[0] * 0.85f, hacks.titleColor[1] * 0.85f, hacks.titleColor[2] * 0.85f, 1);
		style->Colors[ImGuiCol_TabHovered] =
			ImVec4(hacks.titleColor[0] * 0.70f, hacks.titleColor[1] * 0.70f, hacks.titleColor[2] * 0.70f, 1);
		style->Colors[ImGuiCol_TabUnfocused] =
			ImVec4(hacks.titleColor[0] * 0.60f, hacks.titleColor[1] * 0.60f, hacks.titleColor[2] * 0.60f, 1);
		style->Colors[ImGuiCol_TabUnfocusedActive] =
			ImVec4(hacks.titleColor[0] * 0.60f, hacks.titleColor[1] * 0.60f, hacks.titleColor[2] * 0.60f, 1);
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
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.15f, 0.18f, 0.3f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.15f, 0.18f, 0.4f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.14f, 0.15f, 0.18f, 0.5f);

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

void Init()
{
	srand(time(NULL));

	ImGuiIO& io = ImGui::GetIO();
	auto font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.0f);
	io.Fonts->Build();
	ImGui_ImplOpenGL3_CreateFontsTexture();
	io.FontDefault = font;

	io.IniFilename = NULL;

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
	if (!std::filesystem::is_directory("GDMenu/macros") || !std::filesystem::exists("GDMenu/macros"))
	{
		std::filesystem::create_directory("GDMenu/macros");
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
	if (!std::filesystem::is_directory("GDMenu/clicks/mediumclicks") ||
		!std::filesystem::exists("GDMenu/clicks/mediumclicks"))
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
			ExternData::musicPaths.push_back(loop.path().string());
		}
	}

	for (size_t i = 0; i < ExternData::musicPaths.size(); i++)
	{
		musicPathsVet.push_back(ExternData::musicPaths[i].c_str());
	}

	Hacks::AnticheatBypass();

	std::ifstream mods;
	std::stringstream buffer;

	mods.open("GDMenu/mod/variables.json");
	buffer << mods.rdbuf();
	ExternData::variables = json::parse(buffer.str());
	mods.close();
	buffer.str("");
	buffer.clear();

	mods.open("GDMenu/mod/bypass.json");
	buffer << mods.rdbuf();
	ExternData::bypass = json::parse(buffer.str());
	hackAmts[0] = ExternData::bypass["mods"].size();
	mods.close();
	buffer.str("");
	buffer.clear();
	InitJSONHacks(ExternData::bypass);

	mods.open("GDMenu/mod/creator.json");
	buffer << mods.rdbuf();
	ExternData::creator = json::parse(buffer.str());
	hackAmts[1] = ExternData::creator["mods"].size();
	mods.close();
	buffer.str("");
	buffer.clear();
	InitJSONHacks(ExternData::creator);

	mods.open("GDMenu/mod/global.json");
	buffer << mods.rdbuf();
	ExternData::global = json::parse(buffer.str());
	hackAmts[2] = ExternData::global["mods"].size();
	mods.close();
	buffer.str("");
	buffer.clear();
	InitJSONHacks(ExternData::global);

	mods.open("GDMenu/mod/level.json");
	buffer << mods.rdbuf();
	ExternData::level = json::parse(buffer.str());
	hackAmts[3] = ExternData::level["mods"].size();
	mods.close();
	buffer.str("");
	buffer.clear();
	InitJSONHacks(ExternData::level);

	mods.open("GDMenu/mod/player.json");
	buffer << mods.rdbuf();
	ExternData::player = json::parse(buffer.str());
	hackAmts[4] = ExternData::player["mods"].size();
	mods.close();
	buffer.str("");
	buffer.clear();
	InitJSONHacks(ExternData::player);

	auto extpath = "GDMenu/dll/extensions.json";
	if (!std::filesystem::exists(extpath))
	{
		std::ofstream file(extpath);
		file.close();
	}
	mods.open(extpath);
	buffer << mods.rdbuf();
	if (buffer.rdbuf()->in_avail() > 0)
		ExternData::dlls = json::parse(buffer.str());
	mods.close();
	buffer.str("");
	buffer.clear();

	auto winpath = "GDMenu/windows.json";
	if (!std::filesystem::exists(winpath))
	{
		std::ofstream file(winpath);
		file.close();
	}

	mods.open(winpath);
	buffer << mods.rdbuf();
	if (buffer.rdbuf()->in_avail() > 0)
		ExternData::windowPositions = json::parse(buffer.str());
	mods.close();
	buffer.str("");

	std::ifstream f;
	f.open("GDMenu/settings.bin", std::fstream::binary);
	if (f)
	{
		f.read((char*)&hacks, sizeof(HacksStr));
		ExternData::level["mods"][24]["toggle"] = false;

		Hacks::FPSBypass(hacks.fps);
		Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
		Hacks::WriteRef(gd::base + 0x20A677, hacks.respawnTime);
		hacks.recording = false;

		Hacks::Priority(hacks.priority);
		ExternData::tps = hacks.tpsBypass;
		ExternData::screenFps = hacks.screenFPS;
	}
	else
	{
		hacks.menuAnimationLength = 0.25f;
		Hacks::FPSBypass(60);
	}

	if (hacks.dockSpace)
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	else
		io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;

	f.close();

	f.open("GDMenu/labels.bin", std::fstream::binary);
	if (f)
	{
		f.read((char*)&labels, sizeof(Labels));
	}

	f.close();

	Shortcuts::Load();

	ExternData::ds.InitDiscord();

	auto path = CCFileUtils::sharedFileUtils()->getWritablePath2() + "GDMenu/dll";

	for (const auto& file : std::filesystem::directory_iterator(path))
	{
		if (file.path().filename().string().find("Sai") != std::string::npos)
		{
			gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
									 ("Sai Mod Pack is not compatible with this menu and will not be loaded"))
				->show();
			continue;
		}
		if (file.path().extension() == ".dll")
		{
			auto dllname = file.path().filename().string();
			ExternData::dllNames.push_back(dllname);
			if (!ExternData::dlls.contains(dllname))
			{
				ExternData::dlls[dllname] = true;
			}
			else if (ExternData::dlls[dllname].get<bool>() == false)
			{
				continue;
			}
			LoadLibrary(file.path().string().c_str());
		}
	}

	path = CCFileUtils::sharedFileUtils()->getWritablePath2();

	for (const auto& file : std::filesystem::directory_iterator(path))
	{
		if (file.path().extension() == ".dll" && file.path().filename().string().find("Sai") != std::string::npos)
		{
			ExternData::hasSaiModPack = true;
			break;
		}
	}

	ExternData::screenSizeX = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize().width / 1920.0f;

	loaded = true;
}

void Hacks::RenderMain()
{
	if (ExternData::ds.core)
		ExternData::ds.core->RunCallbacks();

	const int windowWidth = 220;
	const int arrowButtonPosition = windowWidth - 39;

	if (hacks.hitboxMultiplier <= 0)
		hacks.hitboxMultiplier = 1;

	const float size = ExternData::screenSizeX * hacks.menuSize;
	const float windowSize = windowWidth * size;

	const auto playLayer = gd::GameManager::sharedState()->getPlayLayer();

	if (!playLayer)
	{
		hacks.recording = false;
		if (ReplayPlayer::getInstance().recorder.m_renderer.m_texture &&
			ReplayPlayer::getInstance().recorder.m_recording)
			ReplayPlayer::getInstance().recorder.stop();
	}

	if (ExternData::show || debug.enabled)
		SetStyle();

	if (debug.enabled)
	{
		ImGui::PushStyleColor(0, {1, 1, 1, 1});
		// ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
		ImGui::Begin("Debug");

		ImGui::SetWindowFontScale(ExternData::screenSizeX * hacks.menuSize);

		GDMO::ImInputFloat("N", &debug.debugNumber);
		ImGui::Text(debug.debugString.c_str());

		if (GDMO::ImButton("Copy levelstring"))
		{
			if (playLayer)
				ImGui::SetClipboardText(playLayer->m_level->levelString.c_str());
		}

		ImGui::End();

		ImGui::Begin("CocosExplorer by Mat", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::SetWindowFontScale(ExternData::screenSizeX * hacks.menuSize);

		CocosExplorer::draw();

		ImGui::End();
		ImGui::PopStyleColor();
	}

	if (ExternData::show)
	{
		float s = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize().width / 1920.0f;
		if(s != ExternData::screenSizeX) ExternData::resetWindows = true;
		ExternData::screenSizeX = s;
		s = CCDirector::sharedDirector()->getOpenGLView()->getFrameSize().height / 1080.0f;
		if(s != ExternData::screenSizeY) ExternData::resetWindows = true;
		ExternData::screenSizeY = s;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, hacks.windowRounding);

		for (auto func : ExternData::imguiFuncs)
		{
			func();
		}

		GDMO::ImBegin("Menu Settings");

		ImGui::PushItemWidth(70 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImInputFloat("Menu UI Size", &hacks.menuSize);
		if (hacks.menuSize > 3)
			hacks.menuSize = 1;
		else if (hacks.menuSize < 0.5f)
			hacks.menuSize = 0.5f;
		GDMO::ImInputFloat("Border Size", &hacks.borderSize);
		GDMO::ImInputFloat("Window Rounding", &hacks.windowRounding);
		GDMO::ImInputInt("Window Snap", &hacks.windowSnap, 0);
		ImGui::PopItemWidth();

		if (ImGui::BeginMenu("Menu Colors"))
		{
			GDMO::ImColorEdit3("Title BG Color", hacks.titleColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Border Color", hacks.borderColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("BG Color", hacks.windowBgColor, ImGuiColorEditFlags_NoInputs);
			if (GDMO::ImButton("Reset"))
			{
				hacks.windowBgColor[0] = 0.06f;
				hacks.windowBgColor[1] = 0.05f;
				hacks.windowBgColor[2] = 0.07f;
			}
			ImGui::EndMenu();
		}

		ImGui::PushItemWidth(180 * ExternData::screenSizeX * hacks.menuSize);
		if (GDMO::ImHotkey("Toggle Menu", &hacks.menuKey))
		{
			ImGuiHook::setKeybind(hacks.menuKey);
		}
		ImGui::PopItemWidth();

		// GDMO::ImCheckbox("Experimental Features", &hacks.experimentalFeatures);

		if (isDecember)
			GDMO::ImCheckbox("Snow", &hacks.snow);

		GDMO::ImCheckbox("Rainbow Menu", &hacks.rainbowMenu);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::BeginMenu("##rain", true, 5))
		{
			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImInputFloat("Rainbow Speed", &hacks.menuRainbowSpeed);
			GDMO::ImInputFloat("Rainbow Brightness", &hacks.menuRainbowBrightness);
			ImGui::PopItemWidth();
			ImGui::EndMenu();
		}

		if (GDMO::ImCheckbox("Docking", &hacks.dockSpace))
		{
			ImGuiIO& io = ImGui::GetIO();
			if (hacks.dockSpace)
				io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			else
				io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
		}

		if (GDMO::ImButton("Load Windows"))
			ExternData::resetWindows = true;

		ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImInputFloat("Open Speed", &hacks.menuAnimationLength);
		GDMO::ImInputText("Search", ExternData::searchbar, 30);
		ImGui::PopItemWidth();

		ImGui::End();

		GDMO::ImBegin("General Mods");

		ImGui::PushItemWidth(70 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImInputFloat("FPS Bypass", &hacks.fps);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Changes Max FPS. Disable VSync both in gd and your gpu drivers for it to work.");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (hacks.fps <= 1)
				hacks.fps = 60;
			Hacks::FPSBypass(hacks.fps);
		}

		GDMO::ImInputFloat("##TPSBypass", &hacks.tpsBypass);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Changes how many times the physics gets updated every second.");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			ExternData::tps = hacks.tpsBypass;
		}
		ImGui::SameLine();
		GDMO::ImCheckbox("TPS Bypass", &hacks.tpsBypassBool);
		GDMO::ImInputFloat("##Draw Divide", &hacks.screenFPS);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Changes how many frames of the game will actually be rendered, otherwise they will be "
							  "only processed.");
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			ExternData::screenFps = hacks.screenFPS;
		}
		ImGui::SameLine();
		GDMO::ImCheckbox("Draw Divide", &hacks.drawDivideBool);

		GDMO::ImInputFloat("##Speed hack", &hacks.speed);
		if (ImGui::IsItemDeactivatedAfterEdit() && hacks.speedhackBool)
		{
			if (hacks.speed <= 0)
				hacks.speed = 1;
			Hacks::Speedhack(hacks.speed);

			if (hacks.tieMusicToSpeed)
				SpeedhackAudio::set(hacks.speed);

			if (hacks.autoUpdateRespawn)
				Hacks::WriteRef(gd::base + 0x20A677,
								hacks.respawnTime * CCDirector::sharedDirector()->getScheduler()->getTimeScale());
		}
		ImGui::SameLine();
		if (GDMO::ImCheckbox("Speedhack", &hacks.speedhackBool))
		{
			Hacks::Speedhack(hacks.speedhackBool ? hacks.speed : 1.0f);
			if (hacks.tieMusicToSpeed)
				SpeedhackAudio::set(hacks.speedhackBool ? hacks.speed : 1.0f);

			if (hacks.autoUpdateRespawn)
				Hacks::WriteRef(gd::base + 0x20A677,
								hacks.respawnTime * CCDirector::sharedDirector()->getScheduler()->getTimeScale());
		}
		if (hacks.tieMusicToSpeed)
		{
			ImGui::BeginDisabled();
			GDMO::ImInputFloat("Music Speed", &hacks.musicSpeed);
			ImGui::EndDisabled();
		}
		else
			GDMO::ImInputFloat("Music Speed", &hacks.musicSpeed);

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			SpeedhackAudio::set(hacks.musicSpeed);
		}

		GDMO::ImCombo("Thread Priority", &hacks.priority, priorities, 5);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			Hacks::Priority(hacks.priority);
		}

		if (GDMO::ImCheckbox("Tie Music to Gamespeed", &hacks.tieMusicToSpeed))
		{
			SpeedhackAudio::set(hacks.tieMusicToSpeed ? hacks.speed : hacks.musicSpeed);
		}

		GDMO::ImCheckbox("Tie DeathTime to Gamespeed", &hacks.autoUpdateRespawn);

		ImGui::PopItemWidth();

		ImGui::End();

		GDMO::ImBegin("Global");

		GDMO::ImCheckbox("Auto Deafen", &hacks.autoDeafen);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("autod", 1))
			ImGui::OpenPopup("Auto Deafen Settings");

		if (ImGui::BeginPopupModal("Auto Deafen Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImInputFloat("Auto Deafen %", &hacks.percentage);

			ImGui::PushItemWidth(180 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImHotkey("Mute Key", &hacks.muteKey);
			ImGui::PopItemWidth();

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Set a key combination in discord with leftalt + the key you set here");
			ImGui::PopItemWidth();
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Discord RPC", &hacks.discordRPC))
		{
			if (!hacks.discordRPC)
			{
				if (ExternData::ds.core)
					ExternData::ds.core->ActivityManager().ClearActivity([](discord::Result result) {});
			}
		}

		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("rpc", 1))
			ImGui::OpenPopup("RPC Settings");

		if (ImGui::BeginPopupModal("RPC Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImCombo("##editing", &editing, rpc, 3);

			switch (editing)
			{
			case 0:
				ImGui::Text("{name}, {author}, {best}, {stars}, {id}, {run}");

				GDMO::ImInputText("Details", hacks.levelPlayDetail, 60);
				GDMO::ImInputText("State", hacks.levelPlayState, 60);
				break;
			case 1:
				ImGui::Text("{name}, {objects}");

				GDMO::ImInputText("Details", hacks.editorDetail, 60);
				GDMO::ImInputText("State", hacks.editorState, 60);
				break;

			case 2:

				GDMO::ImInputText("Details", hacks.menuDetail, 60);
				GDMO::ImInputText("State", hacks.menuState, 60);
				break;
			}

			ImGui::PopItemWidth();
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		GDMO::ImCheckbox("Hide Pause Menu", &hacks.hidePause);

		GDMO::ImCheckbox("Custom Menu Music", &hacks.replaceMenuMusic);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("custmm", 1))
			ImGui::OpenPopup("Custom Menu Music Settings");

		if (ImGui::BeginPopupModal("Custom Menu Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			if (GDMO::ImButton("Select Song"))
			{
				auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath(),
												{"Audio File", "*.mp3"}, pfd::opt::none)
									 .result();
				for (auto const& filename : selection)
				{
					std::filesystem::path p = filename;
					memset(hacks.menuSongId, 0, sizeof(hacks.menuSongId));
					p.stem().string().copy(hacks.menuSongId, 10);
				}
				Hacks::MenuMusic();
			}
			ImGui::PopItemWidth();
			GDMO::ImCheckbox("Random Menu Music", &hacks.randomMusic);
			if (ExternData::path.empty())
				ExternData::path =
					ExternData::musicPaths[hacks.randomMusic ? hacks.randomMusicIndex : hacks.musicIndex];

			std::string diobono = hacks.menuSongId;
			if (hacks.randomMusic)
				ImGui::Text(("Playing: " + ExternData::path.filename().string()).c_str());
			else
				ImGui::Text(("Playing: " + diobono).c_str());
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		DrawFromJSON(ExternData::global);

		ImGui::End();

		GDMO::ImBegin("Level");

		GDMO::ImCheckbox("StartPos Switcher", &hacks.startPosSwitcher);
		GDMO::ImCheckbox("Smart StartPos", &hacks.smartStartPos);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("smar", 1))
			ImGui::OpenPopup("Smart StartPos Settings");

		if (ImGui::BeginPopupModal("Smart StartPos Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			GDMO::ImCheckbox("Enable Gravity Detection", &hacks.gravityDetection);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(
					"Enable auto detection of gravity, might not work since it does not account for blue pads or orbs");
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		GDMO::ImCheckbox("Show Hitboxes", &hacks.showHitboxes);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("hit", 1))
			ImGui::OpenPopup("Hitbox Settings");

		if (ImGui::BeginPopupModal("Hitbox Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			GDMO::ImCheckbox("Only on Death", &hacks.onlyOnDeath);
			GDMO::ImCheckbox("Show Decorations", &hacks.showDecorations);
			GDMO::ImCheckbox("Coin Tracker", &hacks.coinTracker);
			GDMO::ImCheckbox("Hitboxes only", &hacks.hitboxOnly);
			GDMO::ImCheckbox("Hitbox trail", &hacks.hitboxTrail);
			GDMO::ImInputFloat("Trail Length", &hacks.hitboxTrailLength);
			GDMO::ImInputInt("Hitbox Opacity", &hacks.borderOpacity, 0);
			GDMO::ImInputInt("Fill Opacity", &hacks.hitboxOpacity, 0);
			GDMO::ImInputFloat("Hitbox Thickness", &hacks.hitboxThickness);
			GDMO::ImColorEdit3("Solid Color", hacks.solidHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Slope Color", hacks.slopeHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Hazard Color", hacks.hazardHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Portal Color", hacks.portalHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Pad Color", hacks.padHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Ring Color", hacks.ringHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Collectible Color", hacks.collectibleHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Modifier Color", hacks.modifierHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Player Color", hacks.playerHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Rotated Player Color", hacks.rotatedHitboxColor, ImGuiColorEditFlags_NoInputs);
			GDMO::ImColorEdit3("Center Player Color", hacks.centerHitboxColor, ImGuiColorEditFlags_NoInputs);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}
		GDMO::ImCheckbox("Layout Mode", &hacks.layoutMode);
		GDMO::ImCheckbox("Hide Attempts", &hacks.hideattempts);

		DrawFromJSON(ExternData::level);

		GDMO::ImCheckbox("Auto Safe Mode", &hacks.autoSafeMode);
		GDMO::ImCheckbox("Practice Fix", &hacks.fixPractice);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(
				"Activate this if you want the practice fixes to be active even if macrobot is not recording");

		GDMO::ImCheckbox("Auto Sync Music", &hacks.autoSyncMusic);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("ausm", 1))
			ImGui::OpenPopup("Auto Sync Music Settings");

		if (ImGui::BeginPopupModal("Auto Sync Music Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			GDMO::ImInputInt("Max Desync Amount (ms)", &hacks.musicMaxDesync, 0);
			if (GDMO::ImButton("Sync Now"))
				PlayLayer::SyncMusic();
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		GDMO::ImCheckbox("Confirm Quit", &hacks.confirmQuit);
		GDMO::ImCheckbox("Show Endscreen Info", &hacks.showExtraInfo);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("sei", 1))
			ImGui::OpenPopup("Endscreen Settings");

		if (ImGui::BeginPopupModal("Endscreen Settings", NULL))
		{
			GDMO::ImCheckbox("Safe Mode", &hacks.safeModeEndscreen);
			GDMO::ImCheckbox("Practice Button", &hacks.practiceButtonEndscreen);
			GDMO::ImCheckbox("Cheat Indicator##chea", &hacks.cheatIndicatorEndscreen);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		ImGui::PushItemWidth(50 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImCheckbox("Hitbox Multiplier", &hacks.enableHitboxMultiplier);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("hbm", 1))
			ImGui::OpenPopup("Hitbox Multiplier Settings");

		if (ImGui::BeginPopupModal("Hitbox Multiplier Settings", NULL))
		{
			GDMO::ImInputFloat("Harards", &hacks.hitboxMultiplier);
			GDMO::ImInputFloat("Solids", &hacks.hitboxSolids);
			GDMO::ImInputFloat("Special", &hacks.hitboxSpecial);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		ImGui::End();

		GDMO::ImBegin("Bypass");

		DrawFromJSON(ExternData::bypass);

		ImGui::End();

		GDMO::ImBegin("Player");

		DrawFromJSON(ExternData::player);

		GDMO::ImCheckbox("Void Click Fix", &hacks.voidClick);
		GDMO::ImCheckbox("Lock Cursor", &hacks.lockCursor);
		GDMO::ImCheckbox("2P One Key", &hacks.twoPlayerOneKey);
		GDMO::ImCheckbox("Show Trajectory", &hacks.trajectory);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Feature not complete, does not work with portals or rings.");

		GDMO::ImCheckbox("No Wave Pulse", &hacks.solidWavePulse);

		GDMO::ImCheckbox("Rainbow Icons", &hacks.rainbowIcons);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("rain", 1))
			ImGui::OpenPopup("Rainbow Icons Settings");

		if (ImGui::BeginPopupModal("Rainbow Icons Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			GDMO::ImCheckbox("Rainbow Color 1", &hacks.rainbowPlayerC1);
			GDMO::ImCheckbox("Rainbow Color 2", &hacks.rainbowPlayerC2);
			GDMO::ImCheckbox("Rainbow Wave Trail", &hacks.rainbowWaveTrail);
			GDMO::ImCheckbox("Rainbow Glow", &hacks.rainbowOutline);

			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImInputFloat("Rainbow Speed Interval", &hacks.rainbowSpeed);
			if (GDMO::ImInputFloat("Rainbow Pastel Amount", &hacks.pastel))
			{
				hacks.pastel = hacks.pastel <= 0.1f ? 0.1f : hacks.pastel > 1.0f ? 1.0f : hacks.pastel;
			}
			ImGui::PopItemWidth();
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		GDMO::ImColorEdit3("##WaveTrailColor", hacks.waveTrailColor, ImGuiColorEditFlags_NoInputs);
		ImGui::SameLine();
		GDMO::ImCheckbox("Wave Trail Color", &hacks.enableWaveTrailColor);

		ImGui::PushItemWidth(90 * ExternData::screenSizeX * hacks.menuSize);
		if (GDMO::ImInputFloat("Wave Trail Size", &hacks.waveSize))
			Hacks::Write<float>(gd::base + 0x2E63A0, hacks.waveSize);
		if (GDMO::ImInputFloat("Respawn Time", &hacks.respawnTime))
			Hacks::WriteRef(gd::base + 0x20A677,
							hacks.respawnTime * (hacks.autoUpdateRespawn
													 ? CCDirector::sharedDirector()->getScheduler()->getTimeScale()
													 : 1));
		ImGui::PopItemWidth();

		ImGui::End();

		GDMO::ImBegin("Creator");

		DrawFromJSON(ExternData::creator);

		ImGui::End();

		GDMO::ImBegin("Status");
		GDMO::ImCheckbox("Hide All", &labels.hideLabels);

		GDMO::ImCheckbox("Rainbow Labels", &labels.rainbowLabels);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::BeginMenu("##rainl", true, 5))
		{
			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImInputFloat("Rainbow Speed##lab", &labels.rainbowSpeed);
			ImGui::PopItemWidth();
			ImGui::EndMenu();
		}

		if (GDMO::ImCheckbox("Cheat Indicator", &labels.statuses[0]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("ci", 1))
			ImGui::OpenPopup("Cheat Indicator Settings");
		if (ImGui::BeginPopupModal("Cheat Indicator Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			TextSettings(0, false);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("FPS Counter", &labels.statuses[1]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("fps", 1))
			ImGui::OpenPopup("FPS Counter Settings");
		if (ImGui::BeginPopupModal("FPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(1, true);
			GDMO::ImInputText("Style##fpsc", labels.styles[0], 15);
			ImGui::SameLine();
			if (GDMO::ImButton("Reset##fpsc"))
			{
				strcpy(labels.styles[0], "%.0f/%.0f");
			}
			GDMO::ImInputFloat("Update Interval", &labels.fpsUpdate);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("CPS Counter", &labels.statuses[2]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("cps", 1))
			ImGui::OpenPopup("CPS Counter Settings");
		if (ImGui::BeginPopupModal("CPS Counter Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(2, true);
			GDMO::ImInputText("Style##cpsc", labels.styles[1], 15);
			ImGui::SameLine();
			if (GDMO::ImButton("Reset##cpsc"))
			{
				strcpy(labels.styles[1], "%i/%i");
			}
			GDMO::ImColorEdit3("Clicked Color", hacks.clickColor, ImGuiColorEditFlags_NoInputs);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Noclip accuracy", &labels.statuses[3]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("nca", 1))
			ImGui::OpenPopup("Noclip Accuracy Settings");

		if (ImGui::BeginPopupModal("Noclip Accuracy Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			TextSettings(3, true);
			GDMO::ImInputText("Style##noclipacc", labels.styles[2], 15);
			ImGui::SameLine();
			if (GDMO::ImButton("Reset##noclipacc"))
			{
				strcpy(labels.styles[2], "Accuracy: %.2f");
			}
			GDMO::ImInputFloat("Noclip Accuracy limit", &hacks.noClipAccuracyLimit);
			GDMO::ImCheckbox("Play Sound on death", &hacks.accuracySound);
			GDMO::ImCheckbox("Enable Screen Effect", &hacks.noclipRed);
			ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImInputFloat("Opacity Limit", &hacks.noclipRedLimit);
			GDMO::ImInputFloat("Opacity Rate Up", &hacks.noclipRedRate);
			GDMO::ImInputFloat("Opacity Rate Down", &hacks.noclipRedRateDown);
			GDMO::ImColorEdit3("Overlay Color", hacks.noclipColor, ImGuiColorEditFlags_NoInputs);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Noclip deaths", &labels.statuses[4]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("ncd", 1))
			ImGui::OpenPopup("Noclip Deaths Settings");

		if (ImGui::BeginPopupModal("Noclip Deaths Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			GDMO::ImInputText("Style##noclipdeaths", labels.styles[3], 15);
			ImGui::SameLine();
			if (GDMO::ImButton("Reset##noclipdeaths"))
			{
				strcpy(labels.styles[3], "Deaths: %i");
			}
			TextSettings(4, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Clock", &labels.statuses[5]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("clock", 1))
			ImGui::OpenPopup("Clock Settings");
		if (ImGui::BeginPopupModal("Clock Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(5, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Best Run", &labels.statuses[6]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("best run", 1))
			ImGui::OpenPopup("Best Run Settings");
		if (ImGui::BeginPopupModal("Best Run Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(6, true);
			GDMO::ImCheckbox("Accumulate Runs", &hacks.accumulateRuns);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Attempts", &labels.statuses[7]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("attempts", 1))
			ImGui::OpenPopup("Attempts Settings");
		if (ImGui::BeginPopupModal("Attempts Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(7, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("From %", &labels.statuses[8]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("from", 1))
			ImGui::OpenPopup("From % Settings");
		if (ImGui::BeginPopupModal("From % Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(8, true);
			GDMO::ImCheckbox("Only in Runs", &hacks.onlyInRuns);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Message Status", &labels.statuses[9]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("stat", 1))
			ImGui::OpenPopup("Message Status Settings");
		if (ImGui::BeginPopupModal("Message Status Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			TextSettings(9, true);
			GDMO::ImInputText("Message", hacks.message, 30);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Current Attempt", &labels.statuses[10]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("curr", 1))
			ImGui::OpenPopup("Current Attempt Settings");
		if (ImGui::BeginPopupModal("Current Attempt Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			TextSettings(10, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Level ID", &labels.statuses[11]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("levelid", 1))
			ImGui::OpenPopup("Level ID Settings");
		if (ImGui::BeginPopupModal("Level ID Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(11, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Jumps", &labels.statuses[12]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("jumps", 1))
			ImGui::OpenPopup("Jumps Settings");
		if (ImGui::BeginPopupModal("Jumps Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(12, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (GDMO::ImCheckbox("Frame", &labels.statuses[13]))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("frame", 1))
			ImGui::OpenPopup("Frame Settings");
		if (ImGui::BeginPopupModal("Frame Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			TextSettings(13, true);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
		if (GDMO::ImInputFloat("Label Spacing", &labels.labelSpacing))
			for (size_t i = 0; i < STATUSSIZE; i++)
				PlayLayer::UpdatePositions(i);
		ImGui::PopItemWidth();

		ImGui::End();

		GDMO::ImBegin("Shortcuts");

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
			ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
			if (GDMO::ImButton(("x##" + std::to_string(i)).c_str()))
			{
				Shortcuts::shortcuts.erase(Shortcuts::shortcuts.begin() + i);
				Shortcuts::Save();
			}

			ImGui::Separator();
		}

		if (GDMO::ImButton("Open GD Settings"))
			gd::OptionsLayer::addToCurrentScene(false);
		if (GDMO::ImButton("Open Song Folder"))
		{
			ShellExecute(0, NULL, Hacks::GetSongFolder().c_str(), NULL, NULL, SW_SHOW);
		}

		if (GDMO::ImButton("Uncomplete Level"))
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

		if (GDMO::ImButton("Inject DLL"))
		{
			auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath2(),
											{"DLL File", "*.dll"}, pfd::opt::multiselect)
								 .result();
			for (auto const& filename : selection)
			{
				LoadLibrary(filename.c_str());
				std::filesystem::path path = filename;
				ExternData::dllNames.push_back(path.filename().string());
			}
		}

		if (GDMO::ImButton("Reset Level"))
		{
			if (playLayer)
				PlayLayer::resetLevelHook(playLayer, 0);
		}

		if (GDMO::ImButton("Toggle Practice Mode"))
		{
			if (playLayer)
				PlayLayer::togglePracticeModeHook(playLayer, 0, !playLayer->m_isPracticeMode);
		}

		ImGui::End();

		GDMO::ImBegin("Pitch Shift");

		ImGui::PushItemWidth(120 * ExternData::screenSizeX * hacks.menuSize);
		if (GDMO::ImButton("Select Song##pitch"))
		{
			auto selection = pfd::open_file("Select a file", CCFileUtils::sharedFileUtils()->getWritablePath(),
											{"Audio File", "*.mp3"}, pfd::opt::none)
								 .result();
			for (auto const& filename : selection)
			{
				std::filesystem::path p = filename;
				memset(hacks.pitchId, 0, sizeof(hacks.pitchId));
				p.stem().string().copy(hacks.pitchId, 10);
			}
		}

		ImGui::SameLine();
		ImGui::Text(hacks.pitchId);

		GDMO::ImInputFloat("Pitch", &pitch);
		ImGui::PopItemWidth();

		if (GDMO::ImButton("Render"))
			Hacks::ChangePitch(pitch);

		ImGui::End();

		GDMO::ImBegin("Nong Downloader");

		ImGui::PushItemWidth(120 * ExternData::screenSizeX * hacks.menuSize);

		GDMO::ImInputText("Song Url", url, 100);
		GDMO::ImInputText("Song Id", id, 30);
		ImGui::PopItemWidth();

		if (GDMO::ImButton("Download"))
			Hacks::NongDownload(url, id);

		ImGui::End();

		GDMO::ImBegin("Internal Recorder");

		if (GDMO::ImCheckbox("Record", &hacks.recording))
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
		GDMO::ImCheckbox("Include Clicks", &hacks.includeClicks);
		ImGui::PushItemWidth(110 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImInputInt2("Size##videosize", hacks.videoDimenstions);
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(75 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImInputInt("Framerate", &hacks.videoFps, 0);
		GDMO::ImInputFloat("Music Volume", &hacks.renderMusicVolume);
		GDMO::ImInputFloat("Click Volume", &hacks.renderClickVolume);
		GDMO::ImInputText("Bitrate", hacks.bitrate, 8);
		GDMO::ImInputText("Codec", hacks.codec, 20);
		GDMO::ImInputText("Extraargs Before -i", hacks.extraArgs, 60);
		GDMO::ImInputText("Extraargs After -i", hacks.extraArgsAfter, 60);
		GDMO::ImInputInt("Click Chunk Size", &hacks.clickSoundChunkSize, 0);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("How many actions does a click chunk file contains? A click chunk file is a part of the "
							  "whole rendered clicks, i have to split them to bypass the command character limit.\nTry "
							  "increasing this if the clicks do not render.");
		GDMO::ImInputFloat("Show End For", &hacks.afterEndDuration);
		ImGui::PopItemWidth();

		GDMO::Marker("Usage",
					 "Hit record in a level and let a macro play. The rendered video will be in "
					 "GDmenu/renders/level - levelid. If you're unsure of what a setting does, leave it on "
					 "default.\n If you're using an NVIDIA GPU i reccomend settings your extra args before -i to: "
					 "-hwaccel cuda -hwaccel_output_format cuda and the encoder to: h264_nvenc.\n If you're using "
					 "an AMD GPU i reccomend setting the encoder to either: h264_amf or hevc_amf.");
		GDMO::Marker("Credits", "All the credits for the recording side goes to matcool's replaybot implementation, i "
								"integrated my clickbot into it");

		ImGui::End();

		GDMO::ImBegin("Variable Changer");

		auto jarray = ExternData::variables["variables"];
		std::vector<std::string> variableTabs;
		std::vector<std::string> variables;

		for (auto jobject : jarray)
		{
			variableTabs.push_back(jobject["name"].get<std::string>());
		}

		ImGui::PushItemWidth(140 * ExternData::screenSizeX * hacks.menuSize);

		ImGui::Combo(
			"Tab", &tabIndex,
			[](void* vec, int idx, const char** out_text) {
				std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
				if (idx < 0 || idx >= vector->size())
					return false;
				*out_text = vector->at(idx).c_str();
				return true;
			},
			reinterpret_cast<void*>(&variableTabs), variableTabs.size());

		auto arr = jarray[tabIndex];

		for (auto job : arr["data"])
		{
			variables.push_back(job["name"].get<std::string>());
		}

		ImGui::Combo(
			"Variable", &variableIndex,
			[](void* vec, int idx, const char** out_text) {
				std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
				if (idx < 0 || idx >= vector->size())
					return false;
				*out_text = vector->at(idx).c_str();
				return true;
			},
			reinterpret_cast<void*>(&variables), variables.size());

		if (variableIndex >= arr["data"].size())
			variableIndex = 0;

		auto type = arr["data"][variableIndex]["pointers"][0]["type"].get<std::string>();

		if (type == "float")
		{
			static float input;
			if (GDMO::ImInputFloat(arr["data"][variableIndex]["name"].get<std::string>().c_str(), &input))
			{
				for (auto ob : arr["data"][variableIndex]["pointers"])
				{
					std::vector<uint32_t> addrs;
					for (auto str : Hacks::splitByDelim(ob["offsets"].get<std::string>(), ' '))
					{
						addrs.push_back(std::stoul(str, nullptr, 16));
					}
					auto addr = GetPointerAddress(addrs);
					arr["data"][variableIndex].contains("is_reference") ? Hacks::WriteRef<float>(addr, input)
																		: Hacks::Write<float>(addr, input);
				}
			}
		}
		else
		{
			static int input;
			if (GDMO::ImInputInt(arr["data"][variableIndex]["name"].get<std::string>().c_str(), &input))
			{
				for (auto ob : arr["data"][variableIndex]["pointers"])
				{
					std::vector<uint32_t> addrs;
					for (auto str : Hacks::splitByDelim(ob["offsets"].get<std::string>(), ' '))
					{
						addrs.push_back(std::stoul(str, nullptr, 16));
					}
					auto addr = GetPointerAddress(addrs);
					arr["data"][variableIndex].contains("is_reference") ? Hacks::WriteRef<int>(addr, input)
																		: Hacks::Write<int>(addr, input);
				}
			}
		}

		ImGui::PopItemWidth();

		ImGui::End();

		GDMO::ImBegin("Extensions");
		char buffer[256];
		sprintf(buffer, "Extensions Loaded: %d", ExternData::dllNames.size());
		GDMO::ImText(buffer);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("dll_lists", 1))
			ImGui::OpenPopup("DLL List");

		if (ImGui::IsPopupOpen("DLL List"))
		{
			ImGui::SetNextWindowSizeConstraints({windowSize, windowSize}, {windowSize * 2, 1000});
		}
		if (ImGui::BeginPopupModal("DLL List"))
		{
			for (const auto& name : ExternData::dllNames)
			{
				GDMO::ImCheckbox(name.c_str(), ExternData::dlls[name].get<bool*>());
			}
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}
		GDMO::ImButton("Extensions Folder");
		if (ImGui::IsItemClicked())
		{
			ShellExecute(NULL, "open", "GDMenu\\dll", NULL, NULL, SW_SHOWNORMAL);
		}
		ImGui::End();

		GDMO::ImBegin("Macrobot");

		if (ReplayPlayer::getInstance().IsRecording())
			ImGui::PushStyleColor(0, ImVec4(0, 1, 0, 1));
		else
			ImGui::PushStyleColor(0, ImVec4(1, 0, 0, 1));
		if (GDMO::ImButton("Toggle Recording"))
			ReplayPlayer::getInstance().ToggleRecording();
		if (ReplayPlayer::getInstance().IsPlaying())
			ImGui::PushStyleColor(0, ImVec4(0, 1, 0, 1));
		else
			ImGui::PushStyleColor(0, ImVec4(1, 0, 0, 1));
		if (GDMO::ImButton("Toggle Playing"))
			ReplayPlayer::getInstance().TogglePlaying();
		else
			ImGui::PushStyleColor(0, ImVec4(1, 1, 1, 1));
		GDMO::ImCheckbox("Show Macro Label", &hacks.botTextEnabled);

		GDMO::ImCheckbox("Click sounds", &hacks.clickbot);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("clicks", 1))
			ImGui::OpenPopup("Click sounds settings");
		if (ImGui::BeginPopupModal("Click sounds settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) ||
			ExternData::fake)
		{
			GDMO::ImInputFloat("Click volume", &hacks.baseVolume);
			GDMO::ImInputFloat("Max pitch variation", &hacks.maxPitch);
			GDMO::ImInputFloat("Min pitch variation", &hacks.minPitch);
			ImGui::InputDouble("Play Medium Clicks at", &hacks.playMediumClicksAt);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Amount of time between click so that a medium click is played");
			GDMO::ImInputFloat("Minimum time difference", &hacks.minTimeDifference);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Minimum time difference for a click to play, to avoid tiny double clicks");
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			GDMO::Marker("?", "Put clicks, releases and mediumclicks in the respective folders found in GDMenu/clicks");
			if (!ExternData::fake)
				ImGui::EndPopup();
		}
		GDMO::ImCheckbox("Prevent inputs", &hacks.preventInput);

		ImGui::PushItemWidth(80 * ExternData::screenSizeX * hacks.menuSize);
		if (ReplayPlayer::getInstance().IsRecording())
		{
			ImGui::BeginDisabled();
			GDMO::ImCombo("Correction Type", &hacks.replayMode, replayMode, 3);
			ImGui::EndDisabled();
		}
		else
			GDMO::ImCombo("Correction Type", &hacks.replayMode, replayMode, 3);

		ImGui::PopItemWidth();

		// GDMO::ImCheckbox("Disable Corrections", &hacks.disableBotCorrection);
		// if (ImGui::IsItemHovered())
		// 	ImGui::SetTooltip("Disable physics correction with the bot, only uses the clicks.");

		GDMO::ImCheckbox("Autoclicker", &hacks.autoclicker);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("aut", 1))
			ImGui::OpenPopup("Autoclicker Settings");

		if (ImGui::BeginPopupModal("Autoclicker Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			GDMO::ImInputFloat("Click time", &hacks.clickTime);
			GDMO::ImInputFloat("Release time", &hacks.releaseTime);
			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		GDMO::ImCheckbox("Frame Step", &hacks.frameStep);
		ImGui::SameLine(arrowButtonPosition * ExternData::screenSizeX * hacks.menuSize);
		if (ImGui::ArrowButton("fra", 1))
			ImGui::OpenPopup("Frame Step Settings");

		if (ImGui::BeginPopupModal("Frame Step Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize) || ExternData::fake)
		{
			GDMO::ImInputInt("Step Count", &hacks.stepCount, 0);
			ImGui::PushItemWidth(180 * ExternData::screenSizeX * hacks.menuSize);
			GDMO::ImHotkey("Step Key", &hacks.stepIndex);
			ImGui::PopItemWidth();
			GDMO::ImCheckbox("Hold to Advance", &hacks.holdAdvance);

			if (GDMO::ImButton("Close", false))
			{
				ImGui::CloseCurrentPopup();
			}
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		ImGui::Spacing();
		if (GDMO::ImButton("Clear actions"))
			ReplayPlayer::getInstance().ClearActions();

		ImGui::Spacing();
		ImGui::PushItemWidth(100 * ExternData::screenSizeX * hacks.menuSize);
		GDMO::ImInputText("Macro Name", ExternData::replayName, 30);
		ImGui::PopItemWidth();
		if (GDMO::ImButton("Save"))
		{
			ReplayPlayer::getInstance().Save(ExternData::replayName);
			GetMacros();
		}

		ImGui::SameLine();

		static bool showSelector = false;

		if (GDMO::ImButton("Open Selector"))
		{
			showSelector = !showSelector;
		}

		bool cool = true;
		if (showSelector)
		{
			ImGui::SetNextWindowSizeConstraints({windowSize * 4, windowSize * 4}, {windowSize * 4, windowSize * 4});
			ImGui::Begin("Selector", &showSelector);
			GDMO::ImInputText("Search", macroName, 100);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {10, 10});
			if (ImGui::BeginTable("table1", 4,
								  ImGuiTableFlags_RowBg | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_Resizable |
									  ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody))
			{
				ImGui::TableSetupColumn("Macro Name");
				ImGui::TableSetupColumn("FPS");
				ImGui::TableSetupColumn("Actions");
				ImGui::TableSetupColumn("Captures");
				ImGui::TableHeadersRow();
				for (auto ri : replays)
				{
					if (macroName[0] != 0 && ri.name.find(macroName) == std::string::npos)
						continue;
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text(ri.name.c_str());
					ImGui::TableNextColumn();
					ImGui::Text(std::to_string(ri.fps).c_str());
					ImGui::TableNextColumn();
					ImGui::Text(std::to_string(ri.actionSize).c_str());
					ImGui::TableNextColumn();
					ImGui::Text(std::to_string(ri.capturesSize).c_str());
					ImGui::SameLine((arrowButtonPosition - 10) * ExternData::screenSizeX * hacks.menuSize);
					if (GDMO::ImButton((std::string("Load##") + ri.name).c_str()))
						ReplayPlayer::getInstance().Load(ri.name);
					ImGui::SameLine((arrowButtonPosition - 70) * ExternData::screenSizeX * hacks.menuSize);
					if (GDMO::ImButton((std::string("Delete##") + ri.name).c_str()))
					{
						ReplayPlayer::getInstance().Delete(ri.name);
						GetMacros();
					}
				}
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
			ImGui::End();
		}

		if (GDMO::ImButton("Load"))
			ReplayPlayer::getInstance().Load(ExternData::replayName);

		ImGui::SameLine();

		if (GDMO::ImButton("Select File"))
		{
			auto selection =
				pfd::open_file("Select a macro", "GDMenu/macros", {"*.macro", "*.replay"}, pfd::opt::none).result();
			if (selection.size() > 0)
			{
				std::filesystem::path p = selection[0];
				ReplayPlayer::getInstance().GetReplay()->Load(p.string());
			}
		}

		ImGui::Spacing();

		if (GDMO::ImButton("Open Converter"))
		{
			ImGui::OpenPopup("Converter");
		}

		if (ImGui::BeginPopupModal("Converter"))
		{
			if (GDMO::ImButton("Export JSON"))
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
				std::ofstream file("GDMenu/macros/" + std::string(ExternData::replayName) + ".mcb.json");
				file << tasmacro;
			}
			if (GDMO::ImButton("Import JSON"))
			{
				auto selection =
					pfd::open_file("Select a macro", "", {"JSON File", "*.mcb.json"}, pfd::opt::none).result();
				for (auto const& filename : selection)
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
					gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
											 ("Macro loaded with " +
											  std::to_string(ReplayPlayer::getInstance().GetActionsSize()) +
											  " actions."))
						->show();
				}
			}
			if (GDMO::ImButton("From TASBOT"))
			{
				auto selection = pfd::open_file("Select a macro", "", {"JSON File", "*.json"}, pfd::opt::none).result();
				for (auto const& filename : selection)
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
					gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
											 ("Macro loaded with " +
											  std::to_string(ReplayPlayer::getInstance().GetActionsSize()) +
											  " actions."))
						->show();
				}
			}
			if (GDMO::ImButton("From zBot"))
			{
				auto selection =
					pfd::open_file("Select a macro", "", {"zBot file", "*.zbot, *.zbf"}, pfd::opt::none).result();
				for (auto const& filename : selection)
				{
					std::filesystem::path path = filename;
					ReplayPlayer::getInstance().ClearActions();
					std::ifstream stream(filename, std::fstream::binary);
					stream.seekg(0, std::fstream::end);
					size_t size = stream.tellg();
					size -= sizeof(float) * 2;
					stream.seekg(0);
					float delta, speed;
					stream.read((char*)&delta, sizeof(float));
					stream.read((char*)&speed, sizeof(float));
					ReplayPlayer::getInstance().GetReplay()->fps = 1 / delta / speed;
					for (int i = 0; i < size / ((path.extension() == ".zbot" ? sizeof(float) : sizeof(uint32_t)) +
												sizeof(bool) + sizeof(bool));
						 i++)
					{
						Action action;
						action.px = -1;
						if (path.extension() == ".zbot")
							stream.read((char*)&action.px, sizeof(float));
						else
							stream.read((char*)&action.frame, sizeof(uint32_t));
						uint8_t cacca1;
						stream.read((char*)&cacca1, sizeof(uint8_t));
						action.press = cacca1 == 0x31;
						uint8_t cacca;
						stream.read((char*)&cacca, sizeof(uint8_t));
						action.player2 = cacca != 0x31;
						action.yAccel = -1;
						ReplayPlayer::getInstance().GetReplay()->AddAction(action);
					}
					gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
											 ("Macro loaded with " +
											  std::to_string(ReplayPlayer::getInstance().GetActionsSize()) +
											  " actions."))
						->show();
				}
			}
			if (GDMO::ImButton("From xBot"))
			{
				auto selection = pfd::open_file("Select a macro", "", {"xBot file", "*.xbot"}, pfd::opt::none).result();
				for (auto const& filename : selection)
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
								*(int*)&ac.px = a;
								ac.frame = -1;
							}

							ac.press = clickType & 1;
							ac.player2 = clickType >> 1;
							ReplayPlayer::getInstance().GetReplay()->AddAction(ac);
						}
					}
					gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
											 ("Macro loaded with " +
											  std::to_string(ReplayPlayer::getInstance().GetActionsSize()) +
											  " actions."))
						->show();
				}
			}
			if (GDMO::ImButton("From ECHO"))
			{
				auto selection = pfd::open_file("Select a macro", "", {"ECHO File", "*.echo"}, pfd::opt::none).result();
				for (auto const& filename : selection)
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
					gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
											 ("Macro loaded with " +
											  std::to_string(ReplayPlayer::getInstance().GetActionsSize()) +
											  " actions."))
						->show();
				}
			}
			if (GDMO::ImButton("From MHREPLAY"))
			{
				auto selection =
					pfd::open_file("Select a macro", "", {"MHR File", "*.mhr.json"}, pfd::opt::none).result();
				for (auto const& filename : selection)
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
					gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
											 ("Macro loaded with " +
											  std::to_string(ReplayPlayer::getInstance().GetActionsSize()) +
											  " actions."))
						->show();
				}
			}
			if (GDMO::ImButton("Close"))
				ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();

		ImGui::End();

		ImGui::PopStyleVar();

		ExternData::resetWindows = false;
		ExternData::repositionWindows = false;
		ExternData::isCheating = PlayLayer::IsCheating();

		if (!ExternData::animationDone && (opening && ExternData::animation == 0 || !opening && ExternData::animation == 1))
		{
			ExternData::animationDone = true;
			ExternData::animationAction = nullptr;
		}
	}
}

void AnimationIsDone()
{
	if (ExternData::animation == 1)
	{
		Hacks::RenderMain();

		Hacks::SaveSettings();

		ExternData::show = false;

		auto p = gd::GameManager::sharedState()->getPlayLayer();

		if (p && !p->m_bIsPaused && !p->m_hasCompletedLevel)
			cocos2d::CCEGLView::sharedOpenGLView()->showCursor(false);
	}
}

DWORD WINAPI my_thread(void* hModule)
{
	ExternData::animation = -10;
	ImGuiHook::setRenderFunction(Hacks::RenderMain);
	ImGuiHook::setInitFunction(Init);
	ImGuiHook::setToggleCallback([]() {
		ExternData::show = !ExternData::show;

		CCEaseRateAction* ac;

		ExternData::animationDone = false;

		ExternData::randomDirection = std::rand() % 4 + 1;

		if (ExternData::show)
		{
			ac = CCEaseIn::create(CustomAction::create(hacks.menuAnimationLength *
														   CCDirector::sharedDirector()->getScheduler()->getTimeScale(),
													   1, 0, &ExternData::animation, nullptr),
								  0.5f);
			for (auto func : ExternData::openFuncs)
			{
				if (func)
					func();
			}
			if (ExternData::hasSaiModPack)
			{
				gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
										 ("Sai Mod Pack is not compatible with this menu, remove it!"))
					->show();
			}

			cocos2d::CCEGLView::sharedOpenGLView()->showCursor(true);
			opening = true;
		}
		else
		{
			ac =
				CCEaseOut::create(CustomAction::create(hacks.menuAnimationLength *
														   CCDirector::sharedDirector()->getScheduler()->getTimeScale(),
													   0, 1, &ExternData::animation, AnimationIsDone),
								  0.5f);

			for (auto func : ExternData::closeFuncs)
			{
				if (func)
					func();
			}

			ExternData::show = true;
			opening = false;
		}

		if (ac)
		{
			CCDirector::sharedDirector()->getRunningScene()->stopActionByTag(4000);
			ac->setTag(4000);
			CCDirector::sharedDirector()->getRunningScene()->runAction(ac);
			ExternData::animationAction = ac;
		}

		GetMacros();
	});
	if (MH_Initialize() == MH_OK)
	{
		ImGuiHook::setupHooks(
			[](void* target, void* hook, void** trampoline) { MH_CreateHook(target, hook, trampoline); });
		SpeedhackAudio::init();
		auto cocos = GetModuleHandleA("libcocos2d.dll");
		auto addr =
			GetProcAddress(cocos, "?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z");

		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x01FB780), PlayLayer::initHook,
					  reinterpret_cast<void**>(&PlayLayer::init));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x2029C0), PlayLayer::updateHook,
					  reinterpret_cast<void**>(&PlayLayer::update));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x20D0D0), PlayLayer::togglePracticeModeHook,
					  reinterpret_cast<void**>(&PlayLayer::togglePracticeMode));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x20a1a0), PlayLayer::destroyPlayer_H,
					  reinterpret_cast<void**>(&PlayLayer::destroyPlayer));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1F4E40), PlayLayer::pushButtonHook,
					  reinterpret_cast<void**>(&PlayLayer::pushButton));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x20D810), PlayLayer::onQuitHook,
					  reinterpret_cast<void**>(&PlayLayer::onQuit));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x15EE00), PlayLayer::editorInitHook,
					  reinterpret_cast<void**>(&PlayLayer::editorInit));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1F4F70), PlayLayer::releaseButtonHook,
					  reinterpret_cast<void**>(&PlayLayer::releaseButton));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x20BF00), PlayLayer::resetLevelHook,
					  reinterpret_cast<void**>(&PlayLayer::resetLevel));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x20A1A0), PlayLayer::hkDeath,
					  reinterpret_cast<void**>(&PlayLayer::death));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1FD3D0), PlayLayer::levelCompleteHook,
					  reinterpret_cast<void**>(&PlayLayer::levelComplete));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x253D60), PlayLayer::triggerObjectHook,
					  reinterpret_cast<void**>(&PlayLayer::triggerObject));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1FFD80), PlayLayer::lightningFlashHook,
					  reinterpret_cast<void**>(&PlayLayer::lightningFlash));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x25FAD0), PlayLayer::uiOnPauseHook,
					  reinterpret_cast<void**>(&PlayLayer::uiOnPause));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x25FD20), PlayLayer::uiTouchBeganHook,
					  reinterpret_cast<void**>(&PlayLayer::uiTouchBegan));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1F9640), PlayLayer::togglePlayerScaleHook,
					  reinterpret_cast<void**>(&PlayLayer::togglePlayerScale));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1f4ff0), PlayLayer::ringJumpHook,
					  reinterpret_cast<void**>(&PlayLayer::ringJump));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xef0e0), PlayLayer::activateObjectHook,
					  reinterpret_cast<void**>(&PlayLayer::activateObject));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x10ed50), PlayLayer::bumpHook,
					  reinterpret_cast<void**>(&PlayLayer::bump));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1FE3A0), PlayLayer::newBestHook,
					  reinterpret_cast<void**>(&PlayLayer::newBest));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xE4A70), PlayLayer::getObjectRectHook,
					  reinterpret_cast<void**>(&PlayLayer::getObjectRect));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xE4B90), PlayLayer::getObjectRectHook2,
					  reinterpret_cast<void**>(&PlayLayer::getObjectRect2));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xEF110), PlayLayer::hasBeenActivatedByPlayerHook,
					  reinterpret_cast<void**>(&PlayLayer::hasBeenActivatedByPlayer));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x14ebc0), PlayLayer::addPointHook,
					  reinterpret_cast<void**>(&PlayLayer::addPoint));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xE5D60), PlayLayer::powerOffObjectHook,
					  reinterpret_cast<void**>(&PlayLayer::powerOffObject));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xEAB20), PlayLayer::playShineEffectHook,
					  reinterpret_cast<void**>(&PlayLayer::playShineEffect));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x207D30), PlayLayer::flipGravityHook,
					  reinterpret_cast<void**>(&PlayLayer::flipGravity));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x207e00), PlayLayer::playGravityEffectHook,
					  reinterpret_cast<void**>(&PlayLayer::playGravityEffect));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1f62c0), PlayLayer::toggleDartModeHook,
					  reinterpret_cast<void**>(&PlayLayer::toggleDartMode));

		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x16B7C0), LevelEditorLayer::drawHook,
					  reinterpret_cast<void**>(&LevelEditorLayer::draw));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x75660), LevelEditorLayer::exitHook,
					  reinterpret_cast<void**>(&LevelEditorLayer::exit));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0xC4BD0), LevelEditorLayer::fadeMusicHook,
					  reinterpret_cast<void**>(&LevelEditorLayer::fadeMusic));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1695A0), LevelEditorLayer::onPlaytestHook,
					  reinterpret_cast<void**>(&LevelEditorLayer::onPlaytest));

		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x94CB0), EndLevelLayer::customSetupHook,
					  reinterpret_cast<void**>(&EndLevelLayer::customSetup));

		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x12ADF0), MenuLayer::onBackHook,
					  reinterpret_cast<void**>(&MenuLayer::onBack));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x18CF40), MenuLayer::loadingStringHook,
					  reinterpret_cast<void**>(&MenuLayer::loadingString));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x1907B0), MenuLayer::hook,
					  reinterpret_cast<void**>(&MenuLayer::init));

		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x17DA60), LevelSearchLayer::hook,
					  reinterpret_cast<void**>(&LevelSearchLayer::init));
		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x9f8e0), LevelSearchLayer::httpHook,
					  reinterpret_cast<void**>(&LevelSearchLayer::http));

		MH_CreateHook(reinterpret_cast<void*>(gd::base + 0x20DDD0), CustomCheckpoint::createHook,
					  reinterpret_cast<void**>(&CustomCheckpoint::create));

		MH_CreateHook(addr, PlayLayer::dispatchKeyboardMSGHook,
					  reinterpret_cast<void**>(&PlayLayer::dispatchKeyboardMSG));
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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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
