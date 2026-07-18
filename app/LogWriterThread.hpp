#pragma once

#include "BlockingQueue.hpp"

#include "logger/ILogger.hpp"
#include "logger/LogLevel.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <string>
#include <thread>
#include <variant>

namespace app {

/**
 * @brief Сообщение, переданное из потока ввода в поток записи.
 */
struct LogRequest {
    std::string text;
    logger::LogLevel level = logger::LogLevel::Info;
    std::chrono::system_clock::time_point timestamp;
};

/**
 * @brief Команда смены порога важности.
 */
struct SetLevelCommand {
    logger::LogLevel level = logger::LogLevel::Info;
};

using QueueItem = std::variant<LogRequest, SetLevelCommand>;

using LogQueue = BlockingQueue<QueueItem>;

/**
 * @brief Отдельный поток, разбирающий очередь и пишущий сообщения в журнал.
 */
class LogWriterThread {
public:
    /**
     * @param queue Очередь заданий. Должна пережить этот объект.
     * @param log Журнал, в который выполняется запись.
     */
    LogWriterThread(LogQueue& queue, logger::ILogger& log);

    ~LogWriterThread();

    LogWriterThread(const LogWriterThread&) = delete;
    LogWriterThread& operator=(const LogWriterThread&) = delete;

    void join();

    std::size_t processed() const noexcept;

    std::size_t failed() const noexcept;

private:
    void run();
    void handle(const LogRequest& request);
    void handle(const SetLevelCommand& command);

    LogQueue& queue_;
    logger::ILogger& log_;

    std::atomic<std::size_t> processed_{0};
    std::atomic<std::size_t> failed_{0};

    std::thread worker_;
};

}
