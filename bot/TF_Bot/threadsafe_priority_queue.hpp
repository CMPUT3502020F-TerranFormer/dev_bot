//
// Created by Carter Sabadash on 2020-10-23
//

#ifndef THREADSAFE_PRIORITY_QUEUE
#define THREADSAFE_PRIORITY_QUEUE

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

	const T& top() const
	{
		std::lock_guard<std::mutex> lock(mutex);
		assert(!empty());
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
		assert(!empty());
		Base::pop();
	}
};

#endif