#pragma once

#include <unordered_map>

namespace momo
{
	template<typename Key, typename Value>
	class Cacher
	{
		using BaseCache = std::unordered_map<Key, Value>;
		BaseCache cache;
	public:
		void Add(const Key& key, const Value& value);
		void Remove(const Key& key);
		bool Has(const Key& key) const;
		Value& operator[] (const Key& key);
	};

	template<typename Key, typename Value>
	inline void Cacher<Key, Value>::Add(const Key& key, const Value& value)
	{
		cache[key] = value;
	}

	template<typename Key, typename Value>
	inline void Cacher<Key, Value>::Remove(const Key& key)
	{
		if (Has(key))
			cache.erase(key);
	}

	template<typename Key, typename Value>
	inline bool Cacher<Key, Value>::Has(const Key& key) const
	{
		return cache.find(key) != cache.end();
	}

	template<typename Key, typename Value>
	inline Value& Cacher<Key, Value>::operator[](const Key& key)
	{
		return cache[key];
	}
}