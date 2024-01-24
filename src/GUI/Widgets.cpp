#include "Widgets.h"
#include "../Common.h"
#include "../ConstData.h"
#include "GUI.h"
#include "Shortcut.h"

#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <Geode/platform/windows.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
using namespace geode::prelude;

inline ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
	return {a.x + b.x, a.y + b.y};
}
inline ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
	return {a.x - b.x, a.y - b.y};
}

bool GUI::button(std::string name)
{
	bool result = false;
	if (GUI::shouldRender())
		result = ImGui::Button(name.c_str());

	if (!result)
		result = GUI::Shortcut::handleShortcut(name);

	return result;
}

bool GUI::checkbox(std::string name, bool* value)
{
	bool result = false;
	if (GUI::shouldRender())
		result = customCheckbox(name.c_str(), value);

	if (!result)
	{
		result = GUI::Shortcut::handleShortcut(name);

		if (result)
			*value = !*value;
	}

	return result;
}

bool GUI::checkbox(std::string name, std::string settingName, bool default_value)
{
	bool result = false;

	bool value = Mod::get()->getSavedValue(settingName, default_value);

	if (GUI::shouldRender())
		result = customCheckbox(name.c_str(), &value);

	if (!result)
	{
		result = GUI::Shortcut::handleShortcut(name);

		if (result)
			value = !value;
	}

	if(result)
		Mod::get()->setSavedValue(settingName, value);

	return result;
}

bool GUI::hotkey(std::string name, int* keyPointer, const ImVec2& size_arg)
{
	if(!shouldRender())
		return false;
	
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(name.c_str());
	const ImVec2 label_size = ImGui::CalcTextSize(name.c_str(), NULL, true);
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
			*keyPointer = 0;

		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	}
	else if (io.MouseClicked[0])
	{
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	bool value_changed = false;
	int key = *keyPointer;

	if (g.ActiveId == id)
	{
		if (!value_changed)
		{
			for (int i = (int)ImGuiKey_Tab; i <= (int)ImGuiKey_KeypadEqual; i++)
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
			*keyPointer = 0;
			ImGui::ClearActiveID();
		}
		else
			*keyPointer = key;
	}

	char buf_display[64] = "None";

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]), true,
					   style.FrameRounding);

	int displayKeyIndex = *keyPointer - 511;
	if(displayKeyIndex < 0)
		displayKeyIndex = 0;

	if (*keyPointer != 0 && g.ActiveId != id)
		strcpy_s(buf_display, KeyNames[displayKeyIndex]);
	else if (g.ActiveId == id)
		strcpy_s(buf_display, "<Press a key>");

	const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y);
	ImVec2 render_pos = frame_bb.Min + style.FramePadding;
	ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL,
							 NULL, style.ButtonTextAlign, &clip_rect);

	if (label_size.x > 0)
		ImGui::RenderText(ImVec2(total_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), name.c_str());

	return value_changed;
}

