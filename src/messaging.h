#pragma once

#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

// Publish/subscribe: a Topic copies each message into one Pipe per reader.
// Subscribe before the threads start: the list of pipes has no lock.

template <class T>
class Pipe {
public:
    explicit Pipe(std::size_t capacity) : capacity_(capacity) {}

    void push(const T& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() == capacity_) {
            queue_.pop_front();  // reader too slow: drop the oldest
        }
        queue_.push_back(msg);
    }

    std::vector<T> drain() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<T> out(queue_.begin(), queue_.end());
        queue_.clear();
        return out;
    }

private:
    std::size_t capacity_;
    std::mutex mutex_;
    std::deque<T> queue_;
};

template <class T>
class Topic {
public:
    Pipe<T>& subscribe(std::size_t capacity = 64) {
        pipes_.push_back(std::make_unique<Pipe<T>>(capacity));
        return *pipes_.back();
    }

    void publish(const T& msg) {
        for (auto& pipe : pipes_) {
            pipe->push(msg);
        }
    }

private:
    std::vector<std::unique_ptr<Pipe<T>>> pipes_;
};
