#pragma once

// https://github.com/lamerman/cpp-lru-cache
// with modifications from SpaghettDev
#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>

template<typename K, typename V>
class LRUCache
{
public:
	using key_value_pair_t = std::pair<K, V>;
	using list_iterator_t = std::list<key_value_pair_t>::iterator;

	LRUCache(std::size_t max_size)
		: m_max_size(max_size)
	{}

	void push(const K& key, const V& value)
	{
		auto it = m_cache_items_map.find(key);
		m_cache_items_list.push_front(key_value_pair_t{ key, value });

		if (it != m_cache_items_map.end())
		{
			m_cache_items_list.erase(it->second);
			m_cache_items_map.erase(it);
		}
		m_cache_items_map[key] = m_cache_items_list.begin();

		if (m_cache_items_map.size() > m_max_size)
			throw std::runtime_error("Max size of cache has been exceeded!");
	}

	V& get(const K& key)
	{
		auto it = m_cache_items_map.find(key);

		if (it == m_cache_items_map.end())
			throw std::range_error("There is no such key in cache");
		else
		{
			m_cache_items_list.splice(m_cache_items_list.begin(), m_cache_items_list, it->second);
			return it->second->second;
		}
	}

	bool exists(const K& key) const { return m_cache_items_map.find(key) != m_cache_items_map.end(); }
	std::size_t size() const { return m_cache_items_map.size(); }
	auto begin() const { return m_cache_items_map.cbegin(); }
	auto end() const { return m_cache_items_map.cend(); }

private:
	std::list<key_value_pair_t> m_cache_items_list;
	std::unordered_map<K, list_iterator_t> m_cache_items_map;
	std::size_t m_max_size;
};
