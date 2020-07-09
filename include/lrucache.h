/************************************************************************/
/*               �ο�boost�� lru_cache ����չ�˹���
/*               1��֧����ͬkey�Ĳ���
/*               2��֧�ֹ���ʱ��
/*               3���������ݺ�������� ʹ�ö���ɾ��
/*
/*				 ʹ�÷�ʽ��
/*				 CLRU_cache<int,std::string> nameCache(100000); // ��ౣ��100000��name
/*               logonCache.insert(1��"chen");
/*				 logonCache.insert(2��"li");
/*               logonCache.setttl(2,5);	     // ����key��2������ʱ��Ϊ5s
/*				 logonCache.insert(3��"wan",10);	 // �������ݣ�����10s �����
/*
/*				 auto name = logonCache.get(1);	  // ����һ��value��ָ��
/*				 if(name){				          // �����Ƿ����
/*					std::string myname = *name;	  // ��ȡ�����ֵ
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

	// ��ֹʹ��ָ����Ϊvalue���ͣ���������ʱ�䲻�ܿ���
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

	// ����key�Ĺ���ʱ��
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

	//����ttl
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