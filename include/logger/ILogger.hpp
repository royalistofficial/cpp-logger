#pragma once

#include "logger/LogLevel.hpp"

#include <chrono>
#include <string_view>

namespace logger {

/**
 * @brief Журнал сообщений.
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    /**
     * @brief Записывает сообщение с указанным уровнем важности.
     * @throw std::runtime_error при ошибке записи.
     */
    virtual void log(std::string_view message, LogLevel level) = 0;

    /// Записывает сообщение с текущим уровнем по умолчанию.
    virtual void log(std::string_view message) = 0;

    /**
     * @brief Записывает сообщение с заранее снятой отметкой времени.
     */
    virtual void log(std::string_view message, LogLevel level,
                     std::chrono::system_clock::time_point timestamp) = 0;

    /// Меняет уровень важности по умолчанию. Действует на последующие вызовы.
    virtual void setDefaultLevel(LogLevel level) noexcept = 0;

    /// @return Текущий уровень важности по умолчанию.
    virtual LogLevel defaultLevel() const noexcept = 0;
};

}  // namespace logger
