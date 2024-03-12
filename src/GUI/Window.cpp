#include "Window.h"
#include "GUI.h"
#include "Settings.hpp"
#include "Blur.h"
#include <Geode/Geode.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

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

		bool blurEnabled = Settings::get<bool>("menu/blur/enabled", false);
		bool blurGD = Settings::get<bool>("menu/blur/gd", false);
		float windowTransparency = Settings::get<float>("menu/window/opacity", 0.98f);
		float blurDarkness = Settings::get<float>("menu/blur/darkness", 1.f);

		if (blurEnabled)
			ImGui::SetNextWindowBgAlpha(blurGD ? (blurDarkness - 0.1f) : 1);
		else
			ImGui::SetNextWindowBgAlpha(windowTransparency);

		ImGui::Begin(name.c_str(), (bool*)0, flags);

		if(Settings::get<bool>("menu/drop_shadow/enabled", true))
			dropShadow(Settings::get<float>("menu/drop_shadow/size", 24.f), 255);

		if(Settings::get<bool>("menu/title_gradient/enabled", true))
			customTitlebar();

		if(blurEnabled && !blurGD)
			Blur::blurWindowBackground();

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

	if (GUI::isVisible && visibleInScreen())
		ImGui::End();
}

//https://github.com/ocornut/imgui/issues/4722
void Window::customTitlebar()
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	draw_list->PushClipRectFullScreen();

	ImVec2 min = ImGui::GetCurrentWindow()->TitleBarRect().Min;
	ImVec2 max = ImGui::GetCurrentWindow()->TitleBarRect().Max;
	auto size_arg = ImVec2(-FLT_MIN, 0.0f);

	auto col_bg = ImGui::GetStyleColorVec4(ImGuiCol_TitleBg);

	ImVec4 bg_color_1 = {col_bg.x * 0.5f, col_bg.y * 1.5f, col_bg.z * 0.75f, col_bg.w};
	ImVec4 bg_color_2 = {col_bg.x * 1.5f, col_bg.y * 0.5f, col_bg.z * 1.25f, col_bg.w};

	ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
	
	const ImVec2 label_size = ImGui::CalcTextSize(ImGui::GetCurrentWindow()->Name, NULL, true);

	ImVec2 pos = {ImGui::GetCurrentWindow()->DC.CursorPos.x, ImGui::GetWindowPos().y};
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);

	int vert_start_idx = draw_list->VtxBuffer.Size;
    draw_list->AddRectFilled(min, max, ImGui::GetColorU32(bg_color_1), g.Style.FrameRounding);
    int vert_end_idx = draw_list->VtxBuffer.Size;
    ImGui::ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, min, max, ImGui::GetColorU32(bg_color_1), ImGui::GetColorU32(bg_color_2));
    if (g.Style.FrameBorderSize > 0.0f)
        draw_list->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameRounding, 0, g.Style.FrameBorderSize);

    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, ImGui::GetCurrentWindow()->Name, NULL, &label_size, style.ButtonTextAlign, &bb);

	draw_list->PopClipRect();
}

//https://gist.github.com/kpcftsz/b044b43213564f2fb32e8685a50daf6a
void Window::dropShadow(float size, ImU8 opacity)
{
	if(!GUI::shadowTexture)
		return;
	
	auto tex_id = (ImTextureID)GUI::shadowTexture->getName();
	ImVec2 p = ImGui::GetWindowPos();
	ImVec2 s = ImGui::GetWindowSize();
	ImVec2 m = {p.x + s.x, p.y + s.y};
	float uv0 = 0.0f;      // left/top region
	float uv1 = 0.333333f; // leftward/upper region
	float uv2 = 0.666666f; // rightward/lower region
	float uv3 = 1.0f;      // right/bottom region
	ImU32 col = (opacity << 24) | 0xFFFFFF;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->PushClipRectFullScreen();
	dl->AddImage(tex_id, {p.x - size, p.y - size}, {p.x,        p.y       }, {uv0, uv0}, {uv1, uv1}, col);
	dl->AddImage(tex_id, {p.x,        p.y - size}, {m.x,        p.y       }, {uv1, uv0}, {uv2, uv1}, col);
	dl->AddImage(tex_id, {m.x,        p.y - size}, {m.x + size, p.y       }, {uv2, uv0}, {uv3, uv1}, col);
	dl->AddImage(tex_id, {p.x - size, p.y       }, {p.x,        m.y       }, {uv0, uv1}, {uv1, uv2}, col);
	dl->AddImage(tex_id, {m.x,        p.y       }, {m.x + size, m.y       }, {uv2, uv1}, {uv3, uv2}, col);
	dl->AddImage(tex_id, {p.x - size, m.y       }, {p.x,        m.y + size}, {uv0, uv2}, {uv1, uv3}, col);
	dl->AddImage(tex_id, {p.x,        m.y       }, {m.x,        m.y + size}, {uv1, uv2}, {uv2, uv3}, col);
	dl->AddImage(tex_id, {m.x,        m.y       }, {m.x + size, m.y + size}, {uv2, uv2}, {uv3, uv3}, col);
	dl->PopClipRect();
}