#include "API.h"
#include "ConstData.h"
#include "ExternData.h"
#include "Hacks.h"
#include "ImgUtil.h"
#include "Shortcuts.h"
#include "fstream"
#include "imgui-hook.hpp"
#include "imgui_internal.h"
#include <imgui.h>

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
	return {a.x + b.x, a.y + b.y};
}
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
	return {a.x - b.x, a.y - b.y};
}

int shortcutIndexKey;

int roundInt(int n)
{
	int a = (n / hacks.windowSnap) * hacks.windowSnap;
	int b = a + hacks.windowSnap;
	return (n - a > b - n) ? b : a;
}

void GDMO::ImBegin(const char* name, bool* open)
{
	const int windowWidth = 220;
	const float size = ExternData::screenSizeX * hacks.menuSize;
	const float windowSize = windowWidth * size;
	ImGui::SetNextWindowSizeConstraints({windowSize, 1}, {windowSize, 10000});
	ImGui::Begin(name, open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
	ImGui::SetWindowFontScale(ExternData::screenSizeX * hacks.menuSize);

	if (!ExternData::windowPositions.contains(name))
	{
		auto pos = ImGui::GetWindowPos();
		ExternData::windowPositions[name]["x"] = (int)pos.x;
		ExternData::windowPositions[name]["y"] = (int)pos.y;
	}

	float winX = ExternData::windowPositions[name]["x"] * ExternData::screenSizeX;
	float winY = ExternData::windowPositions[name]["y"] * ExternData::screenSizeY;

	if (!ExternData::animationDone)
	{
		uint8_t animationType = (strlen(name) + ExternData::randomDirection) % 4;
		int xoff = 0, yoff = 0;
		switch (animationType)
		{
		case 0:
			xoff = 1400;
			yoff = 1400;
			break;
		case 1:
			xoff = 1400;
			yoff = -1400;
			break;
		case 2:
			xoff = -1400;
			yoff = 1400;
			break;
		case 3:
			xoff = -1400;
			yoff = -1400;
			break;
		}
		ImGui::SetWindowPos({winX - (ExternData::animation * xoff), winY - (ExternData::animation * yoff)});
	}
	else
	{
		if (ExternData::resetWindows && ExternData::windowPositions.contains(name))
			ImGui::SetWindowPos({winX, winY});

		if (hacks.windowSnap > 1)
		{
			auto pos = ImGui::GetWindowPos();
			ImGui::SetWindowPos({(float)roundInt(pos.x), (float)roundInt(pos.y)});
		}
	}

	if (ImGui::IsMouseDragging(0) && ImGui::IsWindowFocused() && ExternData::animationDone &&
		ExternData::animation <= 0)
	{
		auto pos = ImGui::GetWindowPos();
		ExternData::windowPositions[name]["x"] = pos.x / ExternData::screenSizeX;
		ExternData::windowPositions[name]["y"] = pos.y / ExternData::screenSizeY;
	}
}

void GDMO::ImEnd()
{
	ImGui::End();
}

void GDMO::ImText(const char* text)
{
	ImGui::Text(text);
}

void GDMO::addImGuiFunc(std::function<void()> func)
{
	ExternData::imguiFuncs.push_back(func);
}

bool Hotkey(const char* label, int* k, const ImVec2& size_arg = ImVec2(0, 0))
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);
	const ImRect frame_bb(window->DC.CursorPos + ImVec2(label_size.x + style.ItemInnerSpacing.x, 0.0f),
						  window->DC.CursorPos + size);
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

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]), true,
					   style.FrameRounding);

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
	ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL,
							 NULL, style.ButtonTextAlign, &clip_rect);

	if (label_size.x > 0)
		ImGui::RenderText(ImVec2(total_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}

void GDMO::Marker(const char* marker, const char* desc)
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

bool findStringIC(const std::string& strHaystack, const std::string& strNeedle)
{
	auto it = std::search(strHaystack.begin(), strHaystack.end(), strNeedle.begin(), strNeedle.end(),
						  [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
	return (it != strHaystack.end());
}

bool GDMO::ImHotkey(const char* label, int* k)
{
	const ImVec2& size_arg = {0, 0};
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

bool GDMO::ImCheckbox(const char* label, bool* v, bool canMakeShortcut)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (openpopuptemp == true)
		{
			ImGui::OpenPopup(("Create shortcut for " + labelString).c_str());
			openpopuptemp = false;
		}

		if (ImGui::BeginPopupModal(("Create shortcut for " + labelString).c_str(), NULL,
								   ImGuiWindowFlags_AlwaysAutoResize))
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
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (labelString == ExternData::hackName)
		{
			ExternData::hackName = "";
			*v = !*v;
			res = true;
			Hacks::SaveSettings();
		}
	}

	return res;
}

bool GDMO::ImButton(const char* label, bool canMakeShortcut)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (openpopuptemp == true)
		{
			ImGui::OpenPopup(("Create shortcut for " + labelString).c_str());
			openpopuptemp = false;
		}

		if (ImGui::BeginPopupModal(("Create shortcut for " + labelString).c_str(), NULL,
								   ImGuiWindowFlags_AlwaysAutoResize))
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
			if (!ExternData::fake)
				ImGui::EndPopup();
		}

		if (labelString == ExternData::hackName)
		{
			ExternData::hackName = "";
			res = true;
		}
	}

	return res;
}

