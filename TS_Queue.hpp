//
// Created by Kerry Cao on 2020-03-31.
//

#ifndef PCHAT_TSQUEUE_HPP
#define PCHAT_TSQUEUE_HPP

#include <mutex>
#include <queue>
#include <cassert>

template <typename T>
using TSQBase = std::queue<T>;

/**
 * Implements the basic methods for a threadsafe queue
 */
template <typename T>
class TSqueue : private TSQBase<T>
{
	std::mutex mutex;
public:
	using Base = TSQBase<T>;

	bool empty()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return Base::empty();
	}

	const T& dequeue()
	{
		std::lock_guard<std::mutex> lock(mutex);
		assert(!Base::empty());
		T& front = Base::front();
		Base::pop();
		return front;
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

	const typename Base::container_type& get_container()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return this->c;
	}
};


#endif //PCHAT_TSQUEUE_HPP