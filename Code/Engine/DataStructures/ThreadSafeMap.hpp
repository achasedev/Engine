/************************************************************************/
/* File: ThreadSafeMap.hpp
/* Author: Andrew Chase
/* Date: July 13th, 2018
/* Description: Template container class for map storage
/************************************************************************/
#pragma once
#include <map>
#include <shared_mutex>
#include "Engine/Core/EngineCommon.hpp"

template <typename K, typename T>
class ThreadSafeMap
{
public:
	//-----Public Methods-----

	// For adding to the queue
	void Insert(const K& key, const T& value)
	{
		m_lock.lock();
		m_map[key] = value;
		m_lock.unlock();
	}

	// For removing from map
	bool Erase()
	{
		m_lock.lock(); // Blocks
		std::map<K,T>::iterator itr = m_map.find(out_value);
		bool itemExists = (itr != m_map.end());

		if (itemExists)
		{
			m_map.erase(itr);
		}

		m_lock.unlock();
		return itemExists;
	}

	// For accessing a value from the map
	bool Get(const K& key, T& out_value) const
	{
		m_lock.lock_shared();

		std::set::iterator<T> itr = m_map.find(key);
		bool itemExists = (itr != m_map.end());

		if (itemExists)
		{
			out_value = itr->second;
		}

		m_lock.unlock_shared();
		return itemExists;
	}

	// For clearing the map
	void Clear()
	{
		m_lock.lock();
		m_map.clear();
		m_lock.unlock();
	}


private:
	//-----Private Data-----

	std::shared_mutex m_lock;
	std::map<K,T> m_map;

};
