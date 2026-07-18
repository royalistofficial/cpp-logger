#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace app {

/**
 * @brief Потокобезопасная очередь с блокирующим извлечением.
 */
template <typename T>
class BlockingQueue {
public:
    BlockingQueue() = default;

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    /**
     * @brief Помещает элемент в очередь и будит одного потребителя.
     * @return false, если очередь закрыта — элемент отброшен.
     */
    bool push(T value) {
        {
            const std::lock_guard<std::mutex> lock(mutex_);
            if (closed_) {
                return false;
            }
            queue_.push(std::move(value));
        }
        notEmpty_.notify_one();
        return true;
    }

    /**
     * @brief Извлекает элемент, ожидая его появления.
     * @return Элемент либо std::nullopt, если очередь закрыта и пуста.
     */
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);

        notEmpty_.wait(lock, [this] { return !queue_.empty() || closed_; });

        if (queue_.empty()) {
            return std::nullopt;  
        }

        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    /**
     * @brief Закрывает очередь и будит всех ожидающих.
     */
    void close() noexcept {
        {
            const std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        notEmpty_.notify_all();
    }

    /// @return true, если очередь закрыта.
    bool isClosed() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

    /// @return Количество элементов, ожидающих обработки.
    std::size_t size() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::queue<T> queue_;
    bool closed_ = false;
};

}