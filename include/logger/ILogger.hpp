#pragma once

#include "logger/Export.hpp"
#include "logger/LogLevel.hpp"

#include <chrono>
#include <string_view>

namespace logger {

/**
 * @brief Журнал сообщений.
 */
class LOGGER_API ILogger {
public:
    virtual ~ILogger() = default;

    ILogger(const ILogger&) = delete;
    ILogger& operator=(const ILogger&) = delete;

    /**
     * @brief Записывает сообщение с указанным уровнем важности.
     * @throw std::runtime_error при ошибке записи.
     */
    virtual void log(std::string_view message, LogLevel level) = 0;

    /**
     * @brief Записывает сообщение с текущим уровнем по умолчанию.
     * @throw std::runtime_error при ошибке записи.
     */
    virtual void log(std::string_view message) = 0;

    /**
     * @brief Записывает сообщение с заранее снятой отметкой времени.
     * @throw std::runtime_error при ошибке записи.
     */
    virtual void log(std::string_view message, LogLevel level,
                     std::chrono::system_clock::time_point timestamp) = 0;

    virtual void setDefaultLevel(LogLevel level) noexcept = 0;

    /// @return Текущий уровень по умолчанию.
    virtual LogLevel defaultLevel() const noexcept = 0;

protected:
    ILogger() = default;
};

}  // namespace logger
