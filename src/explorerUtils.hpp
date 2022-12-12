#pragma once
#include <imgui-hook.hpp>
#include <imgui.h>
#include "pch.h"

std::string get_module_name(HMODULE mod) {
	char buffer[MAX_PATH];
	if (!mod || !GetModuleFileNameA(mod, buffer, MAX_PATH))
		return "Unknown";
	return std::filesystem::path(buffer).filename().string();
}

std::string format_addr(void* addr) {
	HMODULE mod;

	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<char*>(addr), &mod))
		mod = NULL;

	std::stringstream stream;
	stream << get_module_name(mod) << "." << reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(addr) - reinterpret_cast<uintptr_t>(mod));
	return stream.str();
}

// Allows you to access protected members from any class
// by generating a class that inherits the target class
// to then access the member.
// Takes in a pointer btw
// Example:
//   CCNode* foo;
//   public_cast(foo, m_fRotation) = 10.f;
#define public_cast(value, member) [](auto* v) { \
	class FriendClass__; \
	using T = std::remove_pointer<decltype(v)>::type; \
	class FriendeeClass__: public T { \
	protected: \
		friend FriendClass__; \
	}; \
	class FriendClass__ { \
	public: \
		auto& get(FriendeeClass__* v) { return v->member; } \
	} c; \
	return c.get(reinterpret_cast<FriendeeClass__*>(v)); \
}(value)

template <typename T, typename U>
T union_cast(U value) {
	union {
		T a;
		U b;
	} u;
	u.b = value;
	return u.a;
}

void set_clipboard_text(std::string_view text) {
	if (!OpenClipboard(NULL)) return;
	if (!EmptyClipboard()) return;
	const auto len = text.size();
	auto mem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
	memcpy(GlobalLock(mem), text.data(), len + 1);
	GlobalUnlock(mem);
	SetClipboardData(CF_TEXT, mem);
	CloseClipboard();
}

bool operator!=(const cocos2d::CCSize& a, const cocos2d::CCSize& b) { return a.width != b.width || a.height != b.height; }
ImVec2 operator*(const ImVec2& vec, const float m) { return { vec.x * m, vec.y * m }; }
ImVec2 operator/(const ImVec2& vec, const float m) { return { vec.x / m, vec.y / m }; }
ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return { a.x + b.x, a.y + b.y }; }
ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return { a.x - b.x, a.y - b.y }; }

bool operator==(const cocos2d::CCPoint& a, const cocos2d::CCPoint& b) { return a.x == b.x && a.y == b.y; }
bool operator==(const cocos2d::CCRect& a, const cocos2d::CCRect& b) { return a.origin == b.origin && a.size == b.size; }

cocos2d::CCPoint& operator-=(cocos2d::CCPoint& point, const cocos2d::CCPoint& other) {
	point = point - other;
	return point;
}

ImVec2 cocos_to_vec2(const cocos2d::CCPoint& a) {
	const auto size = ImGui::GetMainViewport()->Size;
	const auto win_size = cocos2d::CCDirector::sharedDirector()->getWinSize();
	return {
		a.x / win_size.width * size.x,
		(1.f - a.y / win_size.height) * size.y
	};
}

ImVec2 cocos_to_vec2(const cocos2d::CCSize& a) {
	const auto size = ImGui::GetMainViewport()->Size;
	const auto win_size = cocos2d::CCDirector::sharedDirector()->getWinSize();
	return {
		a.width / win_size.width * size.x,
		-a.height / win_size.height * size.y
	};
}