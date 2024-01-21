#pragma once

#include "json.hpp"
#include <string_view>
#include <iostream>
#include "util.hpp"

using json = nlohmann::json;

namespace
{
	template<typename T>
	constexpr T get_default()
	{
#define DEFINE_DEFAULT(type, def) if constexpr (std::is_same_v<T, type>) default_value = def

		T default_value;

		DEFINE_DEFAULT(bool, false);
		DEFINE_DEFAULT(float, 1.f);
		DEFINE_DEFAULT(int, 1);

		return default_value;
	}
}

namespace Settings
{
	inline json settingsJson;


	template<typename T>
	T get(std::string name, std::remove_pointer_t<T> default_val = get_default<std::remove_pointer_t<T>>())
	{
		if (
			auto split_name = util::split(name, "/");
			split_name.size() > 1
		) {
			nlohmann::json* token = &settingsJson;

			for (unsigned int i = 0; i < split_name.size(); i++)
			{
				if (!token->contains(split_name[i]) && i == split_name.size() - 1)
					(*token)[split_name[i]] = default_val;
				else if (!token->contains(split_name[i]))
					(*token)[split_name[i]] = nlohmann::json::object();

				token = &token->at(split_name[i]);
			}

			return token->get<T>();
		}

		return settingsJson[name].get<T>();
	}

	template<typename T>
	void set(std::string name, T value)
	{
		if (
			auto split_name = util::split(name, "/");
			split_name.size() > 1
		) {
			nlohmann::json* token = &settingsJson;

			for (unsigned int i = 0; i < split_name.size(); i++)
			{
				if (token && token->contains(split_name[i]) && i == split_name.size() - 1)
					(*token)[split_name[i]] = value;
				else if (token && token->contains(split_name[i]))
					token = &token->at(split_name[i]);
			}
		}
		else
			settingsJson[name] = value;
	}


	void save();
	void load();
};