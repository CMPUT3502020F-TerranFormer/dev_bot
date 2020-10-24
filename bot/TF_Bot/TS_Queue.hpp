//
// Created by Kerry Cao on 2020-03-31.
//

#ifndef PCHAT_TSQUEUE_HPP
#define PCHAT_TSQUEUE_HPP

#include <vector>
#include <mutex>

template<class T>
class TSqueue {
public:
    TSqueue() = default;

    ~TSqueue() = default;

    void enqueue(T t) {
        EQlock.lock();
        queue.push_back(std::move(t));
        EQlock.unlock();
    }

    T dequeue() {
        DQlock.lock();
        while (isEmpty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        T temp = queue.at(0);
        queue.erase(queue.begin());
        DQlock.unlock();
        return temp;
    }

    bool isEmpty() {
        return queue.empty();
    }

    int size() {
        return queue.size();
    }

private:
    std::vector<T> queue;
    std::mutex EQlock;
    std::mutex DQlock;
};


#endif //PCHAT_TSQUEUE_HPP