bool GDMO::ImInputFloat(const char* label, float* v)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

bool GDMO::ImInputInt(const char* label, int* v, int step)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

bool GDMO::ImInputInt2(const char* label, int* v)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

bool GDMO::ImInputText(const char* label, char* buf, size_t buf_size)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

bool GDMO::ImCombo(const char* label, int* current_item, const char* const* items, int items_count)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

bool GDMO::ImColorEdit3(const char* label, float* col, int flags)
{
	bool res = false;
	if (strlen(ExternData::searchbar) > 0 || hacks.showPotentialCheats)
	{
		std::string s = label;
		if (!hacks.showPotentialCheats && findStringIC(s, ExternData::searchbar) || std::count(Hacks::cheatNames.begin(), Hacks::cheatNames.end(), s) > 0)
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

template <class T> void GDMO::WriteRef(uint32_t vaddress, const T& value)
{
	Hacks::writeOutput(vaddress, value);
}
template <class T> bool GDMO::Write(uint32_t vaddress, const T& value)
{
	Hacks::Write(vaddress, value);
}
std::vector<uint8_t> GDMO::ReadBytes(uint32_t vaddress, size_t size)
{
	Hacks::ReadBytes(vaddress, size);
}
template <class T> T GDMO::Read(uint32_t vaddress)
{
	Hacks::Read(vaddress);
}
bool GDMO::writeBytes(std::uintptr_t const address, std::vector<uint8_t> const& bytes)
{
	return Hacks::writeBytes(address, bytes);
}
void GDMO::writeOutput(std::string out)
{
	Hacks::writeOutput(out);
}
void GDMO::writeOutput(int out)
{
	Hacks::writeOutput(out);
}
void GDMO::writeOutput(float out)
{
	Hacks::writeOutput(out);
}
void GDMO::writeOutput(double out)
{
	Hacks::writeOutput(out);
}

template <class T> void Save(const char* filename, const char* key, T value)
{
	ExternData::settingFiles.at(filename)[key] = value;
}

void GDMO::SaveInt(const char* filename, const char* key, int value)
{
	Save(filename, key, value);
}
void GDMO::SaveBool(const char* filename, const char* key, bool value)
{
	Save(filename, key, value);
}
void GDMO::SaveFloat(const char* filename, const char* key, float value)
{
	Save(filename, key, value);
}
void GDMO::SaveDouble(const char* filename, const char* key, double value)
{
	Save(filename, key, value);
}
void GDMO::SaveString(const char* filename, const char* key, const char* value)
{
	Save(filename, key, value);
}
void GDMO::ReadInt(const char* filename, const char* key, int* value)
{
	auto json = ExternData::settingFiles.at(filename);
	if (json.contains(key))
		*value = json[key].get<int>();
}
void GDMO::ReadBool(const char* filename, const char* key, bool* value)
{
	auto json = ExternData::settingFiles.at(filename);
	if (json.contains(key))
		*value = json[key].get<bool>();
}
void GDMO::ReadFloat(const char* filename, const char* key, float* value)
{
	auto json = ExternData::settingFiles.at(filename);
	if (json.contains(key))
		*value = json[key].get<float>();
}
void GDMO::ReadDouble(const char* filename, const char* key, double* value)
{
	auto json = ExternData::settingFiles.at(filename);
	if (json.contains(key))
		*value = json[key].get<double>();
}
void GDMO::ReadString(const char* filename, const char* key, std::string* value)
{
	auto json = ExternData::settingFiles.at(filename);
	if (json.contains(key))
		*value = json[key].get<std::string>();
}

void GDMO::Init(std::string name, std::function<void()> openCallback, std::function<void()> closeCallback)
{
	if (!std::filesystem::is_directory("GDMenu/extsettings") || !std::filesystem::exists("GDMenu/extsettings"))
	{
		std::filesystem::create_directory("GDMenu/extsettings");
	}
	auto path = "GDMenu/extsettings/" + name + ".json";
	if (!std::filesystem::exists(path))
	{
		std::ofstream file(path);
		file.close();
	}
	std::ifstream mods;
	std::stringstream buffer;
	mods.open(path);
	buffer << mods.rdbuf();
	json json;
	if (buffer.rdbuf()->in_avail() > 0)
		json = json::parse(buffer.str());
	ExternData::settingFiles.insert({name, json});
	mods.close();
	buffer.clear();
	ExternData::openFuncs.push_back(openCallback);
	ExternData::closeFuncs.push_back(closeCallback);
}