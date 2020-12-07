//
// Created by Carter Sabadash on 2020-10-23
//

#ifndef THREADSAFE_PRIORITY_QUEUE_H
#define THREADSAFE_PRIORITY_QUEUE_H

#include <queue>
#include <mutex>
#include <cassert>

template<typename T>
using QBase = std::priority_queue<T>;

/**
 * Implements the basic methods for a threadsafe priority_queue
 */
template<typename T>
class threadsafe_priority_queue : private QBase<T> {
    std::mutex mutex;
public:
    using Base = QBase<T>;

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return Base::empty();
    }

    void push(T &x) {
        std::lock_guard<std::mutex> lock(mutex);
        Base::push(x);
    }

    void push(const T &x) {
        std::lock_guard<std::mutex> lock(mutex);
        Base::push(x);
    }

    const T pop() {
        std::lock_guard<std::mutex> lock(mutex);
        assert(!Base::empty());
        auto ret = Base::top();
        Base::pop();

        return ret;
    }

    const size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return Base::size();
    }

    const typename Base::container_type &get_container() {
		std::lock_guard<std::mutex> lock(mutex);
		return this->c;
	}

    void remove(T &x) {
        auto it = std::find(this->c.begin(), this->c.end(), x);
        if (it != this->c.end()) {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
        }
    }
};

#endif