#pragma once

#include "logger/LogLevel.hpp"

#include <chrono>
#include <cstddef>
#include <string>

namespace app {

/**
 * @brief Сообщение, принятое от пользователя и ожидающее записи.
 */
struct LogRequest {
    std::string text;
    logger::LogLevel level = logger::LogLevel::Info;
    std::chrono::system_clock::time_point timestamp{};
};

/**
 * @brief Итоги работы потока записи.
 */
struct WriterStats {
    std::size_t processed = 0;
    std::size_t failed = 0;
    std::size_t dropped = 0;
};

/**
 * @brief Приёмник пользовательского ввода со стороны приложения.
 */
class IMessageSink {
public:
    virtual ~IMessageSink() = default;

    IMessageSink(const IMessageSink&) = delete;
    IMessageSink& operator=(const IMessageSink&) = delete;

    /**
     * @brief Передаёт сообщение на запись.
     * @return false, если приёмник закрыт и сообщение отброшено.
     */
    virtual bool submit(LogRequest request) = 0;

    /**
     * @brief Передаёт команду смены уровня по умолчанию.
     * @return false, если приёмник закрыт.
     */
    virtual bool setDefaultLevel(logger::LogLevel level) = 0;

    /// @return Текущие счётчики.
    virtual WriterStats stats() const = 0;

protected:
    IMessageSink() = default;
};

}  // namespace app
