#pragma once

#include <string>

#include <Geode/Geode.hpp>

namespace Settings
{
	using namespace geode::prelude;

	namespace impl
	{
		template<typename T>
		constexpr T get_default()
		{
			#define DEFINE_DEFAULT(type, def) if constexpr (std::is_same_v<T, type>) default_value = def

			T default_value;

			DEFINE_DEFAULT(bool, false);
			DEFINE_DEFAULT(float, .0f);
			DEFINE_DEFAULT(int, 0);
			DEFINE_DEFAULT(std::string, "");

			return default_value;
		}
	}

	template<typename T>
	T get(const std::string_view name, const T& default_value = impl::get_default<T>())
	{
		if (Mod::get()->getSaveContainer().contains(name))
			return Mod::get()->getSavedValue<T>(name);
		else
		{
			Mod::get()->setSavedValue<T>(name, default_value);
			return default_value;
		}
	}
}
