#pragma once

#include <functional>
#include <imgui/imgui.h>
#include <iostream>

namespace GUI
{
	class Window
	{
	private:
		ImGuiWindowFlags flags = 0;

		std::function<void()> windowFunction;

	public:
		std::string name = "";
		ImVec2 position = ImVec2(0, 0);
		ImVec2 renderPosition = ImVec2(0, 0);

		ImVec2 size = ImVec2(220, 120);

		ImVec2 minSize = ImVec2(0, 0);
		ImVec2 maxSize = ImVec2(0, 0);

	public:
		Window(std::string name, const std::function<void()>& windowFunction);

		void addFlag(ImGuiWindowFlags flag);

		void draw();

		bool visibleInScreen();
	};
};