//
// Created by Carter Sabadash on 2020-10-23
//

#ifndef THREADSAFE_PRIORITY_QUEUE_H
#define THREADSAFE_PRIORITY_QUEUE_H

#include <queue>
#include <mutex>
#include <cassert>

template <typename T>
using QBase = std::priority_queue<T>;

/**
 * Implements the basic methods for a threadsafe priority_queue
 */
template <typename T>
class threadsafe_priority_queue : private QBase<T>
{
	std::mutex mutex;
public:
	using Base = QBase<T>;


	bool empty()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return Base::empty();
	}

	const T& top()
	{
		std::lock_guard<std::mutex> lock(mutex);
		assert(!Base::empty());
		return Base::top();
	}

	void push(T& x)
	{
		std::lock_guard<std::mutex> lock(mutex);
		Base::push(x);
	}

	void push(const T& x)
	{
		std::lock_guard<std::mutex> lock(mutex);
		Base::push(x);
	}

	void pop()
	{
		std::lock_guard<std::mutex> lock(mutex);
		assert(!Base::empty());
		Base::pop();
	}
};

#endif