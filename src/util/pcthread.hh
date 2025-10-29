#ifndef PRODUCERCONSUMERTHREAD_HH
#define PRODUCERCONSUMERTHREAD_HH

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <utility>

template <typename T>
class ProducerConsumerThread {
public:
    explicit ProducerConsumerThread(std::string thread_name) : thread_name_(std::move(thread_name)) {}
    virtual ~ProducerConsumerThread() = default;

    void start(std::function<void(std::string)> const& on_error=nullptr) {
        running_.store(true);
        thread_ = std::thread([this, on_error] {
            for(;;) {
                std::optional<T> opt_value = pop();
                if (!opt_value)
                    return;
                try {
                    action(std::move(*opt_value));
                } catch (std::exception& e) {
                    if (on_error)
                        on_error(std::string("thread ") + thread_name_ + " exception: " + e.what());
                } catch (...) {
                    if (on_error)
                        on_error(std::string("thread ") + thread_name_ + " unknown exception");
                }
            }
        });
    }

    void stop() {
        if (running_.load()) {
            running_.store(false);
            cond_.notify_one();
            if (thread_.joinable())
                thread_.join();
        }
    }

    void push(T&& t) {
        std::lock_guard lock(mutex_);
        queue_.push_back(std::move(t));
        cond_.notify_one();
    }

protected:
    virtual void action(T&& t) = 0;

private:
    std::thread             thread_;
    std::mutex              mutex_;
    std::condition_variable cond_;
    std::deque<T>           queue_;
    std::atomic<bool>       running_ = false;
    std::string             thread_name_;

    std::optional<T> pop() {

        std::unique_lock lock(mutex_);
        cond_.wait(lock, [&]{ return !queue_.empty() || !running_.load(); });

        if (queue_.empty() && !running_.load())
            return {};

        T t = std::move(queue_.front());
        queue_.pop_front();
        return t;
    }
};

#endif //PRODUCERCONSUMERTHREAD_HH
