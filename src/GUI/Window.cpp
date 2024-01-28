#include "Window.h"
#include "GUI.h"
#include "Settings.hpp"

#include <Geode/Geode.hpp>
using namespace geode::prelude;

using namespace GUI;

float roundSnap(float n, int snap)
{
	int a = (static_cast<int>(n) / snap) * snap;
	int b = a + snap;
	return static_cast<float>((n - a > b - n) ? b : a);
}

Window::Window(const std::string& name, const std::function<void()>& windowFunction)
{
	this->name = name;
	this->windowFunction = windowFunction;
}

void Window::addFlag(ImGuiWindowFlags flag)
{
	flags |= flag;
}

bool Window::visibleInScreen()
{
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	ImVec2 windowSize = size;

	return renderPosition.x + windowSize.x >= 0 && renderPosition.y + windowSize.y >= 0 &&
		   renderPosition.x <= screenSize.x && renderPosition.y <= screenSize.y;
}

void Window::draw()
{
	if (GUI::isVisible && visibleInScreen())
	{
		if (maxSize.x != 0 && maxSize.y != 0)
			ImGui::SetNextWindowSizeConstraints(minSize, maxSize);

		float windowTransparency = Settings::get<float>("menu/window/opacity", 0.98f);

		ImGui::SetNextWindowBgAlpha(windowTransparency);

		ImGui::Begin(name.c_str(), (bool*)0, flags);

		if (ImGui::IsMouseDragging(0, 0.1f) && ImGui::IsWindowFocused())
		{
			position = ImGui::GetWindowPos();
			size = ImGui::GetWindowSize();
			int snap = Settings::get<int>("menu/window/snap", 10);
			if (snap > 1.f)
				position = {roundSnap(position.x, snap), roundSnap(position.y, snap)};

			int sizeSnap = Settings::get<int>("menu/window/size_snap", 10);
			if (sizeSnap > 1.f)
				size = {roundSnap(size.x, sizeSnap), roundSnap(size.y, sizeSnap)};

			renderPosition = position;

			GUI::setJsonPosition(name, position);
			GUI::setJsonSize(name, size);
		}

		ImGui::SetWindowSize(size);
		ImGui::SetWindowPos(renderPosition);
	}

	if (GUI::isVisible && visibleInScreen() || !GUI::isVisible)
		windowFunction();

	if (GUI::isVisible)
		ImGui::End();
}