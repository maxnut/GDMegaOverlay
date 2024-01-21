#pragma once
#include <MinHook.h>
#include <cocos2d.h>
#include <random>
#include <fstream>
#include <string_view>
#include <algorithm>

inline bool operator==(const cocos2d::CCPoint& a, const cocos2d::CCPoint& b)
{
	return a.x == b.x && a.y == b.y;
}
inline bool operator==(const cocos2d::CCRect& a, const cocos2d::CCRect& b)
{
	return a.origin == b.origin && a.size == b.size;
}

namespace utils
{
#define MEMBERBYOFFSET(type, class, offset) *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(class) + offset)
#define MBO MEMBERBYOFFSET

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

	inline uintptr_t gd_base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
	inline uintptr_t cc_base = reinterpret_cast<uintptr_t>(GetModuleHandle("libcocos2d.dll"));
	inline uintptr_t fmod_base = reinterpret_cast<uintptr_t>(GetModuleHandle("fmod.dll"));

	inline std::vector<uint8_t> hexToBytes(const std::string& hex)
	{
		std::vector<uint8_t> bytes;

		for (unsigned int i = 0; i < hex.length(); i += 3)
		{
			std::string byteString = hex.substr(i, 2);
			uint8_t byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
			bytes.push_back(byte);
		}

		return bytes;
	}

	inline bool writeBytes(std::uintptr_t const address, std::vector<uint8_t> const& bytes, bool vp)
	{
		DWORD oldprot;

		if (vp)
			VirtualProtectEx(
				GetCurrentProcess(),
				reinterpret_cast<LPVOID>(address),
				bytes.size(),
				PAGE_EXECUTE_READWRITE, &oldprot
			);

		auto ret = WriteProcessMemory(
			GetCurrentProcess(),
			reinterpret_cast<LPVOID>(address),
			bytes.data(), bytes.size(),
			nullptr
		);

		if (vp)
			VirtualProtectEx(
				GetCurrentProcess(),
				reinterpret_cast<LPVOID>(address),
				bytes.size(),
				oldprot, &oldprot
			);

		return ret;
	}

	inline std::vector<std::string> split(std::string s, std::string delimiter)
	{
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
		{
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			res.push_back(token);
		}

		res.push_back(s.substr(pos_start));
		return res;
	}

	inline int randomInt(int min, int max)
	{
		std::random_device device;
		std::mt19937 generator(device());
		std::uniform_int_distribution<int> distribution(min, max);

		return distribution(generator);
	}

	// typeid(...).name() returns "class ClassName", the "+ 6" removes "class "
	inline std::string_view getClassName(cocos2d::CCObject* obj)
	{
		return (typeid(*obj).name() + 6);
	}

	template<typename T, std::size_t S>
	inline std::size_t getElementCount(std::array<T, S> const& array, T elem)
	{
		return std::count(array.begin(), array.end(), elem);
	}
	// nullptr overload
	template<typename T, std::size_t S>
	inline std::size_t getElementCount(std::array<T*, S> const& array, std::nullptr_t elem)
	{
		return std::count(array.begin(), array.end(), elem);
	}
}