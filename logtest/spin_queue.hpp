#ifndef SPIN_QUEUE_HPP
#define SPIN_QUEUE_HPP

#include <queue>
#include "spinlock.hpp"

template <typename T>
class spin_queue
{
	public:
	inline void enque(const T& item)
	{
		_lock.lock();
		_queue.push(item);
		_lock.unlock();
	}
	
	inline size_t deque(T& out)
	{
		size_t q_size = 0;
		
		_lock.lock();
		if ((q_size = _queue.size()))
		{
			out = _queue.front();
			_queue.pop();
		}
		_lock.unlock();
		
		return q_size;
	}
	
	inline size_t size()
	{
		size_t q_size = 0;
		
		_lock.lock();
		q_size = _queue.size();
		_lock.unlock();
		
		return q_size;
	}
	
	private:
	
	// some pool allocator would be nice; boost?
	std::queue<T> _queue;
	spinlock _lock;
};

#endif
