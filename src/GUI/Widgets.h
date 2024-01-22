#pragma once

#include <iostream>
#include <imgui.h>
#include <functional>

namespace GUI
{
	struct ButtonFunc
	{
		std::string name;
		std::function<void()> function;

		ButtonFunc(std::string name, std::function<void()> function)
			: name(name), function(function)
		{};

		ButtonFunc(std::nullptr_t)
			: name(""), function(nullptr)
		{};

		operator bool() const
		{
			return function != nullptr;
		}
	};

	bool button(std::string);
	bool checkbox(std::string, bool*);
	bool inputText(std::string, std::string*);
	bool inputInt(std::string, int*, int = -INT_MAX, int = INT_MAX);
	bool inputInt2(std::string, int*, int = -INT_MAX, int = INT_MAX, int = -INT_MAX, int = INT_MAX);
	bool inputFloat(std::string, float*, float = -FLT_MAX, float = FLT_MAX);
	bool dragInt(std::string, int*, int = -INT_MAX, int = INT_MAX);
	bool dragFloat(std::string, float*, float = -FLT_MAX, float = FLT_MAX);
	bool combo(std::string, int*, const char* const[], int);
	bool colorEdit(std::string, float*, bool = false, bool = false);
	void arrowButton(std::string);
	bool hotkey(std::string, int*, const ImVec2& = { 0, 0 });
	bool modalPopup(std::string, const std::function<void()>&, int = ImGuiWindowFlags_AlwaysAutoResize);
	bool alertPopup(std::string, std::string, const ButtonFunc&, const ButtonFunc& = nullptr, int = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
	void marker(std::string, std::string);
	void textURL(std::string, std::string);
	void sameLine(float offset_from_start_x = 0, float spacing_w = -1);


	bool customCheckbox(const char* label, bool* v);
};