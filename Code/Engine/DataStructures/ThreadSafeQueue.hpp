/************************************************************************/
/* File: ThreadSafeQueue.hpp
/* Author: Andrew Chase
/* Date: July 10th, 2018
/* Description: Template container class for thread-safe push/pops
/************************************************************************/
#pragma once
#include <queue>
#include <mutex>
#include "Engine/Core/EngineCommon.hpp"

template <typename T>
class ThreadSafeQueue
{
public:
	//-----Public Methods-----

	// For adding to the queue
	void Enqueue(const T& value)
	{
		m_lock.lock(); // Blocks
		m_queue.push(value);
		m_lock.unlock();
	}


	// For removing and returning from queue
	bool Dequeue(T& out_value)
	{
		m_lock.lock(); // Blocks
		bool hasItem = (m_queue.size() != 0);

		if (hasItem)
		{
			out_value = m_queue.front();
			m_queue.pop();
		}

		m_lock.unlock();
		return hasItem;
	}

	
	// For checking if the queue is empty
	bool IsEmpty()
	{
		m_lock.lock();
		bool isEmpty = m_queue.empty();
		m_lock.unlock();

		return isEmpty;
	}


private:
	//-----Private Data-----

	std::mutex m_lock;
	std::queue<T> m_queue;

};
