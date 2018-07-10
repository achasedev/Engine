#pragma once
#include <queue>
#include <mutex>
#include "Engine/Core/EngineCommon.hpp"
TODO("Comment");
template <typename T>
class ThreadSafeQueue
{
public:
	//-----Public Methods-----

	void Enqueue(const T& value)
	{
		m_lock.lock();
		m_queue.push(value);
		m_lock.unlock();
	}

	bool Dequeue(T& out_value)
	{
		m_lock.lock();
		
		bool hasItem = (m_queue.size() != 0);

		if (hasItem)
		{
			out_value = m_queue.front();
			m_queue.pop();
		}

		m_lock.unlock();

		return hasItem;
	}


private:
	//-----Private Data-----

	std::mutex m_lock;
	std::queue<T> m_queue;

};
