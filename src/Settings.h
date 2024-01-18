#pragma once

#include "json.hpp"
#include <string_view>
#include <iostream>
#include "utils.hpp"
#include "lru_cache.hpp"

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
	inline nlohmann::json settingsJson;
	// increment max_size after adding settings
	inline LRUCache<std::string, nlohmann::json> cache{ 100 };
	inline bool shouldSave = false;


	template<typename T>
	T get(std::string name, std::remove_pointer_t<T> default_val = get_default<std::remove_pointer_t<T>>())
	{
		try
		{
			return cache.get(name).get<T>();
		}
		catch (...) {}

		if (
			auto split_name = utils::split(name, "/");
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

			cache.push(name, *token);
			return token->get<T>();
		}

		cache.push(name, settingsJson[name]);
		return settingsJson[name].get<T>();
	}

	template<typename T>
	void set(std::string name, T value)
	{
		try
		{
			// auto& setting = cache.get(name);

			// if (setting.get<T>() != value)
			// 	shouldSave = true;

			// setting = value;
			cache.get(name) = value;
		}
		catch (...)
		{
			std::cout << "this is bad... key wasn't found!" << '\n';
		}
	}


	void dumpCacheToJson();
	void save();
	void load();
};