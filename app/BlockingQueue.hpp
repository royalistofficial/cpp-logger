#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace app {

/**
 * @brief Потокобезопасная очередь с блокирующим извлечением и ограниченной
 *        ёмкостью.
 */
template <typename T>
class BlockingQueue {
public:
    static constexpr std::size_t kUnbounded = 0;

    explicit BlockingQueue(std::size_t capacity = kUnbounded)
        : capacity_(capacity) {}

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    /**
     * @brief Помещает элемент в очередь, ожидая освобождения места.
     * @return false, если очередь закрыта — элемент отброшен.
     */
    bool push(T value) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notFull_.wait(lock, [this] { return closed_ || !isFull(); });

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
        std::optional<T> value;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            notEmpty_.wait(lock, [this] { return !queue_.empty() || closed_; });

            if (queue_.empty()) {
                return std::nullopt;  // очередь закрыта и исчерпана
            }

            value = std::move(queue_.front());
            queue_.pop();
        }
        notFull_.notify_one();
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
        notFull_.notify_all();
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

    /// @return Максимальное число элементов, 0 — без ограничения.
    std::size_t capacity() const noexcept { return capacity_; }

private:
    /// Вызывается под уже захваченным mutex_.
    bool isFull() const {
        return capacity_ != kUnbounded && queue_.size() >= capacity_;
    }

    mutable std::mutex mutex_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
    std::queue<T> queue_;
    std::size_t capacity_;
    bool closed_ = false;
};

}  // namespace app
