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

		ButtonFunc(const std::string& name, std::function<void()> function)
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

	bool button(const std::string&);
	bool checkbox(const std::string&, bool*);
	bool checkbox(const std::string&, const std::string&, bool = false);
	bool inputText(const std::string&, std::string*);
	bool inputInt(const std::string&, int*, int = -INT_MAX, int = INT_MAX);
	bool inputInt(const std::string&, const std::string&, int, int = -INT_MAX, int = INT_MAX);
	bool inputInt2(const std::string&, int*, int = -INT_MAX, int = INT_MAX, int = -INT_MAX, int = INT_MAX);
	bool inputFloat(const std::string&, float*, float = -FLT_MAX, float = FLT_MAX);
	bool inputFloat(const std::string&, const std::string&, float, float = -FLT_MAX, float = FLT_MAX);
	bool dragInt(const std::string&, int*, int = -INT_MAX, int = INT_MAX);
	bool dragFloat(const std::string&, float*, float = -FLT_MAX, float = FLT_MAX);
	bool combo(const std::string&, int*, const char* const[], int);
	bool colorEdit(const std::string&, float*, bool = false, bool = false);
	void arrowButton(const std::string&);
	bool hotkey(const std::string&, int*, const ImVec2& = { 0, 0 });
	bool modalPopup(const std::string&, const std::function<void()>&, int = ImGuiWindowFlags_AlwaysAutoResize);
	bool alertPopup(const std::string&, const std::string&, const ButtonFunc&, const ButtonFunc& = nullptr, int = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
	void marker(const std::string&, const std::string&);
	void textURL(const std::string&, const std::string&);
	void sameLine(float offset_from_start_x = 0, float spacing_w = -1);


	bool customCheckbox(const char* label, bool* v);
};