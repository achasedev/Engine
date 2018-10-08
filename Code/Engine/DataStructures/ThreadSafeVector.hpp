/************************************************************************/
/* File: ThreadSafeVector.hpp
/* Author: Andrew Chase
/* Date: October 4th, 2018
/* Description: Class to represent a thread safe std::vector
/************************************************************************/
#pragma once
#include <vector>
#include <shared_mutex>

template <typename T>
class ThreadSafeVector
{
public:
	//-----Public Methods-----
	
	void PushBack(T& toAdd)
	{
		m_lock.lock();
		m_vector.push_back(toAdd);
		m_lock.unlock();
	}

	void Insert(unsigned int index, T& toAdd)
	{
		m_lock.lock();
		m_vector.insert(m_vector.begin() + index, toAdd);
		m_lock.unlock();
	}

	T& Get(unsigned int index)
	{
		m_lock.lock_shared();
		T& value = m_vector[index];
		m_lock.unlock_shared();
		return value;
	}
	
	T Remove(unsigned int index)
	{
		m_lock.lock();
		T value = m_vector[index];
		m_vector.erase(m_vector.begin() + index);
		m_lock.unlock();
		return value;
	}

	unsigned int Size()
	{
		m_lock.lock_shared();
		unsigned int size = m_vector.size();
		m_lock.unlock_shared();
		return size;
	}

private:
	//-----Private Data-----
	
	std::shared_mutex	m_lock;
	std::vector<T>		m_vector;
	
};
