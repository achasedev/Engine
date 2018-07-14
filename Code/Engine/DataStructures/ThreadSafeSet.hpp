/************************************************************************/
/* File: ThreadSafeSet.hpp
/* Author: Andrew Chase
/* Date: July 13th, 2018
/* Description: Template container class for thread-safe set operations
/************************************************************************/
#pragma once
#include <set>
#include <shared_mutex>
#include "Engine/Core/EngineCommon.hpp"

template <typename T>
class ThreadSafeSet
{
public:
	//-----Public Methods-----


	// For adding to the queue
	void InsertUnique(const T& value)
	{
		m_lock.lock(); // Blocks
		m_set.insert(value);
		m_lock.unlock();
	}


	// For removing and returning from the set
	bool Remove(const T& value)
	{
		m_lock.lock(); // Blocks
		std::set<T>::iterator itr = m_set.find(value);
		bool itemExisted = (itr != m_set.end());

		if (itemExisted)
		{
			m_set.erase(itr);
		}
		
		m_lock.unlock();
		return itemExisted;
	}

	
	// For accessing a member of the set, by value
	bool Get(T& out_value)
	{
		m_lock.lock_shared();

		std::set::iterator<T> itr = m_set.find(out_value);
		bool itemExists = (itr != m_set.end());

		if (itemExists)
		{
			out_value = itr->value;
		}

		m_lock.unlock_shared();
		return itemExists;
	}


	// For clearing the set
	void Clear()
	{
		m_lock.lock();
		m_set.clear();
		m_lock.unlock();
	}

	
	// For checking containment
	bool Contains(const T& value)
	{
		m_lock.lock_shared();
		bool itemExists = (m_set.find(value) != m_set.end());
		m_lock.unlock_shared();

		return itemExists;
	}


private:
	//-----Private Data-----

	std::shared_mutex m_lock;
	std::set<T> m_set;

};