bool GUI::modalPopup(std::string name, const std::function<void()>& popupFunction, int flags)
{
	if (!GUI::isVisible || ImGui::BeginPopupModal(name.c_str(), NULL, flags) || GUI::shortcutLoop)
	{
		popupFunction();

		if (GUI::shouldRender())
		{
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	return true;
}

bool GUI::alertPopup(std::string name, std::string content, const ButtonFunc& yesButton, const ButtonFunc& noButton,
					 int flags)
{
	if (!ImGui::IsPopupOpen(name.c_str()))
		ImGui::OpenPopup(name.c_str());

	if (ImGui::BeginPopupModal(name.c_str(), NULL, flags))
	{
		ImVec2 displaySize = ImGui::GetIO().DisplaySize;
		ImVec2 windowSize = ImGui::GetWindowSize();

		ImGui::SetWindowPos({(displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f});

		ImGui::Text(content.c_str());

		if (ImGui::Button(yesButton.name.c_str()))
		{
			yesButton.function();
			ImGui::CloseCurrentPopup();
		}

		if (noButton)
		{
			GUI::sameLine();

			if (ImGui::Button(noButton.name.c_str()))
			{
				noButton.function();
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	return true;
}

void GUI::arrowButton(std::string popupName)
{
	if (!GUI::shouldRender())
		return;

	float ww = ImGui::GetWindowSize().x;

	GUI::sameLine(ww - 39);
	if (ImGui::ArrowButton((popupName + "arrow").c_str(), 1))
		ImGui::OpenPopup(popupName.c_str());
}

bool GUI::combo(std::string name, int* value, const char* const items[], int itemsCount)
{
	if (!GUI::shouldRender())
		return false;

	ImGui::PushItemWidth(80);
	bool result = ImGui::Combo(name.c_str(), value, items, itemsCount);
	ImGui::PopItemWidth();
	return result;
}

bool GUI::inputInt(std::string name, int* value, int min, int max)
{
	bool result = false;
	if (GUI::shouldRender())
	{
		ImGui::PushItemWidth(50);
		result = ImGui::InputInt(name.c_str(), value, 0);
		ImGui::PopItemWidth();
	}

	if (*value < min)
		*value = min;
	if (*value > max)
		*value = max;

	return result;
}

bool GUI::inputText(std::string name, std::string* value)
{
	bool result = false;
	if (GUI::shouldRender())
	{
		ImGui::PushItemWidth(80);
		result = ImGui::InputText(name.c_str(), value);
		ImGui::PopItemWidth();
	}

	return result;
}

bool GUI::inputInt2(std::string name, int* value, int min1, int max1, int min2, int max2)
{
	bool result = false;
	if (GUI::shouldRender())
	{
		ImGui::PushItemWidth(90);
		result = ImGui::InputInt2(name.c_str(), value, 0);
		ImGui::PopItemWidth();
	}

	if (value[0] < min1)
		value[0] = min1;
	if (value[0] > max1)
		value[0] = max1;

	if (value[1] < min2)
		value[1] = min2;
	if (value[1] > max2)
		value[1] = max2;

	return result;
}

bool GUI::inputFloat(std::string name, float* value, float min, float max)
{
	bool result = false;
	if (GUI::shouldRender())
	{
		ImGui::PushItemWidth(50);
		result = ImGui::InputFloat(name.c_str(), value);
		ImGui::PopItemWidth();
	}

	if (*value < min)
		*value = min;
	if (*value > max)
		*value = max;

	return result;
}

bool GUI::dragInt(std::string name, int* value, int min, int max)
{
	bool result = false;
	if (GUI::shouldRender())
	{
		ImGui::PushItemWidth(50);
		result = ImGui::DragInt(name.c_str(), value);
		ImGui::PopItemWidth();
	}

	if (*value < min)
		*value = min;
	if (*value > max)
		*value = max;

	return result;
}

bool GUI::dragFloat(std::string name, float* value, float min, float max)
{
	bool result = false;
	if (GUI::shouldRender())
	{
		ImGui::PushItemWidth(50);
		result = ImGui::DragFloat(name.c_str(), value);
		ImGui::PopItemWidth();
	}

	if (*value < min)
		*value = min;
	if (*value > max)
		*value = max;

	return result;
}

bool GUI::colorEdit(std::string name, float* color, bool inputs, bool alpha)
{
	if (!GUI::shouldRender())
		return false;

	bool result = false;

	if (alpha)
		result = ImGui::ColorEdit4(name.c_str(), color, inputs ? 0 : ImGuiColorEditFlags_NoInputs);
	else
		result = ImGui::ColorEdit3(name.c_str(), color, inputs ? 0 : ImGuiColorEditFlags_NoInputs);

	return result;
}

void GUI::marker(std::string title, std::string description)
{
	if (!GUI::shouldRender())
		return;

	ImGui::TextDisabled(title.c_str());
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(description.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

inline void AddUnderLine(ImColor col_)
{
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	min.y = max.y;
	ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
}

void GUI::textURL(std::string text, std::string link)
{
	if (!GUI::shouldRender())
		return;

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 1, 1));
	ImGui::Text(text.c_str());
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseClicked(0))
		{
			ImGui::FocusWindow(nullptr);
			web::openLinkInBrowser(link.c_str());
		}
		AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::SetTooltip("Open in browser\n%s", link.c_str());
	}
	else
	{
		AddUnderLine(ImGui::GetStyle().Colors[ImGuiCol_Button]);
	}
}

#include "imgui_internal.h"

bool GUI::customCheckbox(const char* label, bool* v)
{
	using namespace ImGui;
	ImGuiIO& io = ImGui::GetIO();
	const float cc_sz = 3.0f * io.FontGlobalScale;
	const float cc_pad = 10.0f;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2,
																			  label_size.y + style.FramePadding.y * 2));
	ItemSize({check_bb.GetWidth(), check_bb.GetHeight() - 3}, style.FramePadding.y);

	ImRect total_bb = check_bb;
	if (label_size.x > 0)
		SameLine(0, style.ItemInnerSpacing.x);
	const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y),
						 window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
	if (label_size.x > 0)
	{
		ItemSize({check_bb.GetWidth(), check_bb.GetHeight() - 3}, style.FramePadding.y);
		total_bb = ImRect(check_bb.Min, ImVec2(text_bb.Max.x - cc_pad * 2.0f * io.FontGlobalScale, check_bb.Max.y));
	}

	if (!ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);
	ImVec2 text_pos = text_bb.GetTL();
	if (hovered)
	{
		ImVec2 text_size = text_bb.GetBR();
		auto fill_color = ImGui::ColorConvertFloat4ToU32(ImVec4(50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f));
		window->DrawList->AddRectFilled(
			ImVec2(check_bb.Min.x, check_bb.Min.y),
			ImVec2(check_bb.Min.x + cc_sz + text_size.x - text_pos.x + cc_pad * 2.0f, check_bb.Max.y), fill_color,
			style.FrameRounding);
	}
	if (*v)
	{
		window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x, check_bb.Min.y),
										ImVec2(check_bb.Min.x + cc_sz, check_bb.Max.y), GetColorU32(ImGuiCol_CheckMark),
										style.FrameRounding);
	}
	else
	{
		window->DrawList->AddRectFilled(ImVec2(check_bb.Min.x, check_bb.Min.y),
										ImVec2(check_bb.Min.x + cc_sz, check_bb.Max.y), ImColor(32, 32, 32),
										style.FrameRounding);
	}

	if (label_size.x > 0.0f)
		RenderText(ImVec2(check_bb.Min.x + cc_sz + cc_pad, text_pos.y), label);

	return pressed;
}

void GUI::sameLine(float offset_from_start_x, float spacing_w)
{
	if(!GUI::shouldRender())
		return;

	ImGui::SameLine(offset_from_start_x, spacing_w);
}