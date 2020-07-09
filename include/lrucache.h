/************************************************************************/
/*               参考boost的 lru_cache ，扩展了功能
/*               1、支持相同key的插入
/*               2、支持过期时间
/*               3、过期数据和溢出数据 使用惰性删除
/*
/*				 使用方式：
/*				 CLRU_cache<int,std::string> nameCache(100000); // 最多保留100000个name
/*               logonCache.insert(1，"chen");
/*				 logonCache.insert(2，"li");
/*               logonCache.setttl(2,5);	     // 设置key（2）过期时间为5s
/*				 logonCache.insert(3，"wan",10);	 // 插入数据，并在10s 后过期
/*
/*				 auto name = logonCache.get(1);	  // 返回一个value的指针
/*				 if(name){				          // 对象是否存在
/*					std::string myname = *name;	  // 获取对象的值
/*				 }
/************************************************************************/

#ifndef LRUCACHE_H_
#define LRUCACHE_H_

#include <map>
#include <list>

#include <boost/optional.hpp>

// a cache which evicts the least recently used item when it is full
template <class Key, class Value>
class CLRU_cache
{
public:
	typedef Key key_type;
	typedef Value value_type;
	typedef std::list<key_type> list_type;
	typedef std::map<
		key_type,
		std::tuple<value_type, time_t, typename list_type::iterator>>
		map_type;

	// 禁止使用指针作为value类型，否则作用时间不受控制
	static_assert(!std::is_pointer<value_type>::value, "CLRU_cache cannot use point type as value");

	CLRU_cache(size_t capacity)
		: m_capacity(capacity)
	{
	}

	~CLRU_cache()
	{
	}

	size_t size() const
	{
		return m_map.size();
	}

	size_t capacity() const
	{
		return m_capacity;
	}

	bool empty() const
	{
		return m_map.empty();
	}

	bool contains(const key_type &key)
	{
		return m_map.find(key) != m_map.end();
	}

	// 设置key的过期时间
	void setttl(const key_type& key, const time_t ttl = -1)
	{
		time_t expire = (ttl != -1 ? (time(nullptr) + ttl) : -1);

		typename map_type::iterator i = m_map.find(key);
		if (i != m_map.end())
		{
			const value_type& value = std::get<0>(i->second);
			const typename list_type::iterator&it = std::get<2>(i->second);
			i->second = std::make_tuple(value, expire, it);
		}
	}

	//增加ttl
	void insert(const key_type &key, const value_type &value, const time_t ttl = -1)
	{
		time_t expire = (ttl != -1 ? (time(nullptr) + ttl) : -1);

		typename map_type::iterator i = m_map.find(key);
		if (i == m_map.end())
		{
			// insert item into the cache, but first check if it is full
			if (size() >= m_capacity)
			{
				// cache is full, evict the least recently used item
				evict();
			}

			// insert the new item
			m_list.push_front(key);
			m_map[key] = std::make_tuple(value, expire, m_list.begin());
		}
		else
		{
			m_list.erase(std::get<2>(i->second));
			m_list.push_front(key);

			i->second = std::make_tuple(value, expire, m_list.begin());
		}
	}

	boost::optional<value_type> get(const key_type &key)
	{
		// lookup value in the cache
		typename map_type::iterator i = m_map.find(key);
		if (i == m_map.end())
		{
			// value not in cache
			return boost::none;
		}

		//delete expire time
		time_t nowTime = time(nullptr);
		if (nowTime >= (uint64_t)std::get<1>(i->second))
		{
			m_list.erase(std::get<2>(i->second));
			m_map.erase(i);
			return boost::none;
		}

		// return the value, but first update its place in the most
		// recently used list
		typename list_type::iterator j = std::get<2>(i->second);
		if (j != m_list.begin())
		{
			// move item to the front of the most recently used list
			m_list.erase(j);
			m_list.push_front(key);

			// update iterator in map
			j = m_list.begin();
			const value_type &value = std::get<0>(i->second);
			const time_t &expire = std::get<1>(i->second);
			m_map[key] = std::make_tuple(value, expire, j);

			// return the value
			return value;
		}
		else
		{
			// the item is already at the front of the most recently
			// used list so just return it
			return std::get<0>(i->second);
		}
	}

	map_type getmap()
	{
		return m_map;
	}

	void clear()
	{
		m_map.clear();
		m_list.clear();
	}

private:
	void evict()
	{
		// evict item from the end of most recently used list
		typename list_type::iterator i = --m_list.end();
		m_map.erase(*i);
		m_list.erase(i);
	}

private:
	map_type m_map;
	list_type m_list;
	size_t m_capacity;
};

#endif