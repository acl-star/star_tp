#ifndef LIGHT_THREAD_SAFE_QUEUE_H
#define LIGHT_THREAD_SAFE_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
#include <memory>

template <class T> 
class ThreadSafeQueue {
  private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T> > dataQueue;
    std::condition_variable dataCond;

  public:
    ThreadSafeQueue() {}
    ThreadSafeQueue(ThreadSafeQueue const&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue const&) = delete;
    void push(T tValue) {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(tValue)));
        std::lock_guard<std::mutex> lk(mut);
        dataQueue.push(data);
        dataCond.notify_one();
    }
    void wait_and_pop(T &tValue) {
        std::unique_lock<std::mutex> lk(mut);
        dataCond.wait(lk, [this] { return !dataQueue.empty(); });
        tValue = std::move(*dataQueue.front());
        dataQueue.pop();
    }
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lk(mut);
        dataCond.wait(lk, [this] { return !dataQueue.empty(); });
        std::shared_ptr<T> ret = dataQueue.front();
        dataQueue.pop();
        return ret;
    }
    bool try_pop(T &tValue) {
        std::lock_guard<std::mutex> lk(mut);
        if (dataQueue.empty())
            return false;
        tValue = std::move(*dataQueue.front());
        dataQueue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lk(mut);
        if (dataQueue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> ret = dataQueue.front();
        dataQueue.pop();
        return ret;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return dataQueue.empty();
    }
};
#endif // LIGHT_THREAD_SAFE_QUEUE_H